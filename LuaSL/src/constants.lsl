float		PI			= 3.14159265358979323846264338327950;
float		PI_BY_TWO		= PI / 2;	// 1.57079632679489661923132169163975
float		TWO_PI			= PI * 2;	// 6.28318530717958647692528676655900
float		DEG_TO_RAD		= PI / 180.0;	// 0.01745329252
float		RAD_TO_DEG		= 180.0 / PI;	// 57.2957795131
float		SQRT2			= 1.4142135623730950488016887242097;

integer		ALL_SIDES		= -1;
integer		LINK_SET		= -1;
integer		LINK_ROOT		= 1;
integer		LINK_ALL_OTHERS		= -2;
integer		LINK_ALL_CHILDREN	= -3;
integer		LINK_THIS		= -4;

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

integer		TRUE			= 1;
integer		FALSE			= 0;

string		NULL_KEY		= "00000000-0000-0000-0000-000000000000";
string		EOF			= "\n\n\n";

rotation	ZERO_ROTATION		= <0.0, 0.0, 0.0, 1.0>;
vector		ZERO_VECTOR		= <0.0, 0.0, 0.0>;

// Functions.

float		llPow(float number, float places){}
integer		llRound(float number){}


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
		llResetOtherScript(string name){}
		llResetScript(){}
		llSetScriptState(string name, integer running){}

vector		llGetPos(){}
rotation	llGetRot(){}

rotation	llEuler2Rot(vector vec){}
vector		llRot2Euler(rotation rot){}

string		llGetSubString(string text, integer start, integer end){}
integer		llSubStringIndex(string text, string sub){}
list		llParseString2List(string in, list l, list l1){}

float		llList2Float(list l, integer index){}
integer		llList2Integer(list l, integer index){}
key		llList2Key(list l, integer index){}
string		llList2String(list l, integer index){}
rotation	llList2Rotation(list l, integer index){}
vector		llList2Vector(list l, integer index){}
list		llCSV2List(string text){}
integer		llGetListLength(list l){}
list		llListReplaceList(list l, list part, integer start, integer end){}
list		llListSort(list l, integer stride, integer ascending){}

key		llGetKey(){}
		llSleep(float seconds){}

		llOwnerSay(string text){}
		llSay(integer channel, string text){}
		llShout(integer channel, string text){}
		llWhisper(integer channel, string text){}

		llMessageLinked(integer link, integer num, string text, key aKey){}




