-- A module of LSL stuffs.

-- Using a module means it gets compiled each time?  Maybe not if I can use bytecode files.  Perhaps LuaJIT caches these?
--   Does not seem to be slowing it down noticably, but that might change once the stubs are filled out.

-- Use it like this -
-- local _LSL = require 'LSL'

--[[ From http://lua-users.org/wiki/LuaModuleFunctionCritiqued
A related note on C code: The luaL_register [9] function in C is
somewhat analogous to the module function in Lua, so luaL_register
shares similar problems, at least when a non-NULL libname is used.
Furthermore, the luaL_newmetatable/luaL_getmetatable/luaL_checkudata
functions use a C string as a key into the global registry. This poses
some potential for name conflicts--either because the modules were
written by different people or because they are different versions of
the same module loaded simultaneously. To address this, one may instead
use a lightuserdata (pointer to variable of static linkage to ensure
global uniqueness) for this key or store the metatable as an
upvalue--either way is a bit more efficient and less error prone.
]]

-- http://www.lua.org/pil/15.4.html looks useful.
-- http://www.lua.org/pil/15.5.html the last part about autoloading functions might be useful.

local LSL = {};
local SID = "";
local scriptName = "";
local running = true
local paused = false


-- Stuff called from the wire protocol has to be global, but I think this means just global to this file.
function stop()		paused = true		end
function quit()		running = false		end


-- Debugging aids

-- Functions to print tables.
local print_table, print_table_start;

function print_table_start(table, space, name)
  print(space .. name .. ": ");
  print(space .. "{");
  print_table(table, space .. "  ");
  print(space .. "}");
end

function print_table(table, space)
  for k, v in pairs(table) do
    if type(v) == "table" then
      print_table_start(v, space, k);
    elseif type(v) == "string" then
      print(space .. k .. ': "' .. v .. '";')
    else
      print(space .. k .. ": " .. v .. ";")
    end
  end
end

function msg(...)
  print(SID, ...)  -- The comma adds a tab, fancy that.  B-)
end


-- LSL function and constant creation stuff.

local args2string	-- Pre declare this.
local functions = {}
local mt = {}

local function value2string(value, Type)
  local temp = ""

  if  	 "float"	== Type then temp = temp .. value
  elseif "integer"	== Type then temp = temp .. value
  elseif "key"		== Type then temp = "\"" .. value .. "\""
  elseif "list"		== Type then temp = "[" .. args2string(true, unpack(value)) .. "]"
  elseif "table"	== Type then temp = "[" .. args2string(true, unpack(value)) .. "]"
  elseif "string"	== Type then temp = "\"" .. value .. "\""
  elseif "rotation"	== Type then temp = "<" .. value.x .. ", " .. value.y .. ", " .. value.z .. ", " .. value.s .. ">"
  elseif "vector"	== Type then temp = "<" .. value.x .. ", " .. value.y .. ", " .. value.z .. ">"
  else
    temp = temp .. value
  end
  return temp
end

function args2string(doType, ...)
  local temp = ""
  local first = true

  for j,w in ipairs( {...} ) do
    if first then first = false else temp = temp .. ", " end
    if doType then
      temp = temp .. value2string(w, type(w))
    else
      temp = temp .. w
    end
  end
  return temp
end

function mt.callAndReturn(name, ...)
  luaproc.sendback(name .. "(" .. args2string(true, ...) .. ")")
end

