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
  print(SID, scriptName, ...)  -- The comma adds a tab, fancy that.  B-)
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
--  elseif "boolean"	== Type then if value then temp = '"true"' else temp = '"false"' end
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
  Runnr.send(nil, name .. "(" .. args2string(true, ...) .. ")")
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

  newConst("integer", "AGENT",			0x0001),
  newConst("integer", "AGENT_BY_LEGACY_NAME",	0x0001),
  newConst("integer", "ACTIVE",			0x0002),
  newConst("integer", "PASSIVE",		0x0004),
  newConst("integer", "SCRIPTED",		0x0008),
  newConst("integer", "AGENT_BY_USERNAME",	0x0010),
  newConst("integer", "NPC",			0x0020),
  newConst("integer", "OS_NPC",			0x01000000),

  newConst("integer", "AGENT_FLYING",		0x0001),
  newConst("integer", "AGENT_ATTACHMENTS",	0x0002),
  newConst("integer", "AGENT_SCRIPTED",		0x0004),
  newConst("integer", "AGENT_MOUSELOOK",	0x0008),
  newConst("integer", "AGENT_SITTING",		0x0010),
  newConst("integer", "AGENT_ON_OBJECT",	0x0020),
  newConst("integer", "AGENT_AWAY",		0x0040),
  newConst("integer", "AGENT_WALKING",		0x0080),
  newConst("integer", "AGENT_IN_AIR",		0x0100),
  newConst("integer", "AGENT_TYPING",		0x0200),
  newConst("integer", "AGENT_CROUCHING",	0x0400),
  newConst("integer", "AGENT_BUSY",		0x0800),
  newConst("integer", "AGENT_ALWAYS_RUN",	0x1000),

  newConst("integer", "AGENT_LIST_PARCEL",	0x0001),
  newConst("integer", "AGENT_LIST_PARCEL_OWNER",0x0002),
  newConst("integer", "AGENT_LIST_REGION",	0x0004),

  newConst("integer", "ANIM_ON",		0x0001),
  newConst("integer", "LOOP",			0x0002),
  newConst("integer", "REVERSE",		0x0004),
  newConst("integer", "PING_PONG",		0x0008),
  newConst("integer", "SMOOTH",			0x0010),
  newConst("integer", "ROTATE",			0x0020),
  newConst("integer", "SCALE",			0x0040),

  newConst("integer", "ATTACH_CHEST",		1),
  newConst("integer", "ATTACH_HEAD",		2),
  newConst("integer", "ATTACH_LSHOULDER",	3),
  newConst("integer", "ATTACH_RSHOULDER",	4),
  newConst("integer", "ATTACH_LHAND",		5),
  newConst("integer", "ATTACH_RHAND",		6),
  newConst("integer", "ATTACH_LFOOT",		7),
  newConst("integer", "ATTACH_RFOOT",		8),
  newConst("integer", "ATTACH_BACK",		9),
  newConst("integer", "ATTACH_PELVIS",		10),
  newConst("integer", "ATTACH_MOUTH",		11),
  newConst("integer", "ATTACH_CHIN",		12),
  newConst("integer", "ATTACH_LEAR",		13),
  newConst("integer", "ATTACH_REAR",		14),
  newConst("integer", "ATTACH_LEYE",		15),
  newConst("integer", "ATTACH_REYE",		16),
  newConst("integer", "ATTACH_NOSE",		17),
  newConst("integer", "ATTACH_RUARM",		18),
  newConst("integer", "ATTACH_RLARM",		19),
  newConst("integer", "ATTACH_LUARM",		20),
  newConst("integer", "ATTACH_LLARM",		21),
  newConst("integer", "ATTACH_RHIP",		22),
  newConst("integer", "ATTACH_RULEG",		23),
  newConst("integer", "ATTACH_RLLEG",		24),
  newConst("integer", "ATTACH_LHIP",		25),
  newConst("integer", "ATTACH_LULEG",		26),
  newConst("integer", "ATTACH_LLLEG",		27),
  newConst("integer", "ATTACH_BELLY",		28),
  newConst("integer", "ATTACH_RPEC",		29),
  newConst("integer", "ATTACH_RIGHT_PEC",	29),
  newConst("integer", "ATTACH_LPEC",		30),
  newConst("integer", "ATTACH_LEFT_PEC",	30),
  newConst("integer", "ATTACH_HUD_CENTER_2",	31),
  newConst("integer", "ATTACH_HUD_TOP_RIGHT",	32),
  newConst("integer", "ATTACH_HUD_TOP_CENTER",	33),
  newConst("integer", "ATTACH_HUD_TOP_LEFT",	34),
  newConst("integer", "ATTACH_HUD_CENTER_1",	35),
  newConst("integer", "ATTACH_HUD_BOTTOM_LEFT",	36),
  newConst("integer", "ATTACH_HUD_BOTTOM",	37),
  newConst("integer", "ATTACH_HUD_BOTTOM_RIGHT",38),

  newConst("integer", "CAMERA_PITCH",			0),
  newConst("integer", "CAMERA_FOCUS_OFFSET",		1),
  newConst("integer", "CAMERA_FOCUS_OFFSET_X",		2),
  newConst("integer", "CAMERA_FOCUS_OFFSET_Y",		3),
  newConst("integer", "CAMERA_FOCUS_OFFSET_Z",		4),
  newConst("integer", "CAMERA_POSITION_LAG",		5),
  newConst("integer", "CAMERA_FOCUS_LAG",		6),
  newConst("integer", "CAMERA_DISTANCE",		7),
  newConst("integer", "CAMERA_BEHINDNESS_ANGLE",	8),
  newConst("integer", "CAMERA_BEHINDNESS_LAG",		9),
  newConst("integer", "CAMERA_POSITION_THRESHOLD",	10),
  newConst("integer", "CAMERA_FOCUS_THRESHOLD",		11),
  newConst("integer", "CAMERA_ACTIVE",			12),
  newConst("integer", "CAMERA_POSITION",		13),
  newConst("integer", "CAMERA_POSITION_X",		14),
  newConst("integer", "CAMERA_POSITION_Y",		15),
  newConst("integer", "CAMERA_POSITION_Z",		16),
  newConst("integer", "CAMERA_FOCUS",			17),
  newConst("integer", "CAMERA_FOCUS_X",			18),
  newConst("integer", "CAMERA_FOCUS_Y",			19),
  newConst("integer", "CAMERA_FOCUS_Z",			20),
  newConst("integer", "CAMERA_POSITION_LOCKED",		21),
  newConst("integer", "CAMERA_FOCUS_LOCKED",		22),

  newConst("integer", "CHANGED_INVENTORY",	0x0001),
  newConst("integer", "CHANGED_COLOR",		0x0002),
  newConst("integer", "CHANGED_SHAPE",		0x0004),
  newConst("integer", "CHANGED_SCALE",		0x0008),
  newConst("integer", "CHANGED_TEXTURE",	0x0010),
  newConst("integer", "CHANGED_LINK",		0x0020),
  newConst("integer", "CHANGED_ALLOWED_DROP",	0x0040),
  newConst("integer", "CHANGED_OWNER",		0x0080),
  newConst("integer", "CHANGED_REGION",		0x0100),
  newConst("integer", "CHANGED_TELEPORT",	0x0200),
  newConst("integer", "CHANGED_REGION_RESTART",	0x0400),
  newConst("integer", "CHANGED_REGION_START",	0x0400),
  newConst("integer", "CHANGED_MEDIA",		0x0800),
  newConst("integer", "CHANGED_ANIMATION",	0x4000),

  newConst("integer", "CLICK_ACTION_NONE",	0),
  newConst("integer", "CLICK_ACTION_TOUCH",	0),
  newConst("integer", "CLICK_ACTION_SIT",	1),
  newConst("integer", "CLICK_ACTION_BUY",	2),
  newConst("integer", "CLICK_ACTION_PAY",	3),
  newConst("integer", "CLICK_ACTION_OPEN",	4),
  newConst("integer", "CLICK_ACTION_PLAY",	5),
  newConst("integer", "CLICK_ACTION_OPEN_MEDIA",6),
  newConst("integer", "CLICK_ACTION_ZOOM",	7),

  newConst("integer", "CONTROL_FWD",		0x0001),
  newConst("integer", "CONTROL_BACK",		0x0002),
  newConst("integer", "CONTROL_LEFT",		0x0004),
  newConst("integer", "CONTROL_RIGHT",		0x0008),
  newConst("integer", "CONTROL_UP",		0x0010),
  newConst("integer", "CONTROL_DOWN",		0x0020),
  newConst("integer", "CONTROL_ROT_LEFT",	0x0100),
  newConst("integer", "CONTROL_ROT_RIGHT",	0x0200),
  newConst("integer", "CONTROL_LBUTTON",	0x10000000),
  newConst("integer", "CONTROL_ML_LBUTTON",	0x40000000),

  newConst("integer", "DATA_ONLINE",		1),
  newConst("integer", "DATA_NAME",		2),
  newConst("integer", "DATA_BORN",		3),
  newConst("integer", "DATA_RATING",		4),
  newConst("integer", "DATA_SIM_POS",		5),
  newConst("integer", "DATA_SIM_STATUS",	6),
  newConst("integer", "DATA_SIM_RATING",	7),
  newConst("integer", "DATA_PAYINFO",		8),
  newConst("integer", "DATA_SIM_RELEASE",	128),

  newConst("integer", "DEBUG_CHANNEL",		2147483647),
  newConst("integer", "PUBLIC_CHANNEL",		0),

  newConst("integer", "ESTATE_ACCESS_ALLOWED_AGENT_ADD",	0),
  newConst("integer", "ESTATE_ACCESS_ALLOWED_AGENT_REMOVE",	1),
  newConst("integer", "ESTATE_ACCESS_ALLOWED_GROUP_ADD",	2),
  newConst("integer", "ESTATE_ACCESS_ALLOWED_GROUP_REMOVE",	3),
  newConst("integer", "ESTATE_ACCESS_BANNED_AGENT_ADD",		4),
  newConst("integer", "ESTATE_ACCESS_BANNED_AGENT_REMOVE",	5),

  newConst("integer", "HTTP_METHOD",		0),
  newConst("integer", "HTTP_MIMETYPE",		1),
  newConst("integer", "HTTP_BODY_MAXLENGTH",	2),
  newConst("integer", "HTTP_VERIFY_CERT",	3),

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

  newConst("integer", "LAND_LEVEL",		0),
  newConst("integer", "LAND_RAISE",		1),
  newConst("integer", "LAND_LOWER",		2),
  newConst("integer", "LAND_SMOOTH",		3),
  newConst("integer", "LAND_NOISE",		4),
  newConst("integer", "LAND_REVERT",		5),

  newConst("integer", "LAND_SMALL_BRUSH",	1),
  newConst("integer", "LAND_MEDIUM_BRUSH",	2),
  newConst("integer", "LAND_LARGE_BRUSH",	3),

  newConst("integer", "ALL_SIDES",		-1),
  newConst("integer", "LINK_ROOT",		1),
  newConst("integer", "LINK_SET",		-1),
  newConst("integer", "LINK_ALL_OTHERS",	-2),
  newConst("integer", "LINK_ALL_CHILDREN",	-3),
  newConst("integer", "LINK_THIS",		-4),

  newConst("integer", "LIST_STAT_RANGE",		0),
  newConst("integer", "LIST_STAT_MIN",			1),
  newConst("integer", "LIST_STAT_MAX",			2),
  newConst("integer", "LIST_STAT_MEAN",			3),
  newConst("integer", "LIST_STAT_MEDIAN",		4),
  newConst("integer", "LIST_STAT_STD_DEV",		5),
  newConst("integer", "LIST_STAT_SUM",			6),
  newConst("integer", "LIST_STAT_SUM_SQUARES",		7),
  newConst("integer", "LIST_STAT_NUM_COUNT",		8),
  newConst("integer", "LIST_STAT_GEOMETRIC_MEAN",	9),
  newConst("integer", "LIST_STAT_HARMONIC_MEAN",	100),

  newConst("integer", "LSL_STATUS_OK",			0),
  newConst("integer", "LSL_STATUS_MALFORMED_PARAMS",	1000),
  newConst("integer", "LSL_STATUS_TYPE_MISMATCH",	1001),
  newConst("integer", "LSL_STATUS_BOUNDS_ERROR",	1002),
  newConst("integer", "LSL_STATUS_NOT_FOUND",		1003),
  newConst("integer", "LSL_STATUS_NOT_SUPPORTED",	1004),
  newConst("integer", "LSL_STATUS_INTERNAL_ERROR",	1999),
  newConst("integer", "LSL_STATUS_WHITELIST_FAILED",	2001),

  newConst("integer", "PAY_HIDE",		-1),
  newConst("integer", "PAY_DEFAULT",		-2),

  newConst("integer", "PERM_ALL",		0x7FFFFFFF),
  newConst("integer", "PERM_TRANSFER",		0x00002000),
  newConst("integer", "PERM_COPY",		0x00008000),
  newConst("integer", "PERM_MODIFY",		0x00004000),
  newConst("integer", "PERM_MOVE",		0x00080000),
  newConst("integer", "MASK_BASE",		0),
  newConst("integer", "MASK_OWNER",		1),
  newConst("integer", "MASK_GROUP",		2),
  newConst("integer", "MASK_EVERYONE",		3),
  newConst("integer", "MASK_NEXT",		4),

  newConst("integer", "PERMISSION_DEBIT",		0x0002),
  newConst("integer", "PERMISSION_TAKE_CONTROLS",	0x0004),
  newConst("integer", "PERMISSION_REMAP_CONTROLS",	0x0008),
  newConst("integer", "PERMISSION_TRIGGER_ANIMATION",	0x0010),
  newConst("integer", "PERMISSION_ATTACH",		0x0020),
  newConst("integer", "PERMISSION_RELEASE_OWNERSHIP",	0x0040),
  newConst("integer", "PERMISSION_CHANGE_LINKS",	0x0080),
  newConst("integer", "PERMISSION_CHANGE_JOINTS",	0x0100),
  newConst("integer", "PERMISSION_CHANGE_PERMISSIONS",	0x0200),
  newConst("integer", "PERMISSION_TRACK_CAMERA",	0x0400),
  newConst("integer", "PERMISSION_CONTROL_CAMERA",	0x0800),

  newConst("integer", "OBJECT_UNKNOWN_DETAIL",		-1),
  newConst("integer", "OBJECT_NAME",			1),
  newConst("integer", "OBJECT_DESC",			2),
  newConst("integer", "OBJECT_POS",			3),
  newConst("integer", "OBJECT_ROT",			4),
  newConst("integer", "OBJECT_VELOCITY",		5),
  newConst("integer", "OBJECT_OWNER",			6),
  newConst("integer", "OBJECT_GROUP",			7),
  newConst("integer", "OBJECT_CREATOR",			8),
  newConst("integer", "OBJECT_RUNNING_SCRIPT_COUNT",	9),
  newConst("integer", "OBJECT_TOTAL_SCRIPT_COUNT",	10),
  newConst("integer", "OBJECT_SCRIPT_MEMORY",		11),
  newConst("integer", "OBJECT_SCRIPT_TIME",		12),
  newConst("integer", "OBJECT_PRIM_EQUIVALENCE",	13),
  newConst("integer", "OBJECT_SERVER_COST",		14),
  newConst("integer", "OBJECT_STREAMING_COST",		15),
  newConst("integer", "OBJECT_PHYSICS_COST",		16),
  newConst("integer", "OBJECT_CHARACTER_TIME",		17),
  newConst("integer", "OBJECT_ROOT",			18),
  newConst("integer", "OBJECT_ATTACHED_POINT",		19),
  newConst("integer", "OBJECT_PATHFINDING_TYPE",	20),
  newConst("integer", "OBJECT_PHYSICS",			21),
  newConst("integer", "OBJECT_PHANTOM",			22),
  newConst("integer", "OBJECT_TEMP_ON_REZ",		23),

  newConst("integer", "OPT_OTHER",		-1),
  newConst("integer", "OPT_LEGACY_LINKSET",	0),
  newConst("integer", "OPT_AVATAR",		1),
  newConst("integer", "OPT_CHARACTER",		2),
  newConst("integer", "OPT_WALKABLE",		3),
  newConst("integer", "OPT_STATIC_OBSTACLE",	4),
  newConst("integer", "OPT_MATERIAL_VOLUME",	5),
  newConst("integer", "OPT_EXCLUSION_VOLUME",	6),

  newConst("integer", "OS_ATTACH_MSG_ALL",		-65535),
  newConst("integer", "OS_ATTACH_MSG_INVERT_POINTS",	0x0001),
  newConst("integer", "OS_ATTACH_MSG_OBJECT_CREATOR",	0x0002),
  newConst("integer", "OS_ATTACH_MSG_SCRIPT_CREATOR",	0x0004),

  newConst("integer", "OS_LISTEN_REGEX_NAME",		0x1),
  newConst("integer", "OS_LISTEN_REGEX_MESSAGE",	0x2),

  newConst("integer", "OS_NPC_SIT_NOW",		0),
  newConst("integer", "OS_NPC_FLY",		0),
  newConst("integer", "OS_NPC_NO_FLY",		1),
  newConst("integer", "OS_NPC_LAND_AT_TARGET",	2),
  newConst("integer", "OS_NPC_RUNNING",		4),
  newConst("integer", "OS_NPC_CREATOR_OWNED",	0x0001),
  newConst("integer", "OS_NPC_NOT_OWNED",	0x0002),
  newConst("integer", "OS_NPC_SENSE_AS_AGENT",	0x0004),

  newConst("integer", "PARCEL_COUNT_TOTAL",	0),
  newConst("integer", "PARCEL_COUNT_OWNER",	1),
  newConst("integer", "PARCEL_COUNT_GROUP",	2),
  newConst("integer", "PARCEL_COUNT_OTHER",	3),
  newConst("integer", "PARCEL_COUNT_SELECTED",	4),
  newConst("integer", "PARCEL_COUNT_TEMP",	5),

  newConst("integer", "PARCEL_DETAILS_NAME",		0),
  newConst("integer", "PARCEL_DETAILS_DESC",		1),
  newConst("integer", "PARCEL_DETAILS_OWNER",		2),
  newConst("integer", "PARCEL_DETAILS_GROUP",		3),
  newConst("integer", "PARCEL_DETAILS_AREA",		4),
  newConst("integer", "PARCEL_DETAILS_ID",		5),
  newConst("integer", "PARCEL_DETAILS_SEE_AVATARS",	6),
  newConst("integer", "PARCEL_DETAILS_CLAIMDATE",	10),

  newConst("integer", "PARCEL_FLAG_ALLOW_FLY",			0x0001),
  newConst("integer", "PARCEL_FLAG_ALLOW_SCRIPTS",		0x0002),
  newConst("integer", "PARCEL_FLAG_ALLOW_LANDMARK",		0x0008),
  newConst("integer", "PARCEL_FLAG_ALLOW_TERRAFORM",		0x0010),
  newConst("integer", "PARCEL_FLAG_ALLOW_DAMAGE",		0x0020),
  newConst("integer", "PARCEL_FLAG_ALLOW_CREATE_OBJECTS",	0x0040),
  newConst("integer", "PARCEL_FLAG_USE_ACCESS_GROUP",		0x0100),
  newConst("integer", "PARCEL_FLAG_USE_ACCESS_LIST",		0x0200),
  newConst("integer", "PARCEL_FLAG_USE_BAN_LIST",		0x0400),
  newConst("integer", "PARCEL_FLAG_USE_LAND_PASS_LIST",		0x0800),
  newConst("integer", "PARCEL_FLAG_LOCAL_SOUND_ONLY",		0x8000),
  newConst("integer", "PARCEL_FLAG_RESTRICT_PUSHOBJECT",	0x00200000),
  newConst("integer", "PARCEL_FLAG_ALLOW_GROUP_SCRIPTS",	0x02000000),
  newConst("integer", "PARCEL_FLAG_ALLOW_CREATE_GROUP_OBJECTS",	0x04000000),
  newConst("integer", "PARCEL_FLAG_ALLOW_ALL_OBJECT_ENTRY",	0x08000000),
  newConst("integer", "PARCEL_FLAG_ALLOW_GROUP_OBJECT_ENTRY",	0x10000000),

  newConst("integer", "PARCEL_MEDIA_COMMAND_STOP",	0),
  newConst("integer", "PARCEL_MEDIA_COMMAND_PAUSE",	1),
  newConst("integer", "PARCEL_MEDIA_COMMAND_PLAY",	2),
  newConst("integer", "PARCEL_MEDIA_COMMAND_LOOP",	3),
  newConst("integer", "PARCEL_MEDIA_COMMAND_TEXTURE",	4),
  newConst("integer", "PARCEL_MEDIA_COMMAND_URL",	5),
  newConst("integer", "PARCEL_MEDIA_COMMAND_TIME",	6),
  newConst("integer", "PARCEL_MEDIA_COMMAND_AGENT",	7),
  newConst("integer", "PARCEL_MEDIA_COMMAND_UNLOAD",	8),
  newConst("integer", "PARCEL_MEDIA_COMMAND_AUTO_ALIGN",9),
  newConst("integer", "PARCEL_MEDIA_COMMAND_TYPE",	10),
  newConst("integer", "PARCEL_MEDIA_COMMAND_SIZE",	11),
  newConst("integer", "PARCEL_MEDIA_COMMAND_DESC",	12),

  newConst("integer", "PRIM_TYPE_OLD",		1),
  newConst("integer", "PRIM_MATERIAL",		2),
  newConst("integer", "PRIM_PHYSICS",		3),
  newConst("integer", "PRIM_TEMP_ON_REZ",	4),
  newConst("integer", "PRIM_PHANTOM",		5),
  newConst("integer", "PRIM_POSITION",		6),
  newConst("integer", "PRIM_SIZE",		7),
  newConst("integer", "PRIM_ROTATION",		8),
  newConst("integer", "PRIM_TYPE",		9),
  newConst("integer", "PRIM_TEXTURE",		17),
  newConst("integer", "PRIM_COLOR",		18),
  newConst("integer", "PRIM_BUMP_SHINY",	19),
  newConst("integer", "PRIM_FULLBRIGHT",	20),
  newConst("integer", "PRIM_FLEXIBLE",		21),
  newConst("integer", "PRIM_TEXGEN",		22),
  newConst("integer", "PRIM_POINT_LIGHT",	23),
  newConst("integer", "PRIM_CAST_SHADOWS",	24),
  newConst("integer", "PRIM_GLOW",		25),
  newConst("integer", "PRIM_TEXT",		26),
  newConst("integer", "PRIM_NAME",		27),
  newConst("integer", "PRIM_DESC",		28),
  newConst("integer", "PRIM_ROT_LOCAL",		29),
  newConst("integer", "PRIM_OMEGA",		32),
  newConst("integer", "PRIM_POS_LOCAL",		33),
  newConst("integer", "PRIM_LINK_TARGET",	34),
  newConst("integer", "PRIM_SLICE",		35),

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

  newConst("integer", "PRIM_MEDIA_ALT_IMAGE_ENABLE",	0),
  newConst("integer", "PRIM_MEDIA_CONTROLS",		1),
  newConst("integer", "PRIM_MEDIA_CURRENT_URL",		2),
  newConst("integer", "PRIM_MEDIA_HOME_URL",		3),
  newConst("integer", "PRIM_MEDIA_AUTO_LOOP",		4),
  newConst("integer", "PRIM_MEDIA_AUTO_PLAY",		5),
  newConst("integer", "PRIM_MEDIA_AUTO_SCALE",		6),
  newConst("integer", "PRIM_MEDIA_AUTO_ZOOM",		7),
  newConst("integer", "PRIM_MEDIA_FIRST_CLICK_INTERACT",8),
  newConst("integer", "PRIM_MEDIA_WIDTH_PIXELS",	9),
  newConst("integer", "PRIM_MEDIA_HEIGHT_PIXELS",	10),
  newConst("integer", "PRIM_MEDIA_WHITELIST_ENABLE",	11),
  newConst("integer", "PRIM_MEDIA_WHITELIST",		12),
  newConst("integer", "PRIM_MEDIA_PERMS_INTERACT",	13),
  newConst("integer", "PRIM_MEDIA_PERMS_CONTROL",	14),

  newConst("integer", "PRIM_MEDIA_CONTROLS_STANDARD",	0),
  newConst("integer", "PRIM_MEDIA_CONTROLS_MINI",	1),

  newConst("integer", "PRIM_MEDIA_PERM_NONE",		0),
  newConst("integer", "PRIM_MEDIA_PERM_OWNER",		1),
  newConst("integer", "PRIM_MEDIA_PERM_GROUP",		2),
  newConst("integer", "PRIM_MEDIA_PERM_ANYONE",		4),

  newConst("integer", "PRIM_SCULPT_TYPE_SPHERE",	1),
  newConst("integer", "PRIM_SCULPT_TYPE_TORUS",		2),
  newConst("integer", "PRIM_SCULPT_TYPE_PLANE",		3),
  newConst("integer", "PRIM_SCULPT_TYPE_CYLINDER",	4),
  newConst("integer", "PRIM_SCULPT_TYPE_MESH",		5),
  newConst("integer", "PRIM_SCULPT_TYPE_MIMESH",	6),

  newConst("integer", "PRIM_SCULPT_FLAG_INVERT",	0x0040),
  newConst("integer", "PRIM_SCULPT_FLAG_MIRROR",	0x0080),

  newConst("integer", "PRIM_SHINY_NONE",	0),
  newConst("integer", "PRIM_SHINY_LOW",		1),
  newConst("integer", "PRIM_SHINY_MEDIUM",	2),
  newConst("integer", "PRIM_SHINY_HIGH",	3),

  newConst("integer", "PRIM_TEXGEN_DEFAULT",	0),
  newConst("integer", "PRIM_TEXGEN_PLANAR",	1),

  newConst("integer", "PRIM_TYPE_BOX",		0),
  newConst("integer", "PRIM_TYPE_CYLINDER",	1),
  newConst("integer", "PRIM_TYPE_PRISM",	2),
  newConst("integer", "PRIM_TYPE_SPHERE",	3),
  newConst("integer", "PRIM_TYPE_TORUS",	4),
  newConst("integer", "PRIM_TYPE_TUBE",		5),
  newConst("integer", "PRIM_TYPE_RING",		6),
  newConst("integer", "PRIM_TYPE_SCULPT",	7),

  newConst("integer", "PROFILE_NONE",		0),
  newConst("integer", "PROFILE_SCRIPT_MEMORY",	1),

  newConst("integer", "PSYS_PART_FLAGS",		0),
  newConst("integer", "PSYS_PART_START_COLOR",		1),
  newConst("integer", "PSYS_PART_START_ALPHA",		2),
  newConst("integer", "PSYS_PART_END_COLOR",		3),
  newConst("integer", "PSYS_PART_END_ALPHA",		4),
  newConst("integer", "PSYS_PART_START_SCALE",		5),
  newConst("integer", "PSYS_PART_END_SCALE",		6),
  newConst("integer", "PSYS_PART_MAX_AGE",		7),
  newConst("integer", "PSYS_SRC_ACCEL",			8),
  newConst("integer", "PSYS_SRC_PATTERN",		9),
  newConst("integer", "PSYS_SRC_INNERANGLE",		10),
  newConst("integer", "PSYS_SRC_OUTERANGLE",		11),
  newConst("integer", "PSYS_SRC_TEXTURE",		12),
  newConst("integer", "PSYS_SRC_BURST_RATE",		13),
  newConst("integer", "PSYS_SRC_BURST_PART_COUNT",	15),
  newConst("integer", "PSYS_SRC_BURST_RADIUS",		16),
  newConst("integer", "PSYS_SRC_BURST_SPEED_MIN",	17),
  newConst("integer", "PSYS_SRC_BURST_SPEED_MAX",	18),
  newConst("integer", "PSYS_SRC_MAX_AGE",		19),
  newConst("integer", "PSYS_SRC_TARGET_KEY",		20),
  newConst("integer", "PSYS_SRC_OMEGA",			21),
  newConst("integer", "PSYS_SRC_ANGLE_BEGIN",		22),
  newConst("integer", "PSYS_SRC_ANGLE_END",		23),

  newConst("integer", "PSYS_PART_INTERP_COLOR_MASK",	0x0001),
  newConst("integer", "PSYS_PART_INTERP_SCALE_MASK",	0x0002),
  newConst("integer", "PSYS_PART_BOUNCE_MASK",		0x0004),
  newConst("integer", "PSYS_PART_WIND_MASK",		0x0008),
  newConst("integer", "PSYS_PART_FOLLOW_SRC_MASK",	0x0010),
  newConst("integer", "PSYS_PART_FOLLOW_VELOCITY_MASK",	0x0020),
  newConst("integer", "PSYS_PART_TARGET_POS_MASK",	0x0040),
  newConst("integer", "PSYS_PART_TARGET_LINEAR_MASK",	0x0080),
  newConst("integer", "PSYS_PART_EMISSIVE_MASK",	0x0100),

  newConst("integer", "PSYS_SRC_PATTERN_DROP",			0x0001),
  newConst("integer", "PSYS_SRC_PATTERN_EXPLODE",		0x0002),
  newConst("integer", "PSYS_SRC_PATTERN_ANGLE",			0x0004),
  newConst("integer", "PSYS_SRC_PATTERN_ANGLE_CONE",		0x0008),
  newConst("integer", "PSYS_SRC_PATTERN_ANGLE_CONE_EMPTY",	0x0010),

  newConst("integer", "RC_REJECT_TYPES",	0),
  newConst("integer", "RC_DETECT_PHANTOM",	1),
  newConst("integer", "RC_DATA_FLAGS",		2),
  newConst("integer", "RC_MAX_HITS",		3),

  newConst("integer", "RC_REJECT_AGENTS",	1),
  newConst("integer", "RC_REJECT_PHYSICAL",	2),
  newConst("integer", "RC_REJECT_NONPHYSICAL",	4),
  newConst("integer", "RC_REJECT_LAND",		8),

  newConst("integer", "RC_GET_NORMAL",		1),
  newConst("integer", "RC_GET_ROOT_KEY",	2),
  newConst("integer", "RC_GET_LINK_NUM",	4),

  newConst("integer", "RCERR_UNKNOWN",		-1),
  newConst("integer", "RCERR_SIM_PERF_LOW",	-2),
  newConst("integer", "RCERR_CAST_TIME_EXCEEDED",3),

  newConst("integer", "REGION_FLAG_ALLOW_DAMAGE",		0x0001),
  newConst("integer", "REGION_FLAG_FIXED_SUN",			0x0010),
  newConst("integer", "REGION_FLAG_BLOCK_TERRAFORM",		0x0040),
  newConst("integer", "REGION_FLAG_SANDBOX",			0x0100),
  newConst("integer", "REGION_FLAG_DISABLE_COLLISIONS",		0x1000),
  newConst("integer", "REGION_FLAG_DISABLE_PHYSICS",		0x4000),
  newConst("integer", "REGION_FLAG_BLOCK_FLY",			0x00080000),
  newConst("integer", "REGION_FLAG_ALLOW_DIRECT_TELEPORT",	0x00100000),
  newConst("integer", "REGION_FLAG_RESTRICT_PUSHOBJECT",	0x00400000),

  newConst("integer", "REMOTE_DATA_CHANNEL",	1),
  newConst("integer", "REMOTE_DATA_REQUEST",	2),
  newConst("integer", "REMOTE_DATA_REPLY",	3),

  newConst("integer", "STATS_TIME_DILATION",		0),
  newConst("integer", "STATS_SIM_FPS",			1),
  newConst("integer", "STATS_PHYSICS_FPS",		2),
  newConst("integer", "STATS_AGENT_UPDATES",		3),
  newConst("integer", "STATS_ROOT_AGENTS",		4),
  newConst("integer", "STATS_CHILD_AGENTS",		5),
  newConst("integer", "STATS_TOTAL_PRIMS",		6),
  newConst("integer", "STATS_ACTIVE_PRIMS",		7),
  newConst("integer", "STATS_FRAME_MS",			8),
  newConst("integer", "STATS_NET_MS",			9),
  newConst("integer", "STATS_PHYSICS_MS",		10),
  newConst("integer", "STATS_IMAGE_MS",			11),
  newConst("integer", "STATS_OTHER_MS",			12),
  newConst("integer", "STATS_IN_PACKETS_PER_SECOND",	13),
  newConst("integer", "STATS_OUT_PACKETS_PER_SECOND",	14),
  newConst("integer", "STATS_UNACKED_BYTES",		15),
  newConst("integer", "STATS_AGENT_MS",			16),
  newConst("integer", "STATS_PENDING_DOWNLOADS",	17),
  newConst("integer", "STATS_PENDING_UPLOADS",		18),
  newConst("integer", "STATS_ACTIVE_SCRIPTS",		19),
  newConst("integer", "STATS_SCRIPT_LPS",		20),

  newConst("integer", "STATUS_PHYSICS",		0x0001),
  newConst("integer", "STATUS_ROTATE_X",	0x0002),
  newConst("integer", "STATUS_ROTATE_Y",	0x0004),
  newConst("integer", "STATUS_ROTATE_Z",	0x0008),
  newConst("integer", "STATUS_PHANTOM",		0x0010),
  newConst("integer", "STATUS_SANDBOX",		0x0020),
  newConst("integer", "STATUS_BLOCK_GRAB",	0x0040),
  newConst("integer", "STATUS_DIE_AT_EDGE",	0x0080),
  newConst("integer", "STATUS_RETURN_AT_EDGE",	0x0100),
  newConst("integer", "STATUS_CAST_SHADOWS",	0x0200),

  newConst("integer", "STRING_TRIM_HEAD",	1),
  newConst("integer", "STRING_TRIM_TAIL",	2),
  newConst("integer", "STRING_TRIM",		3),

  newConst("integer", "TOUCH_INVALID_FACE",	0x7FFFFFFF),
  newConst("vector",  "TOUCH_INVALID_TEXCOORD",	{x=-1.0, y=-1.0, z=0.0}),
  newConst("vector",  "TOUCH_INVALID_VECTOR",	{x=0.0, y=0.0, z=0.0}),

  newConst("integer", "TYPE_INVALID",		0),
  newConst("integer", "TYPE_INTEGER",		1),
  newConst("integer", "TYPE_FLOAT",		2),
  newConst("integer", "TYPE_STRING",		3),
  newConst("integer", "TYPE_KEY",		4),
  newConst("integer", "TYPE_VECTOR",		5),
  newConst("integer", "TYPE_ROTATION",		6),

  newConst("integer", "VEHICLE_FLAG_NO_DEFLECTION_UP",		0x0001),
  newConst("integer", "VEHICLE_FLAG_LIMIT_ROLL_ONLY",		0x0002),
  newConst("integer", "VEHICLE_FLAG_HOVER_WATER_ONLY",		0x0004),
  newConst("integer", "VEHICLE_FLAG_HOVER_TERRAIN_ONLY",	0x0008),
  newConst("integer", "VEHICLE_FLAG_HOVER_GLOBAL_HEIGHT",	0x0010),
  newConst("integer", "VEHICLE_FLAG_HOVER_UP_ONLY",		0x0020),
  newConst("integer", "VEHICLE_FLAG_LIMIT_MOTOR_UP",		0x0040),
  newConst("integer", "VEHICLE_FLAG_MOUSELOOK_STEER",		0x0080),
  newConst("integer", "VEHICLE_FLAG_MOUSELOOK_BANK",		0x0100),
  newConst("integer", "VEHICLE_FLAG_CAMERA_DECOUPLED",		0x0200),
  newConst("integer", "VEHICLE_FLAG_NO_X",			0x0400),
  newConst("integer", "VEHICLE_FLAG_NO_Y",			0x0800),
  newConst("integer", "VEHICLE_FLAG_NO_Z",			0x1000),
  newConst("integer", "VEHICLE_FLAG_LOCK_HOVER_HEIGHT",		0x2000),
  newConst("integer", "VEHICLE_FLAG_NO_DEFLECTION",		0x4000),
  newConst("integer", "VEHICLE_FLAG_LOCK_ROTATION",		0x8000),

  newConst("integer", "VEHICLE_TYPE_NONE",	0),
  newConst("integer", "VEHICLE_TYPE_SLED",	1),
  newConst("integer", "VEHICLE_TYPE_CAR",	2),
  newConst("integer", "VEHICLE_TYPE_BOAT",	3),
  newConst("integer", "VEHICLE_TYPE_AIRPLANE",	4),
  newConst("integer", "VEHICLE_TYPE_BALLOON",	5),

  newConst("integer", "VEHICLE_LINEAR_FRICTION_TIMESCALE",	16),
  newConst("integer", "VEHICLE_ANGULAR_FRICTION_TIMESCALE",	17),
  newConst("integer", "VEHICLE_LINEAR_MOTOR_DIRECTION",		18),
  newConst("integer", "VEHICLE_ANGULAR_MOTOR_DIRECTION",	19),
  newConst("integer", "VEHICLE_LINEAR_MOTOR_OFFSET",		20),
  newConst("integer", "VEHICLE_HOVER_HEIGHT",			24),
  newConst("integer", "VEHICLE_HOVER_EFFICIENCY",		25),
  newConst("integer", "VEHICLE_HOVER_TIMESCALE",		26),
  newConst("integer", "VEHICLE_BUOYANCY",			27),
  newConst("integer", "VEHICLE_LINEAR_DEFLECTION_EFFICIENCY",	28),
  newConst("integer", "VEHICLE_LINEAR_DEFLECTION_TIMESCALE",	29),
  newConst("integer", "VEHICLE_LINEAR_MOTOR_TIMESCALE",		30),
  newConst("integer", "VEHICLE_LINEAR_MOTOR_DECAY_TIMESCALE",	31),
  newConst("integer", "VEHICLE_ANGULAR_DEFLECTION_EFFICIENCY",	32),
  newConst("integer", "VEHICLE_ANGULAR_DEFLECTION_TIMESCALE",	33),
  newConst("integer", "VEHICLE_ANGULAR_MOTOR_TIMESCALE",	34),
  newConst("integer", "VEHICLE_ANGULAR_MOTOR_DECAY_TIMESCALE",	35),
  newConst("integer", "VEHICLE_VERTICAL_ATTRACTION_EFFICIENCY",	36),
  newConst("integer", "VEHICLE_VERTICAL_ATTRACTION_TIMESCALE",	37),
  newConst("integer", "VEHICLE_BANKING_EFFICIENCY",		38),
  newConst("integer", "VEHICLE_BANKING_MIX",			39),
  newConst("integer", "VEHICLE_BANKING_TIMESCALE",		40),
  newConst("integer", "VEHICLE_REFERENCE_FRAME",		44),
  newConst("integer", "VEHICLE_RANGE_BLOCK",			45),
  newConst("integer", "VEHICLE_ROLL_FRAME",			46),

  newConst("integer", "WL_WATER_COLOR",			0),
  newConst("integer", "WL_WATER_FOG_DENSITY_EXPONENT",	1),
  newConst("integer", "WL_UNDERWATER_FOG_MODIFIER",	2),
  newConst("integer", "WL_REFLECTION_WAVELET_SCALE",	3),
  newConst("integer", "WL_FRESNEL_SCALE",		4),
  newConst("integer", "WL_FRESNEL_OFFSET",		5),
  newConst("integer", "WL_REFRACT_SCALE_ABOVE",		6),
  newConst("integer", "WL_REFRACT_SCALE_BELOW",		7),
  newConst("integer", "WL_BLUR_MULTIPLIER",		8),
  newConst("integer", "WL_BIG_WAVE_DIRECTION",		9),
  newConst("integer", "WL_LITTLE_WAVE_DIRECTION",	10),
  newConst("integer", "WL_NORMAL_MAP_TEXTURE",		11),
  newConst("integer", "WL_HORIZON",			12),
  newConst("integer", "WL_HAZE_HORIZON",		13),
  newConst("integer", "WL_BLUE_DENSITY",		14),
  newConst("integer", "WL_HAZE_DENSITY",		15),
  newConst("integer", "WL_DENSITY_MULTIPLIER",		16),
  newConst("integer", "WL_DISTANCE_MULTIPLIER",		17),
  newConst("integer", "WL_MAX_ALTITUDE",		18),
  newConst("integer", "WL_SUN_MOON_COLOR",		19),
  newConst("integer", "WL_AMBIENT",			20),
  newConst("integer", "WL_EAST_ANGLE",			21),
  newConst("integer", "WL_SUN_GLOW_FOCUS",		22),
  newConst("integer", "WL_SUN_GLOW_SIZE",		23),
  newConst("integer", "WL_SCENE_GAMMA",			24),
  newConst("integer", "WL_STAR_BRIGHTNESS",		25),
  newConst("integer", "WL_CLOUD_COLOR",			26),
  newConst("integer", "WL_CLOUD_XY_DENSITY",		27),
  newConst("integer", "WL_CLOUD_COVERAGE",		28),
  newConst("integer", "WL_CLOUD_SCALE",			29),
  newConst("integer", "WL_CLOUD_DETAIL_XY_DENSITY",	30),
  newConst("integer", "WL_CLOUD_SCROLL_X",		31),
  newConst("integer", "WL_CLOUD_SCROLL_Y",		32),
  newConst("integer", "WL_CLOUD_SCROLL_Y_LOCK",		33),
  newConst("integer", "WL_CLOUD_SCROLL_X_LOCK",		34),
  newConst("integer", "WL_DRAW_CLASSIC_CLOUDS",		35),
  newConst("integer", "WL_SUN_MOON_POSITION",		36),

  newConst("integer", "TRUE",			1),
  newConst("integer", "FALSE",			0),

  newConst("string", "NULL_KEY",		"00000000-0000-0000-0000-000000000000"),
