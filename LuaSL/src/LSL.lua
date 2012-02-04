-- A module of LSL stuffs.
-- TODO - currently there is a constants.lsl file.  Move it into here.
--   It contains LSL constants and ll*() functions stubs.
--   The compiler compiles this into a LSL_Scripts structure at startup, 
--   then uses that for function and variable lookups, as well as looking up in the current script.
--   Using a module means it gets compiled each time?  Maybe not if I can use bytecode files.  Perhaps LuaJIT caches these?

-- Use it like this -
-- local lsl = require 'lsl'

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

local LSL = {};

LSL.PI					= 3.14159265358979323846264338327950;
LSL.PI_BY_TWO				= LSL.PI / 2;		-- 1.57079632679489661923132169163975
LSL.TWO_PI				= LSL.PI * 2;		-- 6.28318530717958647692528676655900
LSL.DEG_TO_RAD				= LSL.PI / 180.0;	-- 0.01745329252
LSL.RAD_TO_DEG				= 180.0 / LSL.PI;	-- 57.2957795131
LSL.SQRT2				= 1.4142135623730950488016887242097;

LSL.CHANGED_INVENTORY			= 0x001;
LSL.CHANGED_COLOR			= 0x002;
LSL.CHANGED_SHAPE			= 0x004;
LSL.CHANGED_SCALE			= 0x008;
LSL.CHANGED_TEXTURE			= 0x010;
LSL.CHANGED_LINK			= 0x020;
LSL.CHANGED_ALLOWED_DROP		= 0x040;
LSL.CHANGED_OWNER			= 0x080;
LSL.CHANGED_REGION			= 0x100;
LSL.CHANGED_TELEPORT			= 0x200;
LSL.CHANGED_REGION_START		= 0x400;
LSL.CHANGED_MEDIA			= 0x800;

LSL.DEBUG_CHANNEL			= 2147483647;
LSL.PUBLIC_CHANNEL			= 0;

LSL.INVENTORY_ALL			= -1;
LSL.INVENTORY_NONE			= -1;
LSL.INVENTORY_TEXTURE			= 0;
LSL.INVENTORY_SOUND			= 1;
LSL.INVENTORY_LANDMARK			= 3;
LSL.INVENTORY_CLOTHING			= 5;
LSL.INVENTORY_OBJECT			= 6;
LSL.INVENTORY_NOTECARD			= 7;
LSL.INVENTORY_SCRIPT			= 10;
LSL.INVENTORY_BODYPART			= 13;
LSL.INVENTORY_ANIMATION			= 20;
LSL.INVENTORY_GESTURE			= 21;

LSL.ALL_SIDES				= -1;
LSL.LINK_SET				= -1;
LSL.LINK_ROOT				= 1;
LSL.LINK_ALL_OTHERS			= -2;
LSL.LINK_ALL_CHILDREN			= -3;
LSL.LINK_THIS				= -4;

LSL.PERM_ALL				= 0x7FFFFFFF;
LSL.PERM_COPY				= 0x00008000;
LSL.PERM_MODIFY				= 0x00004000;
LSL.PERM_MOVE				= 0x00080000;
LSL.PERM_TRANSFER			= 0x00002000;
LSL.MASK_BASE				= 0;
LSL.MASK_OWNER				= 1;
LSL.MASK_GROUP				= 2;
LSL.MASK_EVERYONE			= 3;
LSL.MASK_NEXT				= 4;
LSL.PERMISSION_DEBIT			= 0x0002;
LSL.PERMISSION_TAKE_CONTROLS		= 0x0004;
LSL.PERMISSION_TRIGGER_ANIMATION	= 0x0010;
LSL.PERMISSION_ATTACH			= 0x0020;
LSL.PERMISSION_CHANGE_LINKS		= 0x0080;
LSL.PERMISSION_TRACK_CAMERA		= 0x0400;
LSL.PERMISSION_CONTRAL_CAMERA		= 0x0800;

LSL.AGENT			 	= 0x01;
LSL.ACTIVE			 	= 0x02;
LSL.PASSIVE			 	= 0x04;
LSL.SCRIPTED		 		= 0x08;

LSL.OBJECT_UNKNOWN_DETAIL		= -1;