function mt.callAndWait(name, ...)
  local func = functions[name]

  mt.callAndReturn(name, ...);
  -- Eventually a sendForth() is called, which should end up passing through SendToChannel().
  -- Wait for the result, which should be a Lua value as a string.
  local message = luaproc.receive(SID)
  if message then
    result, errorMsg = loadstring("return " .. message)  -- "The environment of the returned function is the global environment."  Though normally, a function inherits it's environment from the function creating it.  Which is what we want.  lol
    if nil == result then
      msg("Not a valid result: " .. message .. "  ERROR MESSAGE: " .. errorMsg)
    else
      -- Set the functions environment to ours, for the protection of the script, coz loadstring sets it to the global environment instead.
      setfenv(result, getfenv(1))
      status, result = pcall(result)
      if not status then
	msg("Error from result: " .. message .. "  ERROR MESSAGE: " .. result)
      elseif result then
	return result
      end
    end
  end

  if	 "float"	== func.Type then return 0.0
  elseif "integer"	== func.Type then return 0
  elseif "key"		== func.Type then return LSL.NULL_KEY
  elseif "list"		== func.Type then return {}
  elseif "string"	== func.Type then return ""
  elseif "rotation"	== func.Type then return LSL.ZERO_ROTATION
  elseif "vector"	== func.Type then return LSL.ZERO_VECTOR
  end
  return nil
end

local function newConst(Type, name, value)
  LSL[name] = value
  return { Type = Type, name = name }
end

local function newFunc(Type, name, ... )
  functions[name] = { Type = Type, args = {...} }
  if "" == Type then
    LSL[name] = function(...) mt.callAndReturn(name, ... ) end
  else
    LSL[name] = function(...) return mt.callAndWait(name, ... ) end
  end
end


-- LSL constants.

