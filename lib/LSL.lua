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
local currentState = {}
local detectedGroups = {}
local detectedGrabs = {}
local detectedKeys = {}
local detectedLinkNumbers = {}
local detectedNames = {}
local detectedOwners = {}
local detectedPoss = {}
local detectedRots = {}
local detectedTouchBinormals = {}
local detectedTouchFaces = {}
local detectedTouchNormals = {}
local detectedTouchPoss = {}
local detectedTouchSTs = {}
local detectedTouchUVs = {}
local detectedTypes = {}
local detectedVels = {}
local waitAndProcess


-- Debugging aids

-- Functions to print tables.
local print_table, print_table_start

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


-- Stuff called from the wire protocol has to be global, but I think this means just global to this file.

events = {}

function start()	paused = false		end
function stop()		paused = true		end
function quit()		running = false		end

function events.detectedGroups(list)		detectedGroups = list		end
function events.detectedGrabs(list)		detectedGrabs = list		end
function events.detectedKeys(list)		detectedKeys = list		end
function events.detectedLinkNumbers(list)	detectedLinkNumbers = list	end
function events.detectedNames(list)		detectedNames = list		end
function events.detectedOwners(list)		detectedOwners = list		end
function events.detectedPoss(list)		detectedPoss = list		end
function events.detectedRots(list)		detectedRots = list		end
function events.detectedTouchBinormals(list)	detectedTouchBinormals = list	end
function events.detectedTouchFaces(list)	detectedTouchFaces = list	end
function events.detectedTouchNormals(list)	detectedTouchNormals = list	end
function events.detectedTouchPoss(list)		detectedTouchPoss = list	end
function events.detectedTouchSTs(list)		detectedTouchSTs = list		end
function events.detectedTouchUVs(list)		detectedTouchUVs = list		end
function events.detectedTypes(list)		detectedTypes = list		end
function events.detectedVels(list)		detectedVels = list		end

function events.detectsClear()
  detectedGroups = {}
  detectedGrabs = {}
  detectedKeys = {}
  detectedLinkNumbers = {}
  detectedNames = {}
  detectedOwners = {}
  detectedPoss = {}
  detectedRots = {}
  detectedTouchBinormals = {}
  detectedTouchFaces = {}
  detectedTouchNormals = {}
  detectedTouchPoss = {}
  detectedTouchSTs = {}
  detectedTouchUVs = {}
  detectedTypes = {}
  detectedVels = {}
end