LSL.PRIM_BUMP_SHINY			= 19;
LSL.PRIM_COLOR				= 18;
LSL.PRIM_FLEXIBLE			= 21;
LSL.PRIM_FULLBRIGHT			= 20;
LSL.PRIM_GLOW				= 25;
LSL.PRIM_MATERIAL			= 2;
LSL.PRIM_PHANTOM			= 5;
LSL.PRIM_PHYSICS			= 3;
LSL.PRIM_POINT_LIGHT			= 23;
LSL.PRIM_POSITION			= 6;
LSL.PRIM_ROTATION			= 8;
LSL.PRIM_SIZE				= 7;
LSL.PRIM_TEMP_ON_REZ			= 4;
LSL.PRIM_TYPE				= 9;
LSL.PRIM_TYPE_OLD			= 1;
LSL.PRIM_TEXGEN				= 22;
LSL.PRIM_TEXTURE			= 17;
LSL.PRIM_TEXT				= 26;

LSL.PRIM_BUMP_NONE			= 0;
LSL.PRIM_BUMP_BRIGHT			= 1;
LSL.PRIM_BUMP_DARK			= 2;
LSL.PRIM_BUMP_WOOD			= 3;
LSL.PRIM_BUMP_BARK			= 4;
LSL.PRIM_BUMP_BRICKS			= 5;
LSL.PRIM_BUMP_CHECKER			= 6;
LSL.PRIM_BUMP_CONCRETE			= 7;
LSL.PRIM_BUMP_TILE			= 8;
LSL.PRIM_BUMP_STONE			= 9;
LSL.PRIM_BUMP_DISKS			= 10;
LSL.PRIM_BUMP_GRAVEL			= 11;
LSL.PRIM_BUMP_BLOBS			= 12;
LSL.PRIM_BUMP_SIDING			= 13;
LSL.PRIM_BUMP_LARGETILE			= 14;
LSL.PRIM_BUMP_STUCCO			= 15;
LSL.PRIM_BUMP_SUCTION			= 16;
LSL.PRIM_BUMP_WEAVE			= 17;

LSL.PRIM_HOLE_DEFAULT			= 0;
LSL.PRIM_HOLE_CIRCLE			= 16;
LSL.PRIM_HOLE_SQUARE			= 32;
LSL.PRIM_HOLE_TRIANGLE			= 48;

LSL.PRIM_MATERIAL_STONE			= 0;
LSL.PRIM_MATERIAL_METAL			= 1;
LSL.PRIM_MATERIAL_GLASS			= 2;
LSL.PRIM_MATERIAL_WOOD			= 3;
LSL.PRIM_MATERIAL_FLESH			= 4;
LSL.PRIM_MATERIAL_PLASTIC		= 5;
LSL.PRIM_MATERIAL_RUBBER		= 6;
LSL.PRIM_MATERIAL_LIGHT			= 7;

LSL.PRIM_SCULPT_TYPE_SPHERE		= 1;
LSL.PRIM_SCULPT_TYPE_TORUS		= 2;
LSL.PRIM_SCULPT_TYPE_PLANE		= 3;
LSL.PRIM_SCULPT_TYPE_CYLINDER		= 4;
LSL.PRIM_SCULPT_TYPE_MESH		= 5;
LSL.PRIM_SCULPT_TYPE_MIMESH		= 6;

LSL.PRIM_SHINY_NONE			= 0;
LSL.PRIM_SHINY_LOW			= 1;
LSL.PRIM_SHINY_MEDIUM			= 2;
LSL.PRIM_SHINY_HIGH			= 3;

LSL.PRIM_TYPE_BOX			= 0;
LSL.PRIM_TYPE_CYLINDER			= 1;
LSL.PRIM_TYPE_PRISM			= 2;
LSL.PRIM_TYPE_SPHERE			= 3;
LSL.PRIM_TYPE_TORUS			= 4;
LSL.PRIM_TYPE_TUBE			= 5;
LSL.PRIM_TYPE_RING			= 6;
LSL.PRIM_TYPE_SCULPT			= 7;

LSL.STRING_TRIM				= 3;
LSL.STRING_TRIM_HEAD			= 1;
LSL.STRING_TRIM_TAIL			= 2;

LSL.TRUE				= 1;
LSL.FALSE				= 0;

LSL.TYPE_INTEGER			= 1;
LSL.TYPE_FLOAT				= 2;
LSL.TYPE_STRING				= 3;
LSL.TYPE_KEY				= 4;
LSL.TYPE_VECTOR				= 5;
LSL.TYPE_ROTATION			= 6;
LSL.TYPE_INVALID			= 0;