local constants =
{
  newConst("float", "PI",			3.14159265358979323846264338327950),
  newConst("float", "PI_BY_TWO",		LSL.PI / 2),		-- 1.57079632679489661923132169163975
  newConst("float", "TWO_PI",			LSL.PI * 2),		-- 6.28318530717958647692528676655900
  newConst("float", "DEG_TO_RAD",		LSL.PI / 180.0),	-- 0.01745329252
  newConst("float", "RAD_TO_DEG",		180.0 / LSL.PI),	-- 57.2957795131
  newConst("float", "SQRT2",			1.4142135623730950488016887242097),

  newConst("integer", "CHANGED_INVENTORY",	0x001),
  newConst("integer", "CHANGED_COLOR",		0x002),
  newConst("integer", "CHANGED_SHAPE",		0x004),
  newConst("integer", "CHANGED_SCALE",		0x008),
  newConst("integer", "CHANGED_TEXTURE",	0x010),
  newConst("integer", "CHANGED_LINK",		0x020),
  newConst("integer", "CHANGED_ALLOWED_DROP",	0x040),
  newConst("integer", "CHANGED_OWNER",		0x080),
  newConst("integer", "CHANGED_REGION",		0x100),
  newConst("integer", "CHANGED_TELEPORT",	0x200),
  newConst("integer", "CHANGED_REGION_START",	0x400),
  newConst("integer", "CHANGED_MEDIA",		0x800),

  newConst("integer", "DEBUG_CHANNEL",		2147483647),
  newConst("integer", "PUBLIC_CHANNEL",		0),

  newConst("integer", "INVENTORY_ALL",		-1),
  newConst("integer", "INVENTORY_NONE",		-1),
  newConst("integer", "INVENTORY_TEXTURE",	0),
  newConst("integer", "INVENTORY_SOUND",	1),
  newConst("integer", "INVENTORY_LANDMARK",	3),
  newConst("integer", "INVENTORY_CLOTHING",	5),
  newConst("integer", "INVENTORY_OBJECT",	6),
  newConst("integer", "INVENTORY_NOTECARD",	7),
  newConst("integer", "INVENTORY_SCRIPT",	10),
  newConst("integer", "INVENTORY_BODYPART",	13),
  newConst("integer", "INVENTORY_ANIMATION",	20),
  newConst("integer", "INVENTORY_GESTURE",	21),

  newConst("integer", "ALL_SIDES",		-1),
  newConst("integer", "LINK_SET",		-1),
  newConst("integer", "LINK_ROOT",		1),
  newConst("integer", "LINK_ALL_OTHERS",	-2),
  newConst("integer", "LINK_ALL_CHILDREN",	-3),
  newConst("integer", "LINK_THIS",		-4),

  newConst("integer", "PERM_ALL",		0x7FFFFFFF),
  newConst("integer", "PERM_COPY",		0x00008000),
  newConst("integer", "PERM_MODIFY",		0x00004000),
  newConst("integer", "PERM_MOVE",		0x00080000),
  newConst("integer", "PERM_TRANSFER",		0x00002000),
  newConst("integer", "MASK_BASE",		0),
  newConst("integer", "MASK_OWNER",		1),
  newConst("integer", "MASK_GROUP",		2),
  newConst("integer", "MASK_EVERYONE",		3),
  newConst("integer", "MASK_NEXT",		4),
  newConst("integer", "PERMISSION_DEBIT",		0x0002),
  newConst("integer", "PERMISSION_TAKE_CONTROLS",	0x0004),
  newConst("integer", "PERMISSION_TRIGGER_ANIMATION",	0x0010),
  newConst("integer", "PERMISSION_ATTACH",		0x0020),
  newConst("integer", "PERMISSION_CHANGE_LINKS",	0x0080),
  newConst("integer", "PERMISSION_TRACK_CAMERA",	0x0400),
  newConst("integer", "PERMISSION_CONTRAL_CAMERA",	0x0800),

  newConst("integer", "AGENT",			0x01),
  newConst("integer", "ACTIVE",			0x02),
  newConst("integer", "PASSIVE",		0x04),
  newConst("integer", "SCRIPTED",		0x08),

  newConst("integer", "OBJECT_UNKNOWN_DETAIL",	-1),

  newConst("integer", "PRIM_BUMP_SHINY",	19),
  newConst("integer", "PRIM_COLOR",		18),
  newConst("integer", "PRIM_FLEXIBLE",		21),
  newConst("integer", "PRIM_FULLBRIGHT",	20),
  newConst("integer", "PRIM_GLOW",		25),
  newConst("integer", "PRIM_MATERIAL",		2),
  newConst("integer", "PRIM_PHANTOM",		5),
  newConst("integer", "PRIM_PHYSICS",		3),
  newConst("integer", "PRIM_POINT_LIGHT",	23),
  newConst("integer", "PRIM_POSITION",		6),
  newConst("integer", "PRIM_ROTATION",		8),
  newConst("integer", "PRIM_SIZE",		7),
  newConst("integer", "PRIM_TEMP_ON_REZ",	4),
  newConst("integer", "PRIM_TYPE",		9),
  newConst("integer", "PRIM_TYPE_OLD",		1),
  newConst("integer", "PRIM_TEXGEN",		22),
  newConst("integer", "PRIM_TEXTURE",		17),
  newConst("integer", "PRIM_TEXT",		26),

  newConst("integer", "PRIM_BUMP_NONE",		0),
  newConst("integer", "PRIM_BUMP_BRIGHT",	1),
  newConst("integer", "PRIM_BUMP_DARK",		2),
  newConst("integer", "PRIM_BUMP_WOOD",		3),
  newConst("integer", "PRIM_BUMP_BARK",		4),
  newConst("integer", "PRIM_BUMP_BRICKS",	5),
  newConst("integer", "PRIM_BUMP_CHECKER",	6),
  newConst("integer", "PRIM_BUMP_CONCRETE",	7),
  newConst("integer", "PRIM_BUMP_TILE",		8),
  newConst("integer", "PRIM_BUMP_STONE",	9),
  newConst("integer", "PRIM_BUMP_DISKS",	10),
  newConst("integer", "PRIM_BUMP_GRAVEL",	11),
  newConst("integer", "PRIM_BUMP_BLOBS",	12),
  newConst("integer", "PRIM_BUMP_SIDING",	13),
  newConst("integer", "PRIM_BUMP_LARGETILE",	14),
  newConst("integer", "PRIM_BUMP_STUCCO",	15),
  newConst("integer", "PRIM_BUMP_SUCTION",	16),
  newConst("integer", "PRIM_BUMP_WEAVE",	17),

  newConst("integer", "PRIM_HOLE_DEFAULT",	0),
  newConst("integer", "PRIM_HOLE_CIRCLE",	16),
  newConst("integer", "PRIM_HOLE_SQUARE",	32),
  newConst("integer", "PRIM_HOLE_TRIANGLE",	48),

  newConst("integer", "PRIM_MATERIAL_STONE",	0),
  newConst("integer", "PRIM_MATERIAL_METAL",	1),
  newConst("integer", "PRIM_MATERIAL_GLASS",	2),
  newConst("integer", "PRIM_MATERIAL_WOOD",	3),
  newConst("integer", "PRIM_MATERIAL_FLESH",	4),
  newConst("integer", "PRIM_MATERIAL_PLASTIC",	5),
  newConst("integer", "PRIM_MATERIAL_RUBBER",	6),
  newConst("integer", "PRIM_MATERIAL_LIGHT",	7),

  newConst("integer", "PRIM_SCULPT_TYPE_SPHERE",	1),
  newConst("integer", "PRIM_SCULPT_TYPE_TORUS",		2),
  newConst("integer", "PRIM_SCULPT_TYPE_PLANE",		3),
  newConst("integer", "PRIM_SCULPT_TYPE_CYLINDER",	4),
  newConst("integer", "PRIM_SCULPT_TYPE_MESH",		5),
  newConst("integer", "PRIM_SCULPT_TYPE_MIMESH",	6),

  newConst("integer", "PRIM_SHINY_NONE",	0),
  newConst("integer", "PRIM_SHINY_LOW",		1),
  newConst("integer", "PRIM_SHINY_MEDIUM",	2),
  newConst("integer", "PRIM_SHINY_HIGH",	3),

  newConst("integer", "PRIM_TYPE_BOX",		0),
  newConst("integer", "PRIM_TYPE_CYLINDER",	1),
  newConst("integer", "PRIM_TYPE_PRISM",	2),
  newConst("integer", "PRIM_TYPE_SPHERE",	3),
  newConst("integer", "PRIM_TYPE_TORUS",	4),
  newConst("integer", "PRIM_TYPE_TUBE",		5),
  newConst("integer", "PRIM_TYPE_RING",		6),
  newConst("integer", "PRIM_TYPE_SCULPT",	7),

  newConst("integer", "STRING_TRIM",		3),
  newConst("integer", "STRING_TRIM_HEAD",	1),
  newConst("integer", "STRING_TRIM_TAIL",	2),

  newConst("integer", "TRUE",			1),
  newConst("integer", "FALSE",			0),

  newConst("integer", "TYPE_INTEGER",		1),
  newConst("integer", "TYPE_FLOAT",		2),
  newConst("integer", "TYPE_STRING",		3),
  newConst("integer", "TYPE_KEY",		4),
  newConst("integer", "TYPE_VECTOR",		5),
  newConst("integer", "TYPE_ROTATION",		6),
  newConst("integer", "TYPE_INVALID",		0),

  newConst("string", "NULL_KEY",		"00000000-0000-0000-0000-000000000000"),
  newConst("string", "EOF",			"\\n\\n\\n"),	-- Corner case, dealt with later.

  newConst("rotation", "ZERO_ROTATION",		{x=0.0, y=0.0, z=0.0, s=1.0}),
  newConst("vector", "ZERO_VECTOR",		{x=0.0, y=0.0, z=0.0}),

-- TODO - Temporary dummy variables to get vector and rotation thingies to work for now.

  newConst("float", "s",			1.0),
  newConst("float", "x",			0.0),
  newConst("float", "y",			0.0),
  newConst("float", "z",			0.0),
}