function events.at_rot_target(tnum, targetrot, ourrot)					if nil ~= currentState.at_rot_target		then currentState.at_rot_target(tnum, targetrot, ourrot)				end  events.detectsClear()  end
function events.at_target(tnum, targetpos, ourpos)					if nil ~= currentState.at_target		then currentState.at_target(tnum, targetpos, ourpos)					end  events.detectsClear()  end
function events.attach(id)								if nil ~= currentState.attach			then currentState.attach(id)								end  events.detectsClear()  end
function events.changed(change)								if nil ~= currentState.changed			then currentState.changed(change)							end  events.detectsClear()  end
function events.collision_start(num_detected)						if nil ~= currentState.collision_start		then currentState.collision_start(num_detected)						end  events.detectsClear()  end
function events.collision(num_detected)							if nil ~= currentState.collision		then currentState.collision(num_detected)						end  events.detectsClear()  end
function events.collision_end(num_detected)						if nil ~= currentState.collision_end		then currentState.collision_end(num_detected)						end  events.detectsClear()  end
function events.control(id, held, changed)						if nil ~= currentState.control			then currentState.control(id, held, changed)						end  events.detectsClear()  end
function events.dataserver(queryid, data)						if nil ~= currentState.dataserver		then currentState.dataserver(queryid, data)						end  events.detectsClear()  end
function events.email(Time, address, subj, message, num_left)				if nil ~= currentState.email			then currentState.email(Time, address, subj, message, num_left)				end  events.detectsClear()  end
function events.http_request(request_id, status, metadata, body)			if nil ~= currentState.http_request		then currentState.http_request(request_id, status, metadata, body)			end  events.detectsClear()  end
function events.http_response(request_id, status, metadata, body)			if nil ~= currentState.http_response		then currentState.http_response(request_id, status, metadata, body)			end  events.detectsClear()  end
function events.land_collision_start(pos)						if nil ~= currentState.land_collision_start	then currentState.land_collision_start(pos)						end  events.detectsClear()  end
function events.land_collision(pos)							if nil ~= currentState.land_collision		then currentState.land_collision(pos)							end  events.detectsClear()  end
function events.land_collision_end(pos)							if nil ~= currentState.land_collision_end	then currentState.land_collision_end(pos)						end  events.detectsClear()  end
function events.link_message(sender_num, num, str, id)					if nil ~= currentState.link_message		then currentState.link_message(sender_num, num, str, id)				end  events.detectsClear()  end
function events.listen(channel, name, id, message)					if nil ~= currentState.listen			then currentState.listen(channel, name, id, message)					end  events.detectsClear()  end
function events.money(id, amount)							if nil ~= currentState.money			then currentState.money(id, amount)							end  events.detectsClear()  end
function events.moving_start()								if nil ~= currentState.moving_start		then currentState.moving_start()							end  events.detectsClear()  end
function events.moving_end()								if nil ~= currentState.moving_end		then currentState.moving_end()								end  events.detectsClear()  end
function events.no_sensor()								if nil ~= currentState.no_sensor		then currentState.no_sensor()								end  events.detectsClear()  end
function events.not_at_rot_target()							if nil ~= currentState.not_at_rot_target	then currentState.not_at_rot_target()							end  events.detectsClear()  end
function events.not_at_target()								if nil ~= currentState.not_at_target		then currentState.not_at_target()							end  events.detectsClear()  end
function events.object_rez(id)								if nil ~= currentState.object_rez		then currentState.object_rez()								end  events.detectsClear()  end
function events.on_rez(start_param)							if nil ~= currentState.on_rezz			then currentState.on_rez(start_param)							end  events.detectsClear()  end
function events.remote_data(event_type, channel, message_id, sender, idata, sdata)	if nil ~= currentState.remote_data		then currentState.remote_data(event_type, channel, message_id, sender, idata, sdata)	end  events.detectsClear()  end
function events.run_time_permissions(perm)						if nil ~= currentState.run_time_permisions	then currentState.run_time_permissions(perm)						end  events.detectsClear()  end
function events.sensor(num_detected)							if nil ~= currentState.sensor			then currentState.sensor(num_detected)							end  events.detectsClear()  end
function events.state_entry()								if nil ~= currentState.state_entry		then currentState.state_entry()								end  events.detectsClear()  end
function events.state_exit()								if nil ~= currentState.state_exit		then currentState.state_exit()								end  events.detectsClear()  end
function events.timer()									if nil ~= currentState.timer			then currentState.timer()								end  events.detectsClear()  end
function events.touch_start(num_detected)						if nil ~= currentState.touch_start		then currentState.touch_start(num_detected)						end  events.detectsClear()  end
function events.touch(num_detected)							if nil ~= currentState.touch			then currentState.touch(num_detected)							end  events.detectsClear()  end
function events.touch_end(num_detected)							if nil ~= currentState.touch_end		then currentState.touch_end(num_detected)						end  events.detectsClear()  end
function events.transaction_result(id, success, data)					if nil ~= currentState.transaction_result	then currentState.transaction_result(id, success, data)					end  events.detectsClear()  end


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
  mt.callAndReturn(name, ...);
  -- Eventually a sendForth() is called, which should end up passing through SendToChannel().
  -- Wait for the result, which should be a Lua value as a string.
  return waitAndProcess(true)
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

  newConst("integer", "TOUCH_INVALID_FACE",	0x7FFFFFFF),
  newConst("vector",  "TOUCH_INVALID_TEXCOORD",	{x=-1.0, y=-1.0, z=0.0}),
  newConst("vector",  "TOUCH_INVALID_VECTOR",	{x=0.0, y=0.0, z=0.0}),

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
newFunc("integer",	"llDetectedGroup", "integer index")
newFunc("vector",	"llDetectedGrab", "integer index")
newFunc("key",		"llDetectedKey", "integer index")
newFunc("integer",	"llDetectedLinkNumber", "integer index")
newFunc("string",	"llDetectedName", "integer index")
newFunc("key",		"llDetectedOwner", "integer index")
newFunc("vector",	"llDetectedPos", "integer index")
newFunc("rotation",	"llDetectedRot", "integer index")
newFunc("vector",	"llDetectedTouchBinormal", "integer index")
newFunc("integer",	"llDetectedTouchFace", "integer index")
newFunc("vector",	"llDetectedTouchNormal", "integer index")
newFunc("vector",	"llDetectedTouchPos", "integer index")
newFunc("vector",	"llDetectedTouchST", "integer index")
newFunc("vector",	"llDetectedTouchUV", "integer index")
newFunc("integer",	"llDetectedType", "integer index")
newFunc("vector",	"llDetectedVel", "integer index")


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