--  newConst("string", "EOF",			"\\n\\n\\n"),	-- Corner case, dealt with later.
  newConst("string", "EOF",			"EndOfFuckingAround"),	-- Corner case, dealt with later.

  newConst("rotation", "ZERO_ROTATION",		{x=0.0, y=0.0, z=0.0, s=1.0}),
  newConst("vector", "ZERO_VECTOR",		{x=0.0, y=0.0, z=0.0}),

  newConst("string", "TEXTURE_BLANK",		"5748decc-f629-461c-9a36-a35a221fe21f"),
  newConst("string", "TEXTURE_DEFAULT",		"89556747-24cb-43ed-920b-47caed15465f"),
  newConst("string", "TEXTURE_PLYWOOD",		"89556747-24cb-43ed-920b-47caed15465f"),
  newConst("string", "TEXTURE_TRANSPARENT",	"8dcd4a48-2d37-4909-9f78-f7a9eb4ef903"),
  newConst("string", "TEXTURE_MEDIA",		"8b5fec65-8d8d-9dc5-cda8-8fdf2716e361"),

  newConst("string", "URL_REQUEST_GRANTED",	"URL_REQUEST_GRANTED"),
  newConst("string", "URL_REQUEST_DENIED",	"URL_REQUEST_DENIED"),

