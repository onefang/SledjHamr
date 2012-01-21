float		PI			= 3.14159265358979323846264338327950;
float		PI_BY_TWO		= PI / 2;	// 1.57079632679489661923132169163975
float		TWO_PI			= PI * 2;	// 6.28318530717958647692528676655900
float		DEG_TO_RAD		= PI / 180.0;	// 0.01745329252
float		RAD_TO_DEG		= 180.0 / PI;	// 57.2957795131
float		SQRT2			= 1.4142135623730950488016887242097;

integer		CHANGED_INVENTORY	= 0x001;
integer		CHANGED_COLOR		= 0x002;
integer		CHANGED_SHAPE		= 0x004;
integer		CHANGED_SCALE		= 0x008;
integer		CHANGED_TEXTURE		= 0x010;
integer		CHANGED_LINK		= 0x020;
integer		CHANGED_ALLOWED_DROP	= 0x040;
integer		CHANGED_OWNER		= 0x080;
integer		CHANGED_REGION		= 0x100;
integer		CHANGED_TELEPORT	= 0x200;
integer		CHANGED_REGION_START	= 0x400;
integer		CHANGED_MEDIA		= 0x800;

integer		DEBUG_CHANNEL		= 2147483647;
integer		PUBLIC_CHANNEL		= 0;

integer		INVENTORY_ALL		= -1;
integer		INVENTORY_NONE		= -1;
integer		INVENTORY_TEXTURE	= 0;
integer		INVENTORY_SOUND		= 1;
integer		INVENTORY_LANDMARK	= 3;
integer		INVENTORY_CLOTHING	= 5;
integer		INVENTORY_OBJECT	= 6;
integer		INVENTORY_NOTECARD	= 7;
integer		INVENTORY_SCRIPT	= 10;
integer		INVENTORY_BODYPART	= 13;
integer		INVENTORY_ANIMATION	= 20;
integer		INVENTORY_GESTURE	= 21;

integer		ALL_SIDES		= -1;
integer		LINK_SET		= -1;
integer		LINK_ROOT		= 1;
integer		LINK_ALL_OTHERS		= -2;
integer		LINK_ALL_CHILDREN	= -3;
integer		LINK_THIS		= -4;

integer		PERM_ALL			= 0x7FFFFFFF;
integer		PERM_COPY			= 0x00008000;
integer		PERM_MODIFY			= 0x00004000;
integer		PERM_MOVE			= 0x00080000;
integer		PERM_TRANSFER			= 0x00002000;
integer		MASK_BASE			= 0;
integer		MASK_OWNER			= 1;
integer		MASK_GROUP			= 2;
integer		MASK_EVERYONE			= 3;
integer		MASK_NEXT			= 4;
integer		PERMISSION_DEBIT		= 0x0002;
integer		PERMISSION_TAKE_CONTROLS	= 0x0004;
integer		PERMISSION_TRIGGER_ANIMATION	= 0x0010;
integer		PERMISSION_ATTACH		= 0x0020;
integer		PERMISSION_CHANGE_LINKS		= 0x0080;
integer		PERMISSION_TRACK_CAMERA		= 0x0400;
integer		PERMISSION_CONTRAL_CAMERA	= 0x0800;

integer		AGENT			 = 0x01;
integer		ACTIVE			 = 0x02;
integer		PASSIVE			 = 0x04;
integer		SCRIPTED		 = 0x08;

integer		OBJECT_UNKNOWN_DETAIL	= -1;

integer		TRUE			= 1;
integer		FALSE			= 0;

string		NULL_KEY		= "00000000-0000-0000-0000-000000000000";
string		EOF			= "\n\n\n";

rotation	ZERO_ROTATION		= <0.0, 0.0, 0.0, 1.0>;
vector		ZERO_VECTOR		= <0.0, 0.0, 0.0>;

// Functions.

float		llPow(float number, float places){}
integer		llRound(float number){}

key		llDetectedKey(integer index){}
key		llDetectedGroup(integer index){}
key		llSameGroup(key avatar){}

float		llGetAlpha(integer side){}
		llSetAlpha(float alpha, integer side){}
integer		llGetLinkNumber(){}
string		llGetObjectDesc(){}
		llSetObjectDesc(string text){}
string		llGetObjectName(){}
		llSetObjectName(string text){}

string		llGetInventoryName(integer type, integer index){}
integer		llGetInventoryNumber(integer type){}
integer		llGetInventoryType(string name){}
key		llGetNotecardLine(string name, integer index){}

integer		llGetFreeMemory(){}
string		llGetScriptName(){}
float		llGetTime(){}
		llResetOtherScript(string name){}
		llResetScript(){}
		llResetTime(){}
		llSetScriptState(string name, integer running){}
		llSetTimerEvent(float seconds){}

		llPlaySound(string name, float volume){}
		llRezObject(string name, vector position, vector velocity, rotation rot, integer channel);

vector		llGetPos(){}
rotation	llGetRot(){}

rotation	llEuler2Rot(vector vec){}
vector		llRot2Euler(rotation rot){}

string		llGetSubString(string text, integer start, integer end){}
integer		llSubStringIndex(string text, string sub){}
list		llParseString2List(string in, list l, list l1){}
list		llParseStringKeepNulls(string in, list l, list l1){}

list		llCSV2List(string text){}
list		llDeleteSubList(list l, integer start, integer end){}
string		llList2CSV(list l){}
float		llList2Float(list l, integer index){}
integer		llList2Integer(list l, integer index){}
key		llList2Key(list l, integer index){}
list		llList2List(list l, integer start, integer end){}
string		llList2String(list l, integer index){}
rotation	llList2Rotation(list l, integer index){}
vector		llList2Vector(list l, integer index){}
integer		llGetListLength(list l){}
list		llListReplaceList(list l, list part, integer start, integer end){}
list		llListSort(list l, integer stride, integer ascending){}

list		llGetAnimationList(key id){}
key		llGetKey(){}
key		llGetOwner(){}
integer		llGetPermissions(){}
key		llGetPermissionsKey(){}
string		llKey2Name(key avatar){}
		llRequestPermissions(key avatar, integer perms){}
		llStartAnimation(string anim){}
		llStopAnimation(string anim){}

		llSleep(float seconds){}

		llDialog(key avatar, string caption, list arseBackwardsMenu, integer channel){}
		llListen(integer channel, string name, key id, string msg){}
		llOwnerSay(string text){}
		llSay(integer channel, string text){}
		llShout(integer channel, string text){}
		llWhisper(integer channel, string text){}

		llMessageLinked(integer link, integer num, string text, key aKey){}