-- ll*() function definitions

-- LSL avatar functions
newFunc("key",		"llAvatarOnSitTarget")
newFunc("list",		"llGetAnimationList", "key id")
newFunc("integer",	"llGetPermissions")
newFunc("key",		"llGetPermissionsKey")
newFunc("string",	"llKey2Name", "key avatar")
newFunc("",		"llRequestPermissions", "key avatar", "integer perms")
newFunc("integer",	"llSameGroup", "key avatar")
newFunc("",		"llStartAnimation", "string anim")
newFunc("",		"llStopAnimation", "string anim")
newFunc("",		"llUnSit", "key avatar")

-- LSL collision / detect / sensor functions
newFunc("key",		"llDetectedGroup", "integer index")
newFunc("key",		"llDetectedKey", "integer index")

-- LSL communications functions
newFunc("",		"llDialog", "key avatar", "string caption", "list arseBackwardsMenu", "integer channel")
newFunc("integer",	"llListen", "integer channel", "string name", "key id", "string msg")
newFunc("",		"llListenRemove", "integer handle")
newFunc("",		"llOwnerSay", "string text")
newFunc("",		"llSay", "integer channel", "string text")
newFunc("",		"llShout", "integer channel", "string text")
newFunc("",		"llWhisper", "integer channel", "string text")
newFunc("",		"llMessageLinked", "integer link", "integer num", "string text", "key aKey")