-- TODO - Temporary dummy variables to get vector and rotation thingies to work for now.

  newConst("float", "s",			1.0),
  newConst("float", "x",			0.0),
  newConst("float", "y",			0.0),
  newConst("float", "z",			0.0),
}


-- LSL animation overrider functions
newFunc("string",	"llGetAnimationOverride", "string anim_state")
newFunc("",		"llResetAnimationOverride", "string anim_state")
newFunc("",		"llSetAnimationOverride", "string anim_state", "string anim")

-- LSL avatar functions
newFunc("",		"llAttachToAvatar", "integer attachment")
newFunc("",		"llAttachToAvatarTemp", "integer attachment")
newFunc("key",		"llAvatarOnSitTarget")
newFunc("key",		"llAvatarOnLinkSitTarget", "integer linknum")
newFunc("",		"llClearCameraParams")
newFunc("",		"llDetachFromAvatar")
newFunc("",		"llForceMouselook", "integer mouselook")
newFunc("integer",	"llGetAgentInfo", "string id")
newFunc("string",	"llGetAgentLanguage", "string id")
newFunc("list",		"llGetAgentList", "integer scope", "list options")
newFunc("vector",	"llGetAgentSize", "string id")
newFunc("string",	"llGetAnimation", "string id")
newFunc("list",		"llGetAnimationList", "key id")
newFunc("vector",	"llGetCameraPos")
newFunc("rotation",	"llGetCameraRot")
newFunc("string",	"llGetDisplayName", "string id")
newFunc("integer",	"llGetPermissions")
newFunc("key",		"llGetPermissionsKey")
newFunc("string",	"llGetUsername", "string id")
newFunc("integer",	"llGiveMoney", "string destination", "integer amount")
newFunc("string",	"llKey2Name", "key avatar")
newFunc("",		"llPointAt", "vector pos")
newFunc("",		"llReleaseCamera", "string avatar")
newFunc("",		"llReleaseControls")
newFunc("key",		"llRequestAgentData", "string id", "integer data")
newFunc("string",	"llRequestDisplayName", "string id")
newFunc("",		"llRequestPermissions", "key avatar", "integer perms")
newFunc("string",	"llRequestUsername", "string id")
newFunc("integer",	"llSameGroup", "key avatar")
newFunc("",		"llSetCameraAtOffset", "vector offset")
newFunc("",		"llSetCameraEyeOffset", "vector offset")
newFunc("",		"llSetCameraParams", "list rules")
newFunc("",		"llSetLinkCamera", "integer link", "vector eye", "vector at")
newFunc("",		"llStartAnimation", "string anim")
newFunc("",		"llStopAnimation", "string anim")
newFunc("",		"llStopPointAt")
newFunc("",		"llTakeCamera", "string avatar")
newFunc("",		"llTakeControls", "integer controls", "integer accept", "integer pass_on")
newFunc("",		"llTeleportAgentHome", "string agent")
newFunc("",		"llTeleportAgent", "string agent", "string simname", "vector pos", "vector lookAt")
newFunc("",		"llTeleportAgentGlobalCoords", "string agent", "vector global", "vector pos", "vector lookAt")
newFunc("key",		"llTransferLindenDollars", "key destination", "integer amount")
newFunc("",		"llUnSit", "key avatar")