-- LSL collision / detect / sensor functions
function --[[integer]]	LSL.llDetectedGroup(		--[[integer]] index)	return detectedGroups		[index + 1] or false				end
function --[[vector]]	LSL.llDetectedGrab(		--[[integer]] index)	return detectedGrabs		[index + 1] or LSL.ZERO_VECTOR			end
function --[[key]]	LSL.llDetectedKey(		--[[integer]] index)	return detectedKeys		[index + 1] or LSL.NULL_KEY			end	-- LL says "returns empty key" which is "", but LSL Wiki says NULL_KEY
function --[[integer]]	LSL.llDetectedLinkNumber(	--[[integer]] index)	return detectedLinkNumbers	[index + 1] or 0				end
function --[[string]]	LSL.llDetectedName(		--[[integer]] index)	return detectedNames		[index + 1] or ""				end	-- LL says it returns NULL_KEY, LSL Wiki says an empty string.  Apparently there used to be an exploit for creating multi kb names, normally they are 255 characters.
function --[[key]]	LSL.llDetectedOwner(		--[[integer]] index)	return detectedOwners		[index + 1] or LSL.NULL_KEY			end	-- LL says "returns empty key" which is "", but LSL Wiki says NULL_KEY
function --[[vector]]	LSL.llDetectedPos(		--[[integer]] index)	return detectedPoss		[index + 1] or LSL.ZERO_VECTOR			end
function --[[rotation]]	LSL.llDetectedRot(		--[[integer]] index)	return detectedRots		[index + 1] or LSL.ZERO_ROTATION		end
function --[[vector]]	LSL.llDetectedTouchBinormal(	--[[integer]] index)	return detectedTouchBinormals	[index + 1] or LSL.TOUCH_INVALID_VECTOR		end
function --[[integer]]	LSL.llDetectedTouchFace(	--[[integer]] index)	return detectedTouchFaces	[index + 1] or LSL.TOUCH_INVALID_FACE		end
function --[[vector]]	LSL.llDetectedTouchNormal(	--[[integer]] index)	return detectedTouchNormals	[index + 1] or LSL.TOUCH_INVALID_VECTOR		end
function --[[vector]]	LSL.llDetectedTouchPos(		--[[integer]] index)	return detectedTouchPoss	[index + 1] or LSL.TOUCH_INVALID_VECTOR		end
function --[[vector]]	LSL.llDetectedTouchST(		--[[integer]] index)	return detectedTouchSTs		[index + 1] or LSL.TOUCH_INVALID_TEXCOORD	end
function --[[vector]]	LSL.llDetectedTouchUV(		--[[integer]] index)	return detectedTouchUVs		[index + 1] or LSL.TOUCH_INVALID_TEXCOORD	end
function --[[integer]]	LSL.llDetectedType(		--[[integer]] index)	return detectedTypes		[index + 1] or 0				end
function --[[vector]]	LSL.llDetectedVel(		--[[integer]] index)	return detectedVels		[index + 1] or LSL.ZERO_VECTOR			end


