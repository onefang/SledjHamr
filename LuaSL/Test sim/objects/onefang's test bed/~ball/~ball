// MLPV2 Version 2.3 by Learjeff Innis.  Based on
// MLP MULTI-LOVE-POSE V1.2 - Copyright (c) 2006, by Miffy Fluffy (BSD License)
// OpenSim port by Jez Ember
// Meta 7 fixes by onefang Rejected

// To make ball phantom, put "*" as the first character in the ball's description
// (The ball in MLP object's inventory should be non-phantom.)
// The rest of the description, if any, is used for the sit pie menu and floating text.
// To make this take effect, use STOP to unrez the balls, and then select any pose.

integer Chan;
integer Group;
integer visible = TRUE;
integer Adjusting;
key     Avatar;
string  Name;
integer Handle;

// 15 color support, thanks to Liz Silverstein
// Color is passed as a string by object chat (from menu via poser*)
    
list colors = [ <0.0,0.0,0.0>,          // 0 = HIDE
                <0.835,0.345,0.482>,    // 1 = PINK
                <0.353,0.518,0.827>,    // 2 = BLUE
                <0.635,0.145,0.282>,    // 3 = PINK2 - Dark pink
                <0.153,0.318,0.627>,    // 4 = BLUE2 - Dark blue
                <0.128,0.500,0.128>,    // 5 = GREEN
                <1.000,0.000,1.000>,    // 6 = MAGENTA
                <1.000,0.000,0.000>,    // 7 = RED
                <1.000,0.500,0.000>,    // 8 = ORANGE
                <1.000,1.000,1.000>,    // 9 = WHITE
                <0.0,0.0,0.0>,          // 10 = BLACK
                <1.0,1.0,0.0>,          // 11 = YELLOW
                <0.0,0.8,0.8>,          // 12 = CYAN
                <0.5,0.0,0.0>,          // 13 = RED2
                <0.0,0.5,0.5>,          // 14 = TEAL
                <0.0,0.25,0.25>];       // 15 = GREEN2


render() {
    if (!visible || (Avatar != NULL_KEY && !Adjusting)) {
        // hidden
        llSetScale(<0.01,0.01,0.01>);
        llSetAlpha(0.0, ALL_SIDES);
        llSetText("",<1.0,1.0,1.0>,1.0);    
    } else if (Avatar != NULL_KEY && Adjusting) {
        // sitting and adjusting
        llSetAlpha(0.2,ALL_SIDES);
        llSetText("Adjust",<1.0,1.0,1.0>,1.0);
        llSetScale(<0.1,0.1,5.0>);
    } else {
        // shown
        llSetAlpha(1.0, ALL_SIDES);
        llSetScale(<0.2,0.2,0.2>);
        if (Adjusting) {
            llSetText("Adjust",<1.0,1.0,1.0>,1.0);
        } else {
            llSetText(Name,<1.0,1.0,1.0>,1.0);
        }
    }
}

show() {
    visible = TRUE;
    render();
}

hide() {
    visible = FALSE;
    render();
}

default {
    on_rez(integer channel) {
        Name = llGetObjectDesc();
        if (Name == "" || Name == "(No Description)") {
            Name = "LOVE";
        } else {
            if (llSubStringIndex(Name, "*") == 0) {
                llSetPrimitiveParams([PRIM_PHANTOM, TRUE]);
                Name = llGetSubString(Name, 1, -1);
            }
            if (Name == "none") {
                Name = "";
            }
        }
        llSitTarget(<0.0,0.0,-0.1>,ZERO_ROTATION);
        if (Name != "") {
            llSetSitText(Name);
        }
        Avatar = NULL_KEY;
        Chan = channel;
        Group = 0;

        if (Chan != 0) {
            // register listen and start timer, unless ball was dragged from inv
            llListenRemove(Handle);
            Handle = llListen(Chan,"",NULL_KEY,"");
            llSetTimerEvent(600.0);
        }
    }

    changed(integer change) {
        if (change != CHANGED_LINK) return;
        Avatar = llAvatarOnSitTarget();
        if (Group) {
            if (Avatar != NULL_KEY && !llSameGroup(Avatar)) {
                llUnSit(Avatar);
                llWhisper(0,"no permission to use poseball");
                return;
            }
        }
        llSay(Chan+8,(string)Avatar);     //requests perm, sets animation
        render();
        // if (visible & Avatar == NULL_KEY) show(); else hide();
    }

    listen(integer channel, string name, key object, string str) {
        integer ix;
        ix = llSubStringIndex(str,">");    
        if (ix != -1) {
            llSetRot((rotation)llGetSubString(str,ix+1,-1));
            llSetPos((vector)llGetSubString(str,0,ix));
        } else if (str == "0") {    //HIDE
            hide();
        } else if (str == "SHOW") { //SHOW
            show();
        } else if (str == "ADJUST|1") {
            Adjusting = TRUE;
            render();
        } else if (str == "ADJUST|0") {
            Adjusting = FALSE;
            render();
        } else if (str == "SAVE") {
            llSay(Chan+16,(string)llGetPos()+(string)llGetRot());
        } else if (str == "GROUP") {
            Group = 1;
        } else if (str == "ALL") {
            Group = 0;
        } else if (str == "DIE") {
            llSay(Chan+8, (string)NULL_KEY);    //msg to poser (don't reanimate after STOP)
            llDie();
        } else if (str == "LIVE") {
            llSetTimerEvent(600.0);
        } else {
            list ldata = llParseString2List(str, ["|"], []);
            integer colorIx = (integer) llList2String(ldata,0);
            string ballIx = llList2String(ldata,1);
            Adjusting = (integer) llList2String(ldata,2);
            if ((colorIx > 0) && (colorIx < 16)) { // this must be a color setting  
                llSetColor(llList2Vector(colors, colorIx),ALL_SIDES);     //pull the color out of the list
                render();
                llSetObjectName("~ball" + ballIx);
            }
        }
    }

    timer() {                       //not heard "LIVE" from menu for a while: suicide
        llDie();
    }
}