-- LSL collision / detect / sensor functions
newFunc("list",		"llCastRay", "vector start", "vector End", "list options")
newFunc("",		"llCollisionFilter", "string name", "string id", "integer accept")
newFunc("",		"llCollisionSound", "string impact_sound", "float impact_volume")
newFunc("",		"llCollisionSprite", "string impact_sprite")
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
newFunc("",		"llPassCollisions", "integer pass")
newFunc("integer",	"llRotTarget", "rotation rot", "float error")
newFunc("",		"llRotTargetRemove", "integer number")
newFunc("",		"llSensor", "string name", "string id", "integer Type", "float range", "float arc")
newFunc("",		"llSensorRemove")
newFunc("",		"llSensorRepeat", "string name", "string id", "integer Type", "float range", "float arc", "float rate")
newFunc("integer",	"llTarget", "vector position", "float range")
newFunc("",		"llTargetRemove", "integer number")
newFunc("",		"llVolumeDetect", "integer detect")

-- LSL communications functions
newFunc("",		"llCloseRemoteDataChannel", "string channel")
newFunc("",		"llDialog", "key avatar", "string caption", "list arseBackwardsMenu", "integer channel")
newFunc("",		"llEmail", "string address", "string subject", "string message")
newFunc("integer",	"llGetFreeURLs")
newFunc("string",	"llGetHTTPHeader", "key request_id", "string header")
newFunc("",		"llGetNextEmail", "string address", "string subject")
newFunc("string",	"llHTTPRequest", "string url", "list parameters", "string body")
newFunc("",		"llHTTPResponse", "key id", "integer status", "string body")
newFunc("",		"llInstantMessage", "string user", "string message")
newFunc("integer",	"llListen", "integer channel", "string name", "key id", "string msg")
newFunc("",		"llListenControl", "integer number", "integer active")
newFunc("",		"llListenRemove", "integer handle")
newFunc("",		"llLoadURL", "string avatar_id", "string message", "string url")
newFunc("",		"llMessageLinked", "integer link", "integer num", "string text", "key aKey")
newFunc("",		"llOpenRemoteDataChannel")
newFunc("",		"llOwnerSay", "string text")
newFunc("",		"llRefreshPrimURL")
newFunc("",		"llRegionSay", "integer channelID", "string text")
newFunc("",		"llRegionSayTo", "string target", "integer channelID", "string text")
newFunc("",		"llReleaseURL", "string url")
newFunc("",		"llRemoteDataReply", "string channel", "string message_id", "string sdata", "integer idata")
newFunc("",		"llRemoteDataSetRegion")
newFunc("string",	"llRequestSecureURL")
newFunc("key",		"llRequestURL")
newFunc("",		"llSay", "integer channel", "string text")
newFunc("key",		"llSendRemoteData", "string channel", "string dest", "integer idata", "string sdata")
newFunc("",		"llSetContentType", "key request_id", "integer content_type")
newFunc("",		"llSetPrimURL", "string url")
newFunc("",		"llShout", "integer channel", "string text")
newFunc("",		"llTextBox", "string avatar", "string message", "integer chat_channel")
newFunc("",		"llWhisper", "integer channel", "string text")