-- LSL list functions.

--function --[[list]]	LSL.llCSV2List(--[[string]] text) return {} end;
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

--function --[[integer]] 	LSL.llListFindList(--[[list]] l, --[[list]] l1) return 0 end;
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

--function --[[list]]	LSL.llParseString2List(--[[string]] In, --[[list]] l, --[[list]] l1) return {} end;
--function --[[list]]	LSL.llParseStringKeepNulls(--[[string]] In, --[[list]] l, --[[list]] l1) return {} end;


-- LSL script functions

function --[[string]] LSL.llGetScriptName()
  return scriptName
end


-- LSL string functions

function --[[string]] LSL.llGetSubString(--[[string]] text, --[[integer]] start, --[[integer]] End)
  -- Deal with the impedance mismatch.
  if 0 <= start  then  start = start + 1  end
  if 0 <= End    then  End   = End   + 1  end
-- TODO - If start is larger than end the substring is the exclusion of the entries, so 6,4 would give the entire string except for the 5th character.
  return string.sub(text, start, End)
end

function --[[integer]] LSL.llSubStringIndex(--[[string]] text, --[[string]] sub)
  local start, End = string.find(text, sub, 1, true)

  if nil == start then return -1 else return start - 1 end
end


-- Crements stuff.

function LSL.preDecrement(name) _G[name] = _G[name] - 1; return _G[name]; end;
function LSL.preIncrement(name) _G[name] = _G[name] + 1; return _G[name]; end;
function LSL.postDecrement(name) local temp = _G[name]; _G[name] = _G[name] - 1; return temp; end;
function LSL.postIncrement(name) local temp = _G[name]; _G[name] = _G[name] + 1; return temp; end;


-- State stuff

function LSL.stateChange(x)
  if currentState ~= x then  -- Changing to the same state is a NOP.
    -- TODO - Should clear out pending events, except timer()
    -- Also forget about any event setup that needs setting up via some ll*() function, except timer().
    if nil ~= currentState.state_exit then
      currentState.state_exit();
    end
    msg("LSL.Lua: State change on " .. scriptName)
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
  local status, errorMsg
  local result

  SID = sid
  scriptName = name
  LSL.EOF = "\n\n\n"	-- Fix this up now.

  LSL.stateChange(x);
  waitAndProcess(false)
  msg("LSL.Lua: Script quitting " .. scriptName)
end

function waitAndProcess(returnWanted)
  local Type = "event"

  if returnWanted then Type = "result" end
  while running do
    local message = luaproc.receive(SID)
    if message then
      -- TODO - should we be discarding return values while paused?  I don't think so, so we need to process those,
      if paused then
	if "start()" == message then paused = false  end
      else
	result, errorMsg = loadstring(message)  -- "The environment of the returned function is the global environment."  Though normally, a function inherits it's environment from the function creating it.  Which is what we want.  lol
	if nil == result then
	  msg("Not a valid " .. Type .. ": " .. message .. "  ERROR MESSAGE: " .. errorMsg)
	else
	  -- Set the functions environment to ours, for the protection of the script, coz loadstring sets it to the global environment instead.
	  -- TODO - On the other hand, we will need the global environment when we call event handlers.  So we should probably stash it around here somewhere.
	  --        Meh, seems to be working fine as it is.
	  setfenv(result, getfenv(1))
	  status, result = pcall(result)
	  if not status then
	    msg("Error from " .. Type .. ": " .. message .. "  ERROR MESSAGE: " .. result)
	  elseif result then
	    -- Check if we are waiting for a return, and got it.
	    if returnWanted and string.match(message, "^return ") then
	      return result
            end
	    -- Otherwise, just run it and keep looping.
	    status, errorMsg = luaproc.send(sid, result)
	    if not status then
	      msg("Error sending results from " .. Type .. ": " .. message .. "  ERROR MESSAGE: " .. errorMsg)
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