-- LSL inventory functions.
newFunc("string",	"llGetInventoryName", "integer Type", "integer index")
newFunc("integer",	"llGetInventoryNumber", "integer Type")
newFunc("integer",	"llGetInventoryType", "string name")
newFunc("key",		"llGetNotecardLine", "string name", "integer index")
newFunc("",		"llRezAtRoot", "string name", "vector position", "vector velocity", "rotation rot", "integer channel")
newFunc("",		"llRezObject", "string name", "vector position", "vector velocity", "rotation rot", "integer channel")

-- LSL list functions.
newFunc("list",		"llCSV2List", "string text")
newFunc("list",		"llDeleteSubList", "list l", "integer start", "integer End")
newFunc("string",	"llDumpList2String", "list l", "string separator")
newFunc("integer",	"llGetListLength", "list l")
newFunc("string",	"llList2CSV", "list l")
newFunc("float",	"llList2Float", "list l", "integer index")
newFunc("integer",	"llList2Integer", "list l", "integer index")
newFunc("key",		"llList2Key", "list l", "integer index")
newFunc("list",		"llList2List", "list l", "integer start", "integer End")
newFunc("string",	"llList2String", "list l", "integer index")
newFunc("rotation",	"llList2Rotation", "list l", "integer index")
newFunc("vector",	"llList2Vector", "list l", "integer index")
newFunc("integer",	"llListFindList", "list l", "list l1")
newFunc("list",		"llListInsertList", "list l", "list l1", "integer index")
newFunc("list",		"llListReplaceList", "list l", "list part", "integer start", "integer End")
newFunc("list",		"llListSort", "list l", "integer stride", "integer ascending")
newFunc("list",		"llParseString2List", "string In", "list l", "list l1")
newFunc("list",		"llParseStringKeepNulls", "string In", "list l", "list l1")

-- LSL math functions
newFunc("rotation",	"llEuler2Rot", "vector vec")
newFunc("float",	"llFrand", "float max")
newFunc("float",	"llPow", "float number", "float places")
newFunc("vector",	"llRot2Euler", "rotation rot")
newFunc("integer",	"llRound", "float number")

-- LSL media functions
newFunc("",		"llPlaySound", "string name", "float volume")

-- LSL object / prim functions
newFunc("",		"llDie")
newFunc("key",		"llGetKey")
newFunc("integer",	"llGetLinkNumber")
newFunc("string",	"llGetObjectDesc")
newFunc("string",	"llGetObjectName")
newFunc("key",		"llGetOwner")
newFunc("",		"llSetObjectDesc", "string text")
newFunc("",		"llSetObjectName", "string text")
newFunc("",		"llSetPrimitiveParams", "list params")
newFunc("",		"llSetSitText", "string text")
newFunc("",		"llSetText", "string text", "vector colour", "float alpha")
newFunc("",		"llSitTarget", "vector pos", "rotation rot")