-- LSL inventory functions.
newFunc("key",		"llGetInventoryCreator", "string item")
newFunc("key",		"llGetInventoryKey", "string name")
newFunc("string",	"llGetInventoryName", "integer Type", "integer index")
newFunc("integer",	"llGetInventoryNumber", "integer Type")
newFunc("integer",	"llGetInventoryPermMask", "string item", "integer mask")
newFunc("integer",	"llGetInventoryType", "string name")
newFunc("key",		"llGetNotecardLine", "string name", "integer index")
newFunc("key",		"llGetNumberOfNotecardLines", "string name")
newFunc("",		"llGiveInventory", "string destination", "string inventory")
newFunc("",		"llGiveInventoryList", "string destination", "string category", "list inventory")
newFunc("",		"llGodLikeRezObject", "string inventory", "vector pos")
newFunc("",		"llRemoveInventory", "string item")
newFunc("key",		"llRequestInventoryData", "string name")
newFunc("",		"llRezAtRoot", "string name", "vector position", "vector velocity", "rotation rot", "integer channel")
newFunc("",		"llRezObject", "string name", "vector position", "vector velocity", "rotation rot", "integer channel")
newFunc("",		"llSetInventoryPermMask", "string item", "integer mask", "integer value")

-- LSL JSON functions
newFunc("list",		"llJson2List", "string src")
newFunc("string",	"llJsonGetValue", "string json", "list specifiers")
newFunc("string",	"llJsonSetValue", "string json", "list specifiers", "string value")
newFunc("string",	"llJsonValueType", "string json", "list specifiers")
newFunc("string",	"llList2Json", "string Type", "list values")

-- LSL land / parcel / plot / region / sim / weather functions
newFunc("",		"llAddToLandBanList", "string avatar", "float hours")
newFunc("",		"llAddToLandPassList", "string avatar", "float hours")
newFunc("float",	"llCloud", "vector offset")
newFunc("integer",	"llEdgeOfWorld", "vector pos", "vector dir")
newFunc("",		"llEjectFromLand", "string pest")
newFunc("string",	"llGetEnv", "string name")
newFunc("key",		"llGetLandOwnerAt", "vector pos")
newFunc("list",		"llGetParcelDetails", "vector pos", "list param")
newFunc("integer",	"llGetParcelFlags", "vector pos")
newFunc("integer",	"llGetParcelMaxPrims", "vector pos", "integer sim_wide")
newFunc("integer",	"llGetParcelPrimCount", "vector pos", "integer category", "integer sim_wide")
newFunc("list",		"llGetParcelPrimOwners", "vector pos")
newFunc("integer",	"llGetRegionAgentCount")
newFunc("vector",	"llGetRegionCorner")
newFunc("integer",	"llGetRegionFlags")
newFunc("float",	"llGetRegionFPS")
newFunc("string",	"llGetRegionName")
newFunc("float",	"llGetRegionTimeDilation")
newFunc("float",	"llGetSimStats", "integer stat_type")
newFunc("string",	"llGetSimulatorHostname")
newFunc("vector",	"llGetSunDirection")
newFunc("float",	"llGround", "vector offset")
newFunc("vector",	"llGroundContour", "vector offset")
newFunc("vector",	"llGroundNormal", "vector offset")
newFunc("",		"llGroundRepel", "float height", "integer water", "float tau")
newFunc("vector",	"llGroundSlope", "vector offset")
newFunc("integer",	"llManageEstateAccess", "integer action", "key avatar")
newFunc("",		"llMapDestination", "string simname", "vector pos", "vector look_at")
newFunc("",		"llModifyLand", "integer action", "integer brush")
newFunc("integer",	"llOverMyLand", "string id")
newFunc("",		"llRemoveFromLandBanList", "string avatar")
newFunc("",		"llRemoveFromLandPassList", "string avatar")
newFunc("key",		"llRequestSimulatorData", "string simulator", "integer data")
newFunc("",		"llResetLandBanList")
newFunc("",		"llResetLandPassList")
newFunc("integer",	"llReturnObjectsByID", "list objects")
newFunc("integer",	"llReturnObjectsByOwner", "key owner", "integer scope")
newFunc("float",	"llWater", "vector offset")
newFunc("vector",	"llWind", "vector offset")

-- LSL link / object / prim functions
newFunc("",		"llAllowInventoryDrop", "integer add")
newFunc("",		"llBreakAllLinks")
newFunc("",		"llBreakLink", "integer linknum")
newFunc("",		"llCreateLink", "string target", "integer parent")
newFunc("",		"llDie")
newFunc("integer",	"llGetAttached")
newFunc("list",		"llGetBoundingBox", "string obj")
newFunc("string",	"llGetCreator")
newFunc("float",	"llGetEnergy")
newFunc("vector",	"llGetGeometricCenter")
newFunc("key",		"llGetKey")
newFunc("key",		"llGetLinkKey", "integer linknum")
newFunc("string",	"llGetLinkName", "integer linknum")
newFunc("integer",	"llGetLinkNumber")
newFunc("integer",	"llGetLinkNumberOfSides", "integer link")
newFunc("list",		"llGetLinkPrimitiveParams", "integer linknum", "list rules")
newFunc("float",	"llGetMaxScaleFactor")
newFunc("float",	"llGetMinScaleFactor")
newFunc("integer",	"llGetNumberOfPrims")
newFunc("integer",	"llGetNumberOfSides")
newFunc("string",	"llGetObjectDesc")
newFunc("list",		"llGetObjectDetails", "string id", "list args")
newFunc("string",	"llGetObjectName")
newFunc("integer",	"llGetObjectPermMask", "integer mask")
newFunc("integer",	"llGetObjectPrimCount", "string object_id")
newFunc("vector",	"llGetOmega")
newFunc("key",		"llGetOwner")
newFunc("key",		"llGetOwnerKey", "string id")
newFunc("list",		"llGetPhysicsMaterial")
newFunc("list",		"llGetPrimitiveParams", "list rules")
newFunc("integer",	"llGetStatus", "integer status")
newFunc("",		"llLinkParticleSystem", "integer linknum", "list rules")
newFunc("",		"llLinkSitTarget", "integer link", "vector offset", "rotation rot")
newFunc("",		"llLookAt", "vector target", "float strength", "float damping")
newFunc("",		"llMakeExplosion", "integer particles", "float scale", "float vel", "float lifetime", "float arc", "string texture", "vector offset")
newFunc("",		"llMakeFire", "integer particles", "float scale", "float vel", "float lifetime", "float arc", "string texture", "vector offset")
newFunc("",		"llMakeFountain", "integer particles", "float scale", "float vel", "float lifetime", "float arc", "integer bounce", "string texture", "vector offset", "float bounce_offset")
newFunc("",		"llMakeSmoke", "integer particles", "float scale", "float vel", "float lifetime", "float arc", "string texture", "vector offset")
newFunc("",		"llMoveToTarget", "vector target", "float tau")
newFunc("",		"llParticleSystem", "list rules")
newFunc("",		"llPassTouches", "integer pass")
newFunc("",		"llRotLookAt", "rotation target", "float strength", "float damping")
newFunc("integer",	"llScaleByFactor", "float scaling_factor")
newFunc("",		"llSetClickAction", "integer action")
newFunc("",		"llSetDamage", "float damage")
newFunc("",		"llSetHoverHeight", "float height", "integer water", "float tau")
newFunc("",		"llSetLinkPrimitiveParams", "integer linknumber", "list rules")
newFunc("",		"llSetLinkPrimitiveParamsFast", "integer linknum", "list rules")
newFunc("",		"llSetObjectDesc", "string text")
newFunc("",		"llSetObjectName", "string text")
newFunc("",		"llSetObjectPermMask", "integer mask", "integer value")
newFunc("",		"llSetPayPrice", "integer price", "list quick_pay_buttons")
newFunc("",		"llSetPhysicsMaterial", "integer mask", "float gravity_multiplier", "float restitution", "float friction", "float density")
newFunc("",		"llSetPrimitiveParams", "list params")
newFunc("integer",	"llSetRegionPos", "vector pos")
newFunc("",		"llSetStatus", "integer status", "integer value")
newFunc("",		"llSetSitText", "string text")
newFunc("",		"llSetText", "string text", "vector colour", "float alpha")
newFunc("",		"llSetTouchText", "string text")
newFunc("",		"llSitTarget", "vector pos", "rotation rot")
newFunc("",		"llStopHover")
newFunc("",		"llStopLookAt")
newFunc("",		"llStopMoveToTarget")
newFunc("",		"llTargetOmega", "vector axis", "float spinrate", "float gain")