LSL.NULL_KEY				= "00000000-0000-0000-0000-000000000000";
LSL.EOF					= "\n\n\n";

LSL.ZERO_ROTATION			= {0.0, 0.0, 0.0, 1.0};
LSL.ZERO_VECTOR				= {0.0, 0.0, 0.0};

-- TODO - Temporary dummy variables to got vector and rotation thingies to work for now.

LSL.s					= 1.0;
LSL.x					= 0.0;
LSL.y					= 0.0;
LSL.z					= 0.0;

-- Functions.

function --[[float]]	LSL.llPow(--[[float]] number,--[[float]] places) return 0.0 end;
function --[[float]]	LSL.llFrand(--[[float]] max) return 0.0 end;
function --[[integer]]	LSL.llRound(--[[float]] number) return 0 end;

function --[[key]]	LSL.llDetectedKey(--[[integer]] index) return LSL.NULL_KEY end;
function --[[key]]	LSL.llDetectedGroup(--[[integer]] index) return LSL.NULL_KEY end;
function --[[integer]] 	LSL.llSameGroup(--[[key]] avatar) return 0 end;

function --[[float]] 	LSL.llGetAlpha(--[[integer]] side) return 0.0 end;
function 		LSL.llSetAlpha(--[[float]] alpha,--[[integer]] side) end;
function 		LSL.llSetColor(--[[vector]] colour,--[[integer]] side) end;
function 		LSL.llSetPrimitiveParams(--[[list]] params) end;
function 		LSL.llSetScale(--[[vector]] scale) end;
function 		LSL.llSetSitText(--[[string]] text) end;
function 		LSL.llSetText(--[[string]] text, --[[vector]] colour,--[[float]] alpha) end;
function 		LSL.llSitTarget(--[[vector]] pos, --[[rotation]] rot) end;

function --[[integer]] 	LSL.llGetLinkNumber() return 0 end;
function --[[string]]	LSL.llGetObjectDesc() return "" end;
function 		LSL.llSetObjectDesc(--[[string]] text) end;
function --[[string]]	LSL.llGetObjectName() return "" end;
function 		LSL.llSetObjectName(--[[string]] text) end;

function --[[string]]	LSL.llGetInventoryName(--[[integer]] tyPe,--[[integer]] index) return "" end;
function --[[integer]] 	LSL.llGetInventoryNumber(--[[integer]] tyPe) return 0 end;
function --[[integer]] 	LSL.llGetInventoryType(--[[string]] name) return 0 end;
function --[[key]]	LSL.llGetNotecardLine(--[[string]] name,--[[integer]] index) return LSL.NULL_KEY end;

function 		LSL.llDie() end;
function --[[integer]] 	LSL.llGetFreeMemory() return 0 end;
function --[[string]]	LSL.llGetScriptName() return "" end;
function --[[float]] 	LSL.llGetTime() return 0.0 end;
function 		LSL.llResetOtherScript(--[[string]] name) end;
function 		LSL.llResetScript() end;
function 		LSL.llResetTime() end;
function 		LSL.llSetScriptState(--[[string]] name,--[[integer]] running) end;
function 		LSL.llSetTimerEvent(--[[float]] seconds) end;
function 		LSL.llSleep(--[[float]] seconds) end;

function 		LSL.llPlaySound(--[[string]] name,--[[float]] volume) end;
function 		LSL.llRezObject(--[[string]] name, --[[vector]] position, --[[vector]] velocity, --[[rotation]] rot,--[[integer]] channel) end;
function 		LSL.llRezAtRoot(--[[string]] name, --[[vector]] position, --[[vector]] velocity, --[[rotation]] rot,--[[integer]] channel) end;

function --[[vector]]	LSL.llGetPos() return LSL.ZERO_VECTOR end;
function 		LSL.llSetPos(--[[vector]] pos) end;
function --[[rotation]]	LSL.llGetRot() return LSL.ZERO_ROTATION end;
function 		LSL.llSetRot(--[[rotation]] rot) end;

function --[[rotation]]	LSL.llEuler2Rot(--[[vector]] vec) return LSL.ZERO_ROTATION end;
function --[[vector]]	LSL.llRot2Euler(--[[rotation]] rot)  return LSL.ZERO_VECTOR end;