-- LSL rotation / scaling / translation functions
newFunc("vector",	"llGetPos")
newFunc("rotation",	"llGetRot")
newFunc("",		"llSetPos", "vector pos")
newFunc("",		"llSetRot", "rotation rot")
newFunc("",		"llSetScale", "vector scale")

-- LSL script functions
newFunc("integer",	"llGetFreeMemory")
newFunc("string",	"llGetScriptName")
newFunc("",		"llResetOtherScript", "string name")
newFunc("",		"llResetScript")
newFunc("",		"llSetScriptState", "string name", "integer running")

-- LSL string functions
newFunc("string",	"llGetSubString", "string text", "integer start", "integer End")
newFunc("integer",	"llStringLength", "string text")
newFunc("string",	"llStringTrim", "string text", "integer type")
newFunc("integer",	"llSubStringIndex", "string text", "string sub")

-- LSL texture functions
newFunc("float",	"llGetAlpha", "integer side")
newFunc("",		"llSetAlpha", "float alpha", "integer side")
newFunc("",		"llSetColor", "vector colour", "integer side")

-- LSL time functions
newFunc("float",	"llGetTime")
newFunc("",		"llResetTime")
newFunc("",		"llSetTimerEvent", "float seconds")
newFunc("float",	"llSleep", "float seconds")  -- Faked return type, it actually does not return anything.  This forces it to wait.  Actually fully implements llSleep().  B-)


-- TODO - fake this for now.
function --[[integer]] 	LSL.llGetInventoryType(--[[string]] name) return LSL.INVENTORY_SCRIPT end;


-- LSL list functions.

function --[[list]]	LSL.llCSV2List(--[[string]] text) return {} end;
function --[[list]]	LSL.llDeleteSubList(--[[list]] l,--[[integer]] start,--[[integer]] eNd)
  local result = {}
  local x = 1

  -- Deal with the impedance mismatch.
  start = start + 1
  eNd   = eNd + 1
  for i = 1,#l do
    if      i < start then result[x] = l[i];  x = x + 1
    elseif  i > eNd   then result[x] = l[i];  x = x + 1
    end
  end

  return result
end

function --[[string]]	LSL.llDumpList2String(--[[list]] l, --[[string]] separator)
  local result = ""
  for i = 1,#l do
    if "" ~= result then result = result .. separator end
    result = result .. l[i]
  end
  return result
end

function --[[integer]] 	LSL.llGetListLength(--[[list]] l)
  return #l
end

function --[[string]]	LSL.llList2CSV(--[[list]] l)
  return LSL.llDumpList2String(l, ",")
end

function --[[float]] 	LSL.llList2Float(--[[list]] l,--[[integer]] index)
  local result = tonumber(l[index])
  if nil == result then result = 0.0 end
  return result
end

function --[[integer]] 	LSL.llList2Integer(--[[list]] l,--[[integer]] index)
  local result = tonumber(l[index+1])
  if nil == result then result = 0 end
  return result
end

function --[[key]]	LSL.llList2Key(--[[list]] l,--[[integer]] index)
  local result = l[index+1]
  if result then return "" .. result else return LSL.NULL_KEY end
end

function --[[list]]	LSL.llList2List(--[[list]] l,--[[integer]] start,--[[integer]] eNd)
  local result = {}
  local x = 1

  --[[ TODO -
Using negative numbers for start and/or end causes the index to count backwards from the length of the list, so 0, -1 would capture the entire list.
If start is larger than end the list returned is the exclusion of the entries, so 6, 4 would give the entire list except for the 5th entry.
    ]]

  -- Deal with the impedance mismatch.
  start = start + 1
  eNd   = eNd + 1
  for i = 1,#l do
    if      i >= start then result[x] = l[i];  x = x + 1
    elseif  i <= eNd   then result[x] = l[i];  x = x + 1
    end
  end

  return result