-- LSL list functions.
newFunc("list",		"llCSV2List", "string text")
newFunc("list",		"llDeleteSubList", "list l", "integer start", "integer End")
newFunc("string",	"llDumpList2String", "list l", "string separator")
newFunc("integer",	"llGetListEntryType", "list src", "integer index")
newFunc("integer",	"llGetListLength", "list l")
newFunc("string",	"llList2CSV", "list l")
newFunc("float",	"llList2Float", "list l", "integer index")
newFunc("integer",	"llList2Integer", "list l", "integer index")
newFunc("key",		"llList2Key", "list l", "integer index")
newFunc("list",		"llList2List", "list l", "integer start", "integer End")
newFunc("list",		"llList2ListStrided", "list src", "integer start", "integer eNd", "integer stride")
newFunc("string",	"llList2String", "list l", "integer index")
newFunc("rotation",	"llList2Rot", "list l", "integer index")
newFunc("vector",	"llList2Vector", "list l", "integer index")
newFunc("integer",	"llListFindList", "list l", "list l1")
newFunc("list",		"llListInsertList", "list l", "list l1", "integer index")
newFunc("list",		"llListRandomize", "list src", "integer stride")
newFunc("list",		"llListReplaceList", "list l", "list part", "integer start", "integer End")
newFunc("list",		"llListSort", "list l", "integer stride", "integer ascending")
newFunc("float",	"llListStatistics", "integer operation", "list src")
newFunc("list",		"llParseString2List", "string In", "list l", "list l1")
newFunc("list",		"llParseStringKeepNulls", "string In", "list l", "list l1")

-- LSL math functions
newFunc("integer",	"llAbs", "integer i")
newFunc("float",	"llAcos", "float val")
newFunc("float",	"llAngleBetween", "rotation a", "rotation b")
newFunc("float",	"llAsin", "float val")
newFunc("float",	"llAtan2", "float x", "float y")
newFunc("rotation",	"llAxes2Rot", "vector fwd", "vector left", "vector up")
newFunc("rotation",	"llAxisAngle2Rot", "vector axis", "float angle")
newFunc("integer",	"llCeil", "float f")
newFunc("float",	"llCos", "float f")
newFunc("float",	"llFabs", "float f")
newFunc("integer",	"llFloor", "float f")
newFunc("float",	"llFrand", "float mag")
newFunc("rotation",	"llEuler2Rot", "vector vec")
newFunc("float",	"llFrand", "float max")
newFunc("float",	"llLog", "float val")
newFunc("float",	"llLog10", "float val")
newFunc("integer",	"llModPow", "integer a", "integer b", "integer c")
newFunc("float",	"llPow", "float number", "float places")
newFunc("float",	"llRot2Angle", "rotation rot")
newFunc("vector",	"llRot2Axis", "rotation rot")
newFunc("vector",	"llRot2Euler", "rotation rot")
newFunc("vector",	"llRot2Fwd", "rotation r")
newFunc("vector",	"llRot2Left", "rotation r")
newFunc("vector",	"llRot2Up", "rotation r")
newFunc("rotation",	"llRotBetween", "vector start", "vector eNd")
newFunc("integer",	"llRound", "float number")
newFunc("float",	"llSin", "float f")
newFunc("float",	"llSqrt", "float f")
newFunc("float",	"llTan", "float f")
newFunc("float",	"llVecDist", "vector a", "vector b")
newFunc("float",	"llVecMag", "vector v")
newFunc("vector",	"llVecNorm", "vector v")

-- LSL media functions
newFunc("",		"llAdjustSoundVolume", "float volume")
newFunc("integer",	"llClearLinkMedia", "integer link", "integer face")
newFunc("integer",	"llClearPrimMedia", "integer face")
newFunc("list",		"llGetLinkMedia", "integer link", "integer face", "list rules")
newFunc("string",	"llGetParcelMusicURL")
newFunc("list",		"llGetPrimMediaParams", "integer face", "list rules")
newFunc("",		"llLoopSound", "string sound", "float volume")
newFunc("",		"llLoopSoundMaster", "string sound", "float volume")
newFunc("",		"llLoopSoundSlave", "string sound", "float volume")
newFunc("",		"llParcelMediaCommandList", "list commandList")
newFunc("list",		"llParcelMediaQuery", "list aList")
newFunc("",		"llPlaySound", "string name", "float volume")
newFunc("",		"llPlaySoundSlave", "string sound", "float volume")
newFunc("",		"llPreloadSound", "string sound")
newFunc("integer",	"llSetLinkMedia", "integer link", "integer face", "list rules")
newFunc("",		"llSetParcelMusicURL", "string url")
newFunc("integer",	"llSetPrimMediaParams", "integer face", "list rules")
newFunc("",		"llSetSoundQueueing", "integer queue")
newFunc("",		"llSetSoundRadius", "float radius")
newFunc("",		"llSound", "string sound", "float volume", "integer queue", "integer loop")
newFunc("",		"llSoundPreload", "string sound")
newFunc("",		"llStopSound")
newFunc("",		"llTriggerSound", "string sound", "float volume")
newFunc("",		"llTriggerSoundLimited", "string sound", "float volume", "vector top_north_east", "vector bottom_south_west")

-- LSL path finding functions
newFunc("",		"llCreateCharacter", "list options")
newFunc("",		"llDeleteCharacter")
newFunc("",		"llEvade", "key target", "list options")
newFunc("",		"llExecCharacterCmd", "integer command", "list options")
newFunc("",		"llFleeFrom", "vector position", "float distance", "list options")
newFunc("list",		"llGetClosestNavPoint", "vector point", "list options")
newFunc("list",		"llGetStaticPath", "vector start", "vector End", "float radius", "list params")
newFunc("",		"llNavigateTo", "vector pos", "list options")
newFunc("",		"llPatrolPoints", "list patrolPoints", "list options")
newFunc("",		"llPursue", "key target", "list options")
newFunc("",		"llUpdateCharacter", "list options")
newFunc("",		"llWanderWithin", "vector origin", "vector dist", "list options")

-- LSL physics functions
newFunc("",		"llApplyImpulse", "vector force", "integer local")
newFunc("",		"llApplyRotationalImpulse", "vector force", "integer local")
newFunc("vector",	"llGetAccel")
newFunc("vector",	"llGetCenterOfMass")
newFunc("vector",	"llGetForce")
newFunc("float",	"llGetMass")
newFunc("float",	"llGetMassMKS")
newFunc("float",	"llGetObjectMass", "string id")
newFunc("vector",	"llGetTorque")
newFunc("vector",	"llGetVel")
newFunc("",		"llPushObject", "string target", "vector impulse", "vector ang_impulse", "integer local")
newFunc("",		"llSetAngularVelocity", "vector initial_omega", "integer local")
newFunc("",		"llSetBuoyancy", "float buoyancy")
newFunc("",		"llSetForce", "vector force", "integer local")
newFunc("",		"llSetForceAndTorque", "vector force", "vector torque", "integer local")
newFunc("",		"llSetKeyframedMotion", "list keyframes", "list options")
newFunc("",		"llSetTorque", "vector torque", "integer local")
newFunc("",		"llSetVelocity", "vector force", "integer local")

-- LSL rotation / scaling / translation functions
newFunc("vector",	"llGetLocalPos")
newFunc("rotation",	"llGetLocalRot")
newFunc("vector",	"llGetPos")
newFunc("vector",	"llGetRootPosition")
newFunc("rotation",	"llGetRootRotation")
newFunc("rotation",	"llGetRot")
newFunc("vector",	"llGetScale")
newFunc("",		"llSetLocalRot", "rotation rot")
newFunc("",		"llSetPos", "vector pos")
newFunc("",		"llSetRot", "rotation rot")
newFunc("",		"llSetScale", "vector scale")

-- LSL script functions
newFunc("integer",	"llGetFreeMemory")
newFunc("integer",	"llGetMemoryLimit")
newFunc("string",	"llGetScriptName")
newFunc("integer",	"llGetScriptState", "string name")
newFunc("integer",	"llGetSPMaxMemory")
newFunc("integer",	"llGetStartParameter")
newFunc("integer",	"llGetUsedMemory")
newFunc("",		"llRemoteLoadScript", "string target", "string name", "integer running", "integer start_param")
newFunc("",		"llRemoteLoadScriptPin", "string target", "string name", "integer pin", "integer running", "integer start_param")
newFunc("",		"llResetOtherScript", "string name")
newFunc("",		"llResetScript")
newFunc("integer",	"llScriptDanger", "vector pos")
newFunc("",		"llScriptProfiler", "integer flag")
newFunc("integer",	"llSetMemoryLimit", "integer limit")
newFunc("",		"llSetRemoteScriptAccessPin", "integer pin")
newFunc("",		"llSetScriptState", "string name", "integer running")

-- LSL string functions
newFunc("integer",	"llBase64ToInteger", "string str")
newFunc("string",	"llBase64ToString", "string str")
newFunc("string",	"llDeleteSubString", "string src", "integer start", "integer eNd")
newFunc("string",	"llEscapeURL", "string url")
newFunc("key",		"llGenerateKey")
newFunc("string",	"llGetSubString", "string text", "integer start", "integer End")
newFunc("string",	"llInsertString", "string dst", "integer position", "string src")
newFunc("string",	"llIntegerToBase64", "integer number")
newFunc("string",	"llMD5String", "string src", "integer nonce")
newFunc("string",	"llSHA1String", "string src")
newFunc("integer",	"llStringLength", "string text")
newFunc("string",	"llStringToBase64", "string str")
newFunc("string",	"llStringTrim", "string text", "integer Type")
newFunc("integer",	"llSubStringIndex", "string text", "string sub")
newFunc("string",	"llToLower", "string source")
newFunc("string",	"llToUpper", "string source")
newFunc("string",	"llUnescapeURL", "string url")
newFunc("string",	"llXorBase64", "string str1", "string str2")
newFunc("string",	"llXorBase64Strings", "string str1", "string str2")
newFunc("string",	"llXorBase64StringsCorrect", "string str1", "string str2")

-- LSL texture functions
newFunc("float",	"llGetAlpha", "integer side")
newFunc("vector",	"llGetColor", "integer face")
newFunc("string",	"llGetTexture", "integer face")
newFunc("vector",	"llGetTextureOffset", "integer face")
newFunc("float",	"llGetTextureRot", "integer side")
newFunc("vector",	"llGetTextureScale", "integer side")
newFunc("",		"llOffsetTexture", "float u", "float v", "integer face")
newFunc("",		"llRotateTexture", "float rot", "integer face")
newFunc("",		"llScaleTexture", "float u", "float v", "integer face")
newFunc("",		"llSetAlpha", "float alpha", "integer side")
newFunc("",		"llSetColor", "vector colour", "integer side")
newFunc("",		"llSetLinkAlpha", "integer linknumber", "float alpha", "integer face")
newFunc("",		"llSetLinkColor", "integer linknumber", "vector color", "integer face")
newFunc("",		"llSetLinkTexture", "integer linknumber", "string texture", "integer face")
newFunc("",		"llSetLinkTextureAnim", "integer linknum", "integer mode", "integer face", "integer sizex", "integer sizey", "float start", "float length", "float rate")
newFunc("",		"llSetTexture", "string texture", "integer face")
newFunc("",		"llSetTextureAnim", "integer mode", "integer face", "integer sizex", "integer sizey", "float start", "float length", "float rate")