function --[[string]]	LSL.llGetSubString(--[[string]] text,--[[integer]] start,--[[integer]] eNd) return "" end;
function --[[integer]] 	LSL.llStringLength(--[[string]] text) return 0 end;
function --[[string]]	LSL.llStringTrim(--[[string]] text,--[[integer]] tyPe) return "" end;
function --[[integer]] 	LSL.llSubStringIndex(--[[string]] text, --[[string]] sub) return 0 end;
function --[[list]]	LSL.llParseString2List(--[[string]] In, --[[list]] l, --[[list]] l1) return {} end;
function --[[list]]	LSL.llParseStringKeepNulls(--[[string]] In, --[[list]] l, --[[list]] l1) return {} end;

function --[[list]]	LSL.llCSV2List(--[[string]] text) return {} end;
function --[[list]]	LSL.llDeleteSubList(--[[list]] l,--[[integer]] start,--[[integer]] eNd) return {} end;
function --[[string]]	LSL.llDumpList2String(--[[list]] l, --[[string]] separator) return "" end;
function --[[string]]	LSL.llList2CSV(--[[list]] l) return "" end;
function --[[float]] 	LSL.llList2Float(--[[list]] l,--[[integer]] index) return 0.0 end;
function --[[integer]] 	LSL.llList2Integer(--[[list]] l,--[[integer]] index) return 0 end;
function --[[key]]	LSL.llList2Key(--[[list]] l,--[[integer]] index) return LSL.NULL_KEY end;
function --[[list]]	LSL.llList2List(--[[list]] l,--[[integer]] start,--[[integer]] eNd) return {} end;
function --[[string]]	LSL.llList2String(--[[list]] l,--[[integer]] index) return "" end;
function --[[rotation]]	LSL.llList2Rotation(--[[list]] l,--[[integer]] index) return LSL.ZERO_ROTATION end;
function --[[vector]]	LSL.llList2Vector(--[[list]] l,--[[integer]] index) return LSL.ZERO_VECTOR end;
function --[[integer]] 	LSL.llListFindList(--[[list]] l, --[[list]] l1) return 0 end;
function --[[list]]	LSL.llListInsertList(--[[list]] l, --[[list]] l1,--[[integer]] index) return {} end;
function --[[integer]] 	LSL.llGetListLength(--[[list]] l) return 0 end;
function --[[list]]	LSL.llListReplaceList(--[[list]] l, --[[list]] part,--[[integer]] start,--[[integer]] eNd) return {} end;
function --[[list]]	LSL.llListSort(--[[list]] l,--[[integer]] stride,--[[integer]] ascending) return {} end;

function --[[key]]	LSL.llAvatarOnSitTarget() return LSL.NULL_KEY end;
function --[[list]]	LSL.llGetAnimationList(--[[key]] id) return {} end;
function --[[key]]	LSL.llGetKey() return LSL.NULL_KEY end;
function --[[key]]	LSL.llGetOwner() return LSL.NULL_KEY end;
function --[[integer]] 	LSL.llGetPermissions() return 0 end;
function --[[key]]	LSL.llGetPermissionsKey() return LSL.NULL_KEY end;
function --[[string]]	LSL.llKey2Name(--[[key]] avatar) return "" end;
function 		LSL.llRequestPermissions(--[[key]] avatar,--[[integer]] perms) end;
function 		LSL.llStartAnimation(--[[string]] anim) end;
function 		LSL.llStopAnimation(--[[string]] anim) end;
function 		LSL.llUnSit(--[[key]] avatar) end;

function 		LSL.llDialog(--[[key]] avatar, --[[string]] caption, --[[list]] arseBackwardsMenu,--[[integer]] channel) end;
function --[[integer]] 	LSL.llListen(--[[integer]] channel, --[[string]] name, --[[key]] id, --[[string]] msg) return 0 end;
function 		LSL.llListenRemove(--[[integer]] handle) end;
function 		LSL.llOwnerSay(--[[string]] text) end;
function 		LSL.llSay(--[[integer]] channel, --[[string]] text) end;
function 		LSL.llShout(--[[integer]] channel, --[[string]] text) end;
function 		LSL.llWhisper(--[[integer]] channel, --[[string]] text) end;

function 		LSL.llMessageLinked(--[[integer]] link,--[[integer]] num, --[[string]] text, --[[key]] aKey) end;


return LSL;