end

function --[[string]]	LSL.llList2String(--[[list]] l,--[[integer]] index)
  local result = l[index+1]
  if result then return "" .. result else return "" end
end

function --[[rotation]]	LSL.llList2Rotation(--[[list]] l,--[[integer]] index)
  local result = l[index+1]
  if nil == result then result = LSL.ZERO_ROTATION end
  -- TODO - check if it's not an actual rotation, then return LSS.ZERO_ROTATION
  return result
end

function --[[vector]]	LSL.llList2Vector(--[[list]] l,--[[integer]] index)
  local result = l[index+1]
  if nil == result then result = LSL.ZERO_VECTOR end
  -- TODO - check if it's not an actual rotation, then return LSS.ZERO_VECTOR
  return result
end

function --[[integer]] 	LSL.llListFindList(--[[list]] l, --[[list]] l1) return 0 end;
function --[[list]]	LSL.llListInsertList(--[[list]] l, --[[list]] l1,--[[integer]] index)
  local result = {}
  local x = 1
  local y
  for i = 1,index do
    result[x] = l[i]
    x = x + 1
  end
  y = x
  for i = 1,#ll do
    result[x] = ll[i]
    x = x + 1
  end
  for i = y,#l do
    result[x] = l[i]
    x = x + 1
  end
  return result
end

function --[[list]]	LSL.llListReplaceList(--[[list]] l, --[[list]] part,--[[integer]] start,--[[integer]] eNd)
  local result = {}
  local x = 1
  local y
  for i = 1,index do
    result[x] = l[i]
    x = x + 1
  end
  for i = 1,#part do
    result[x] = part[i]
    x = x + 1
  end
  for i = index,#l do
    result[x] = l[i]
    x = x + 1
  end
  return result
end

function --[[list]]	LSL.llListSort(--[[list]] l,--[[integer]] stride,--[[integer]] ascending)
  local result = {}

  -- TODO - Deal with stride and ascending.
  for i = 1,#l do
    result[x] = l[i];  x = x + 1
  end
  table.sort(result)

  return result
end

function --[[list]]	LSL.llParseString2List(--[[string]] In, --[[list]] l, --[[list]] l1) return {} end;
function --[[list]]	LSL.llParseStringKeepNulls(--[[string]] In, --[[list]] l, --[[list]] l1) return {} end;


-- LSL script functions

function --[[string]] LSL.llGetScriptName()
  return scriptName
end


-- LSL string functions
function --[[string]] LSL.llGetSubString( --[[string]] text, --[[integer]] start, --[[integer]] End)
  -- Deal with the impedance mismatch.
  if 0 <= start  then  start = start + 1  end
  if 0 <= End    then  End   = End   + 1  end
-- TODO - If start is larger than end the substring is the exclusion of the entries, so 6,4 would give the entire string except for the 5th character.
  return string.sub(text, start, End)
end


-- Crements stuff.

function LSL.preDecrement(name) _G[name] = _G[name] - 1; return _G[name]; end;
function LSL.preIncrement(name) _G[name] = _G[name] + 1; return _G[name]; end;
function LSL.postDecrement(name) local temp = _G[name]; _G[name] = _G[name] - 1; return temp; end;
function LSL.postIncrement(name) local temp = _G[name]; _G[name] = _G[name] + 1; return temp; end;


-- State stuff

local currentState = {}