-- LSL time functions
newFunc("float",	"llGetAndResetTime")
newFunc("string",	"llGetDate")
newFunc("float",	"llGetGMTclock")
newFunc("float",	"llGetTime")
newFunc("float",	"llGetTimeOfDay")
newFunc("string",	"llGetTimestamp")
newFunc("integer",	"llGetUnixTime")
newFunc("float",	"llGetWallclock")
newFunc("",		"llMinEventDelay", "float delay")
newFunc("",		"llResetTime")
newFunc("",		"llSetTimerEvent", "float seconds")
newFunc("float",	"llSleep", "float seconds")  -- Faked return type, it actually does not return anything.  This forces it to wait.  Actually fully implements llSleep().  B-)

-- LSL vehicle functions
newFunc("",		"llRemoveVehicleFlags", "integer flags")
newFunc("",		"llSetVehicleFlags", "integer flags")
newFunc("",		"llSetVehicleFloatParam", "integer param", "float value")
newFunc("",		"llSetVehicleRotationParam", "integer param", "rotation rot")
newFunc("",		"llSetVehicleType", "integer Type")
newFunc("",		"llSetVehicleVectorParam", "integer param", "vector vec")


--- OS functions

-- OS animation functions
newFunc("",		"osAvatarPlayAnimation", "string avatar", "string animation")
newFunc("",		"osAvatarStopAnimation", "string avatar", "string animation")

-- OS attachment functions
newFunc("",		"osDropAttachment")
newFunc("",		"osDropAttachmentAt", "vector pos", "rotation rot")
newFunc("",		"osForceAttachToAvatar", "integer attachment")
newFunc("",		"osForceAttachToAvatarFromInventory", "string itemName", "integer attachment")
newFunc("",		"osForceAttachToOtherAvatarFromInventory", "string rawAvatarId", "string itemName", "integer attachmentPoint")
newFunc("",		"osForceDetachFromAvatar")
newFunc("",		"osForceDropAttachment")
newFunc("",		"osForceDropAttachmentAt", "vector pos", "rotation rot")
newFunc("list",		"osGetNumberOfAttachments", "key avatar", "list attachmentPoints")
newFunc("",		"osMessageAttachments", "key avatar", "string message", "list attachmentPoints", "integer flags")

-- OS avatar functions
newFunc("key",		"osAgentSaveAppearance", "key agentId", "string notecard")
newFunc("string",	"osAvatarName2Key", "string firstname", "string lastname")
newFunc("",		"osCauseDamage", "string avatar", "float damage")
newFunc("",		"osCauseHealing", "string avatar", "float healing")
newFunc("integer",	"osEjectFromGroup", "key agentId")
newFunc("string",	"osGetAgentIP", "string agent")
newFunc("list",		"osGetAgents")
newFunc("list",		"osGetAvatarList")
newFunc("float",	"osGetHealth", "string avatar")
newFunc("integer",	"osInviteToGroup", "key agentId")
newFunc("string",	"osKey2Name", "string id")
newFunc("",		"osKickAvatar", "string FirstName", "string SurName", "string alert")
newFunc("key",		"osOwnerSaveAppearance", "string notecard")
newFunc("",		"osSetSpeed", "string UUID", "float SpeedModifier")
-- TODO - Hmmm, function overloading, Lua doesn't support that directly.  Though it can be faked, as with everything in Lua.  B-)
newFunc("",		"osTeleportAgent", "string agent", "string regionName", "vector position", "vector lookat")
newFunc("",		"osTeleportAgent", "string agent", "integer regionX", "integer regionY", "vector position", "vector lookat")
newFunc("",		"osTeleportAgent", "string agent", "vector position", "vector lookat")
newFunc("",		"osTeleportOwner", "string regionName", "vector position", "vector lookat")
newFunc("",		"osTeleportOwner", "integer regionX", "integer regionY", "vector position", "vector lookat")
newFunc("",		"osTeleportOwner", "vector position", "vector lookat")

-- OS communication functions
newFunc("integer",	"osListenRegex", "integer channelID", "string name", "string ID", "string msg", "integer regexBitfield")
newFunc("",		"osMessageObject", "key objectUUID", "string message")

-- OS grid functions
newFunc("string",	"osGetGridCustom", "string k")
newFunc("string",	"osGetGridGatekeeperURI")
newFunc("string",	"osGetGridHomeURI")
newFunc("string",	"osGetGridLoginURI")
newFunc("string",	"osGetGridName")
newFunc("string",	"osGetGridNick")

-- OS inventory functions
newFunc("string",	"osGetNotecard", "string name")
newFunc("string",	"osGetNotecardLine", "string name", "integer line")
newFunc("integer",	"osGetNumberOfNotecardLines", "string name")
--newFunc("",		"osMakeNotecard","string notecardName", "LSL_Types.list contents")

-- OS land / parcel / plot / region / sim / weather functions
--newFunc("bool",		"osConsoleCommand", "string Command")
newFunc("float",	"osGetCurrentSunHour")
newFunc("key",		"osGetMapTexture")
newFunc("key",		"osGetRegionMapTexture", "string regionName")
newFunc("list",		"osGetRegionStats")
newFunc("integer",	"osGetSimulatorMemory")
newFunc("string",	"osGetSimulatorVersion")
newFunc("float",	"osGetSunParam", "string param")
newFunc("float",	"osGetTerrainHeight", "integer x", "integer y")
newFunc("float",	"osGetWindParam", "string plugin", "string param")
newFunc("string",	"osLoadedCreationDate")
newFunc("string",	"osLoadedCreationID")
newFunc("string",	"osLoadedCreationTime")
newFunc("",		"osParcelJoin", "vector pos1", "vector pos2")
newFunc("",		"osParcelSetDetails", "vector pos", "list rules")
newFunc("",		"osParcelSubdivide", "vector pos1", "vector pos2")
newFunc("",		"osRegionNotice", "string msg")
newFunc("integer",	"osRegionRestart", "float seconds")
--newFunc("",		"osSetEstateSunSettings", "bool sunFixed", "float sunHour")
newFunc("",		"osSetParcelDetails", "vector pos", "list rules")
newFunc("",		"osSetParcelMediaURL", "string url")
newFunc("",		"osSetParcelSIPAddress", "string SIPAddress")
--newFunc("",		"osSetRegionSunSettings", "bool useEstateSun", "bool sunFixed", "float sunHour")
newFunc("",		"osSetRegionWaterHeight", "float height")
newFunc("",		"osSetSunParam", "string param", "float value")
newFunc("integer",	"osSetTerrainHeight", "integer x", "integer y", "float val")
newFunc("",		"osSetTerrainTexture", "integer level", "key texture")
newFunc("",		"osSetTerrainTextureHeight", "integer corner", "float low", "float high")
newFunc("",		"osSetWindParam", "string plugin", "string param", "float value")
newFunc("float",	"osSunGetParam", "string param")
newFunc("",		"osSunSetParam", "string param", "float value")
newFunc("",		"osTerrainFlush")
newFunc("float",	"osTerrainGetHeight", "integer x", "integer y")
newFunc("integer",	"osTerrainSetHeight", "integer x", "integer y", "float val")
newFunc("string",	"osWindActiveModelPluginName")

-- OS link / object / prim functions
newFunc("list",		"osGetLinkPrimitiveParams", "integer linknumber", "list rules")
newFunc("list",		"osGetPrimitiveParams", "key prim", "list rules")
newFunc("key",		"osGetRezzingObject")
newFunc("",		"osSetContentType", "key id", "string type")
newFunc("",		"osSetPrimFloatOnWater", "integer floatYN")
--newFunc("",		"osSetProjectionParams", "bool projection", "key texture", "float fov", "float focus", "float amb")
--newFunc("",		"osSetProjectionParams", "key prim", "bool projection", "key texture", "float fov", "float focus", "float amb")

-- OS list functions
--newFunc("float",	"osList2Double", "LSL_Types.list src", "integer index")

-- OS math functions
newFunc("float",	"osMax", "float a", "float b")
newFunc("float",	"osMin", "float a", "float b")

-- OS NPC functions
newFunc("key",		"osNpcCreate", "string user", "string name", "vector position", "key cloneFrom")
newFunc("key",		"osNpcCreate", "string user", "string name", "vector position", "string notecard")
newFunc("key",		"osNpcCreate", "string user", "string name", "vector position", "string notecard", "integer options")
newFunc("key",		"osNpcGetOwner", "key npc")
newFunc("vector",	"osNpcGetPos", "key npc")
newFunc("rotation",	"osNpcGetRot", "key npc")
newFunc("integer",	"osIsNpc", "key npc")
newFunc("",		"osNpcLoadAppearance", "key npc", "string notecard")
newFunc("",		"osNpcMoveTo", "key npc", "vector position")
newFunc("",		"osNpcMoveToTarget", "key npc", "vector target", "integer options")
newFunc("",		"osNpcPlayAnimation", "key npc", "string animation")
newFunc("",		"osNpcRemove", "key npc")
newFunc("key",		"osNpcSaveAppearance", "key npc", "string notecard")
newFunc("",		"osNpcSay", "key npc", "string message")
newFunc("",		"osNpcSay", "key npc", "integer channel", "string message")
newFunc("",		"osNpcSetRot", "key npc", "rotation rot")
newFunc("",		"osNpcShout", "key npc", "integer channel", "string message")
newFunc("",		"osNpcSit", "key npc", "key target", "integer options")
newFunc("",		"osNpcStand", "key npc")
newFunc("",		"osNpcStopAnimation", "key npc", "string animation")
newFunc("",		"osNpcStopMoveToTarget", "key npc")
newFunc("",		"osNpcTouch", "key npcLSL_Key", "key object_key", "integer link_num")
newFunc("",		"osNpcWhisper", "key npc", "integer channel", "string message")

-- OS script functions
newFunc("string",	"osGetScriptEngineName")

-- OS string functions
newFunc("integer",	"osIsUUID", "string thing")
newFunc("string",	"osFormatString", "string str", "list strings")
newFunc("list",		"osMatchString", "string src", "string pattern", "integer start")
--newFunc("Hashtable",	"osParseJSON","string JSON")
newFunc("integer",	"osRegexIsMatch", "string input", "string pattern")
newFunc("string",	"osReplaceString", "string src", "string pattern", "string replace", "integer count", "integer start")

