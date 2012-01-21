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

integer		TRUE			= 1;
integer		FALSE			= 0;

string		NULL_KEY		= "00000000-0000-0000-0000-000000000000";
string		EOF			= "\n\n\n";

rotation	ZERO_ROTATION		= <0.0, 0.0, 0.0, 1.0>;
vector		ZERO_VECTOR		= <0.0, 0.0, 0.0>;

// Functions.

vector		llGetPos(){}
rotation	llGetRot(){}
string		llGetObjectDesc(){}
		llSetObjectDesc(string text){}

rotation	llEuler2Rot(vector vec){}


string		llGetSubString(string text, integer start, integer end){}
list		llParseString2List(string in, list l, list l1){}

integer		llList2Integer(list l, integer index){}
string		llList2String(list l, integer index){}
list		llCSV2List(string text){}

key		llGetKey(){}

		llSay(integer channel, string text){}
		llOwnerSay(string text){}

		llMessageLinked(integer link, integer num, string text, key aKey){}