function LSL.stateChange(x)
  if currentState ~= x then  -- Changing to the same state is a NOP.
    -- TODO - Should clear out pending events, except timer()
    -- Also forget about any event setup that needs setting up via some ll*() function, except timer().
    if nil ~= currentState.state_exit then
      currentState.state_exit();
    end
    currentState = x;
    --[[  Never return to the current states event handler.  In theory.  lol
	  Notably, it's not actually legal to do a state change from a function, only from handlers.
	  There is a hack though, but it's unsupported, so I don't have to worry about it so much.

	  Write out "state new;" as "return _LSL.stateChange(newState);", with stateChange() returning new.state_entry()) which will force two tail calls.

	  The caller of stateChange() might be a function rather than an event handler.
	  Which will return to the event handler (possibly via other nested function calls) as if the state never changed.
	  http://lslwiki.net/lslwiki/wakka.php?wakka=FunctionStateChangeHack seems to imply that this is exactly what LSL does.
	  Soooo, this might actually work perfectly.
	  Except for one minor quirk, as that page shows - only the top level function's state sticks, unless the handler does one to, then the handlers one overrides things.
	  Which I can probably ignore anyway, as this entire calling states within functions thing is an unsupported hack.
      ]]
    if nil ~= currentState.state_entry then
      return currentState.state_entry();
    end
  end
end;

function LSL.mainLoop(sid, name, x)
  local status, errorMsg = luaproc.newchannel(sid)
  local result

  SID = sid
  scriptName = name

  LSL.EOF = "\n\n\n"	-- Fix this up now.

  if not status then
    msg("Can't open the luaproc channel " .. sid .. "  ERROR MESSAGE: " .. errorMsg)
    return
  end

  LSL.stateChange(x);

  -- Need a FIFO queue of incoming events.  Which will be in the C main thread, coz that's listening on the socket for us.
  -- The ecore_con stuff ends up being a sorta FIFO queue of the commands coming from OpenSim.
  -- Plus, I think the luaproc message system manages a FIFO queue for us as well.
  -- Might still need one.  lol

  while running do
    local message = luaproc.receive(sid)
    if message then
      if paused then
	if "start()" == message then paused = false  end
      else
	result, errorMsg = loadstring(message)  -- "The environment of the returned function is the global environment."  Though normally, a function inherits it's environment from the function creating it.  Which is what we want.  lol
	if nil == result then
	  msg("Not a valid event: " .. message .. "  ERROR MESSAGE: " .. errorMsg)
	else
	  -- Set the functions environment to ours, for the protection of the script, coz loadstring sets it to the global environment instead.
	  -- TODO - On the other hand, we will need the global environment when we call event handlers.  So we should probably stash it around here somewhere.
	  setfenv(result, getfenv(1))
	  status, result = pcall(result)
	  if not status then
	    msg("Error from event: " .. message .. "  ERROR MESSAGE: " .. result)
	  elseif result then
	    status, errorMsg = luaproc.send(sid, result)
	    if not status then
	      msg("Error sending results from event: " .. message .. "  ERROR MESSAGE: " .. errorMsg)
	    end
	  end
	end
      end
    end
  end
end


-- Typecasting stuff.

function LSL.floatTypecast(x)
  local temp = tonumber(x)
  if nil == temp then temp = 0 end
  return temp;
end

function LSL.integerTypecast(x)
  local temp = tonumber(x)
  if nil == temp then temp = 0 end
  return temp;
end

function LSL.keyTypecast(x)
  return "" .. x;
end

function LSL.listTypecast(x)
  return {x};
end

function LSL.rotationTypecast(x)
   return x;
end

function LSL.stringTypecast(x)
   return "" .. x;
end

function LSL.vectorTypecast(x)
   return x;
end


-- Called at compiler set up time, to produce the constants.lsl file.
function LSL.gimmeLSL()
  for i,v in ipairs(constants) do
    local value = LSL[v.name]

    print(v.Type .. " " .. v.name .. " = " .. value2string(value, v.Type) .. ";")
  end

  for k in pairs(functions) do
    local v = functions[k]
    if nil == v.args then
      print(v.Type .. " " .. k .. "(){}")
    else
      print(v.Type .. " " .. k .. "(" .. args2string(false, unpack(v.args)) .. "){}")
    end
  end
end


return LSL;