-- OS texture functions
newFunc("string",	"osDrawEllipse", "string drawList", "integer width", "integer height")
newFunc("string",	"osDrawFilledPolygon", "string drawList", "list x", "list y")
newFunc("string",	"osDrawFilledRectangle", "string drawList", "integer width", "integer height")
newFunc("string",	"osDrawImage", "string drawList", "integer width", "integer height", "string imageUrl")
newFunc("string",	"osDrawLine", "string drawList", "integer startX", "integer startY", "integer endX", "integer endY")
newFunc("string",	"osDrawLine", "string drawList", "integer endX", "integer endY")
newFunc("string",	"osDrawPolygon", "string drawList", "list x", "list y")
newFunc("string",	"osDrawRectangle", "string drawList", "integer width", "integer height")
newFunc("string",	"osDrawText", "string drawList", "string text")
newFunc("vector",	"osGetDrawStringSize", "string contentType", "string text", "string fontName", "integer fontSize")
newFunc("string",	"osMovePen", "string drawList", "integer x", "integer y")
newFunc("string",	"osSetDynamicTextureData", "string dynamicID", "string contentType", "string data", "string extraParams", "integer timer")
newFunc("string",	"osSetDynamicTextureDataBlend", "string dynamicID", "string contentType", "string data", "string extraParams", "integer timer", "integer alpha")
--newFunc("string",	"osSetDynamicTextureDataBlendFace", "string dynamicID", "string contentType", "string data", "string extraParams", "bool blend", "integer disp", "integer timer", "integer alpha", "integer face")
newFunc("string",	"osSetDynamicTextureURL", "string dynamicID", "string contentType", "string url", "string extraParams", "integer timer")
newFunc("string",	"osSetDynamicTextureURLBlend", "string dynamicID", "string contentType", "string url", "string extraParams", "integer timer", "integer alpha")
--newFunc("string",	"osSetDynamicTextureURLBlendFace", "string dynamicID", "string contentType", "string url", "string extraParams", "bool blend", "integer disp", "integer timer", "integer alpha", "integer face")
newFunc("string",	"osSetFontName", "string drawList", "string fontName")
newFunc("string",	"osSetFontSize", "string drawList", "integer fontSize")
newFunc("string",	"osSetPenCap", "string drawList", "string direction", "string type")
newFunc("string",	"osSetPenColor", "string drawList", "string color")
newFunc("string",	"osSetPenColour", "string drawList", "string colour")
newFunc("string",	"osSetPenSize", "string drawList", "integer penSize")
newFunc("",		"osSetStateEvents", "integer events")

-- OS time functions
--newFunc("string",	"osUnixTimeToTimestamp", "long time")

-- OS windlight functions
newFunc("",		"lsClearWindlightScene")
newFunc("list",		"lsGetWindlightScene", "list rules")
newFunc("integer",	"lsSetWindlightScene", "list rules")
newFunc("integer",	"lsSetWindlightSceneTargeted", "list rules", "key target")



-- Function implementations

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
    result = result .. LSL.llList2String(l, i)
  end
  return result
end

function --[[integer]] 	LSL.llGetListLength(--[[list]] l)
  return #l
end

function --[[string]]	LSL.llList2CSV(--[[list]] l)
  return LSL.llDumpList2String(l, ", ")
end

function --[[list]]	LSL.llCSV2List(--[[string]] text)
  local result = {}
  local i = 1
  local b = 1
  local len = string.len(text)
  local s, e

  -- Apparently llCSV2List() really is this dumb.  http://lslwiki.net/lslwiki/wakka.php?wakka=llCSV2List
  repeat
    s, e = string.find(text, ', ', b, true)
    if s then
      local temp = string.sub(text, b, s - 1)
      local s1, e1 = string.find(temp, '<', 1, true)

      -- Skip commas enclosed in <>, even if it's just garbage.
      if s1 then
        local s2, e2 = string.find(text, '>', b + e1, true)
        if s1 then
          temp = string.sub(text, b, e2)
          e = e2
        end
      end
      result[i] = temp
      i = i + 1
      b = e + 1
    end
  until nil == s

  return result
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

  if 'boolean' == type(result) then
    if result then result = 'true' else result = 'false' end
  end
  if 'table' == type(result) then
    -- Figure out if things are vectors or rotations, so we can output <> instead of [].
    if nil == result.x then
      result = '[' .. LSL.llDumpList2String(result, ', ') .. ']'
    else
      result = '<' .. LSL.llDumpList2String(result, ', ') .. '>'
    end
  end
  if result then return "" .. result else return "" end
end

function --[[rotation]]	LSL.llList2Rot(--[[list]] l,--[[integer]] index)
  local result = l[index+1]
  if nil == result then result = LSL.ZERO_ROTATION end
  -- Check if it's not an actual rotation, then return LSS.ZERO_ROTATION
  if 'table' ~= type(result) then result = LSL.ZERO_ROTATION end
  if nil == result.x then result = LSL.ZERO_ROTATION end
  if nil == result.y then result = LSL.ZERO_ROTATION end
  if nil == result.z then result = LSL.ZERO_ROTATION end
  if nil == result.s then result = LSL.ZERO_ROTATION end
  return result
end

function --[[vector]]	LSL.llList2Vector(--[[list]] l,--[[integer]] index)
  local result = l[index+1]
  if nil == result then result = LSL.ZERO_VECTOR end
  -- Check if it's not an actual rotation, then return LSS.ZERO_VECTOR
  if 'table' ~= type(result) then result = LSL.ZERO_VECTOR end
  if nil == result.x then result = LSL.ZERO_VECTOR end
  if nil == result.y then result = LSL.ZERO_VECTOR end
  if nil == result.z then result = LSL.ZERO_VECTOR end
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
  for i = 1,#l1 do
    result[x] = l1[i]
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

  for i = 1,start do
    result[x] = l[i]
    x = x + 1
  end
  for i = 1,#part do
    result[x] = part[i]
    x = x + 1
  end
  for i = eNd+2,#l do
    result[x] = l[i]
    x = x + 1
  end
  return result
end

function --[[list]]	LSL.llListSort(--[[list]] l,--[[integer]] stride,--[[integer]] ascending)
  local result = {}
  local x = 1

  -- TODO - Deal with stride and ascending.
  for i = 1,#l do
    result[x] = l[i];
    x = x + 1
  end
  table.sort(result)

  return result
end

function --[[list]]	LSL.llParseStringKeepNulls(--[[string]] In, --[[list]] l, --[[list]] l1)
  local result = {}
  local temp = {}
  local len = #l
  local slen = string.len(In)
  local b = 1
  local c = 1
  local lastE = 0

--[[
print("LSL.llParseStringKeepNulls(" .. In .. ") " .. #In)
for i, v in ipairs(l) do
  print("  [" .. v .. "]")
end
print(",")
for i, v in ipairs(l1) do
  print("  {" .. v .. "}")
end
]]

  -- Start scanning at the beginning of the string.
  repeat
    local f = {slen, -slen, 0}
    local t = l

    -- Search for the first of all the matches.
    for j = 1, len + #l1 do
      local k = j

      if j > len then t = l1;  k = j - len end
      local s, e = string.find(In, t[j], b, true)
      if s then
        if s < f[1] then
          f = {s, e, j}
          lastE = e
        end
      end
    end

    -- We found at least one of them, save it.
    if f[2] > 0 then
      temp[c] = {b, f[1] - 1}
      -- Split at and keep spacers.
      if f[3] > len then
        c = c + 1
        temp[c] = {f[1], f[2]}
      end
      c = c + 1
      b = f[2]
    end

    -- Continue on from the next character, possibly after the match.
    b = b + 1
  until b >= slen

  -- Save any left over, including the entire string if no matches found.
  if lastE < slen then
    temp[c] = {lastE + 1, slen}
  end

  -- Sort the findings.
  table.sort(temp, function(a, b) return a[1] < b[1] end)

  -- Construct the result.
  for i, v in ipairs(temp) do
    result[i] = string.sub(In, v[1], v[2])
  end

--[[
print("RESULT = ")
for i, v in ipairs(result) do
  print("  {" .. v .. "}")
end
]]

  return result
end

function --[[list]]	LSL.llParseString2List(--[[string]] In, --[[list]] l, --[[list]] l1)
  local temp = LSL.llParseStringKeepNulls(--[[string]] In, --[[list]] l, --[[list]] l1)
  local result = {}
  local c = 1

  -- Strip out the NULLS.
  for i, v in ipairs(temp) do
    if "" ~= v then
      result[c] = v
      c = c + 1
    end
  end

--[[
print("RESULT SANS NULLS = ")
for i, v in ipairs(result) do
  print("  {" .. v .. "}")
end
]]

  return result
end



-- LSL script functions

function --[[string]] LSL.llGetScriptName()
  return scriptName
end


-- LSL string functions

function --[[integer]] 	LSL.llStringLength(--[[string]] s)
  return string.len(s)
end


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

function LSL.llStringTrim(--[[string]] text, --[[integer]] Type)
  -- Trims spaces (ASCII 32), tabs (ASCII 9) '\t', and new lines (ASCII 10) '\n'
  -- Lua doesn't document what it considers to be "space characters"
  if (Type == LSL.STRING_TRIM_HEAD) or (Type == LSL.STRING_TRIM) then
    text = string.gsub(text, '^%s*(.*)', '%1')
  end
  if (Type == LSL.STRING_TRIM_TAIL) or (Type == LSL.STRING_TRIM) then
    text = string.gsub(text, '(.-)%s*$', '%1')
  end
  return text
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
--    msg("LSL.Lua: State change on " .. scriptName)
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
--  LSL.EOF = "\n\n\n"	-- Fix this up now.
  LSL.EOF = "EndOfFuckingAround"	-- Fix this up now.

  LSL.stateChange(x);
  waitAndProcess(false)
--  msg("LSL.Lua: Script quitting " .. scriptName)
end

function waitAndProcess(returnWanted)
  local Type = "event"

  if returnWanted then Type = "result" end
  while running do
    local message = Runnr.receive()
    if message then
--print('GOT MESSAGE for  script ' .. scriptName .. ' - "' .. message .. '"')
      -- TODO - should we be discarding return values while paused?  I don't think so, so we need to process those,
      if paused then
	if "start()" == message then paused = false  end
      else
	local result, errorMsg = loadstring(message)  -- "The environment of the returned function is the global environment."  Though normally, a function inherits it's environment from the function creating it.  Which is what we want.  lol
	if nil == result then
	  msg("Not a valid " .. Type .. ": " .. message .. "  ERROR MESSAGE: " .. errorMsg)
	else
	  -- Set the functions environment to ours, for the protection of the script, coz loadstring sets it to the global environment instead.
	  -- TODO - On the other hand, we will need the global environment when we call event handlers.  So we should probably stash it around here somewhere.
	  --        Meh, seems to be working fine as it is.  Or not.
	  setfenv(result, getfenv(1))
	  local status, result1 = pcall(result)
	  if not status then
	    msg("Error from " .. Type .. ": " .. message .. "  ERROR MESSAGE: " .. result1)
	  elseif result1 then
	    -- Check if we are waiting for a return, and got it.
	    if returnWanted and string.match(message, "^return ") then
--	      print("RETURNING " .. result1)
	      return result1
            end
	    -- Otherwise, just run it and keep looping.
	    -- TODO - Not sure why I had this here.  "sid" is not set anywhere, and SID would just send it to ourselves.
--	    status, errorMsg = Runnr.send(nil, result1)
--	    if not status then
--	      msg("Error sending results from " .. Type .. ": " .. message .. "  ERROR MESSAGE: " .. errorMsg)
--	    end
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
  for i, v in ipairs(constants) do
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

-- Misc support functions.

function LSL.listAdd(a, b)
  local i = 1
  local result = {}

  -- Deal with implicit typecasts.
  if 'table' ~= type(a) then a = {a} end
  if 'table' ~= type(b) then b = {b} end

  for j, v in ipairs(a) do
    table.insert(result, i, v)
    i = i + 1
  end

  for j, v in ipairs(b) do
    table.insert(result, i, v)
    i = i + 1
  end

  return result;
end

function LSL.listConcat(a, b)
  local i = table.maxn(a)
  local result = a

  table.insert(result, i + 1, b)
  return result;
end

-- Lua really hates 0, it's not false, and it can't be a table index.
function LSL.toBool(x)
  local t = type(x)

  if 'boolean' == t then return x end
  if 'number' == t then return (x ~= 0) end
  if 'nil' == t then return false end
  -- Is an empty string, empty list, zero vector/rotation false?  Fucked if I know.
  return true
end

return LSL;
