#include "extantz.h"


static void _on_camera_input_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    GLData *gld = data;
    Evas_Event_Key_Down *ev = event_info;

    if (gld->move)
    {
	// TODO - Careful, gld->move MIGHT be read at the other end by another thread.  MIGHT, coz I really don't know at what point the camera animate routine is actually called.

	// Yes, we are dealing with the horrid Evas keyboard handling FUCKING STRING COMPARES!  Soooo ...
	// TODO - make this a hash lookup dammit.
	if (0 == strcmp(ev->key, "Escape"))
	{
	}
	else if  (0 == strcmp(ev->key, "Left"))
	    gld->move->r = 2.0;
	else if  (0 == strcmp(ev->key, "Right"))
	    gld->move->r = -2.0;
	else if  (0 == strcmp(ev->key, "Up"))
	    gld->move->x = 2.0;
	else if  (0 == strcmp(ev->key, "Down"))
	    gld->move->x = -2.0;
	else if  (0 == strcmp(ev->key, "Prior"))
	    gld->move->z = -2.0;
	else if  (0 == strcmp(ev->key, "Next"))
	    gld->move->z = 2.0;
	else if  (0 == strcmp(ev->key, "Home"))
	    gld->move->y = 2.0;
	else if  (0 == strcmp(ev->key, "End"))
	    gld->move->y = -2.0;
	else if  (0 == strcmp(ev->key, "space"))
	    gld->move->jump = 1.0;
	else
	    printf("Unexpected down keystroke - %s\n", ev->key);
    }
    else
	printf("Camera input not ready\n");
}

/* SL / OS camera controls
    up / down / w / s		moves avatar forward / backward
	shifted version does the same
	double tap triggers run mode / or fast fly mode
	    Running backwards turns your avatar to suit, walking does not.
    left / right / a / d	rotates avatar left / right, strafes in mouselook
	shifted version turns the avatar to walk sideways, so not really a strafe.
	So not sure if the "strafe" in mouse look turns the avatar as well?
    PgDn / c			crouch while it is held down	move up in flight mode
    PgUp			jump				move down in flight mode
    Home			toggle flying
    End				Nothing?
    Esc				return to third person view
    m				toggle mouse look
    mouse wheel			move view closer / further away from current focused object or avatar
    Alt left click		focus on some other object
    Ins				???
    Del				???
    BS				???
    Tab				???

    Mouse look is just first person view, moving mouse looks left / right / up / down.
	Not sure if the avatar rotates with left / right, but that's likely.

    mouse moves With the left mouse button held down -
				left / right		up / down
				---------------------------------
    for avatar			swings avatar around	zoom in and out of avatar
    for object			nothing
    alt				orbit left / right	zoom in and out
    alt ctrl			orbit left / right	orbit up / down
    alt shift			orbit left / right	zoom in and out
    alt ctrl shift		shift view left / right / up / down
    ctrl			Nothing?
    shift			Nothing?
    ctrl shift			Nothing?

    Need to also consider when looking at a moving object / avatar.

    I think there are other letter keys that duplicate arrow keys and such.  I'll look for them later, but I don't use them.
    No idea what the function keys are mapped to, but think it's various non camera stuff.
    I'm damn well leaving the Win/Command and Menu keys for the OS / window manager.  lol
    Keypad keys?  Not interested, I don't have them.
    Print Screen / SysRq, Pause / Break, other oddball keys, also not interested.
    NOTE - gonna have an easily programmable "bind key to command" thingy, like E17s, so that can deal with other keys.
	Should even let them be saveable so people can swap them with other people easily.

    TODO - implement things like space mouse, sixaxis, phone as controller, joysticks, data gloves, etc.
*/

/* A moveRotate array of floats.
 * X, Y, Z, and whatever the usual letters are for rotations.  lol
 * Each one means "move or rotate this much in this direction".
 * Where 1.0 means "what ever the standard move is if that key is held down".
 * So a keyboard move would just change it's part to 1.0 or -1.0 on key down,
 *   and back to 0.0 on key up.  Or 2.0 / -2.0 if in run mode.
 *   Which would even work in fly mode.
 * A joystick could be set to range over -2.0 to 2.0, and just set it's part directly.
 * A mouse look rotate, well will come to that when we need to.  B-)
 *   Setting the x or y to be the DIFFERENCE in window position of the mouse (-1.0 to 1.0) since the last frame.
 *
 * TODO - In the Elm_glview version, 2.0 seems to be correct speed for walking, but I thought 1.0 was in Evas_GL.
 */

static void _on_camera_input_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    GLData *gld = data;
    Evas_Event_Key_Up *ev = event_info;

    if (gld->move)
    {
	// TODO - Careful, gld->move MIGHT be read at the other end by another thread.  MIGHT, coz I really don't know at what point the camera animate routine is actually called.

	// Yes, we are dealing with the horrid Evas keyboard handling FUCKING STRING COMPARES!  Soooo ...
	// TODO - make this a hash lookup dammit.
	if (0 == strcmp(ev->key, "Escape"))
	{
	}
	else if  (0 == strcmp(ev->key, "Left"))
	    gld->move->r = 0.0;
	else if  (0 == strcmp(ev->key, "Right"))
	    gld->move->r = 0.0;
	else if  (0 == strcmp(ev->key, "Up"))
	    gld->move->x = 0.0;
	else if  (0 == strcmp(ev->key, "Down"))
	    gld->move->x = 0.0;
	  else if  (0 == strcmp(ev->key, "Prior"))
	    gld->move->z = 0.0;
	else if  (0 == strcmp(ev->key, "Next"))
	    gld->move->z = 0.0;
	else if  (0 == strcmp(ev->key, "Home"))
	    gld->move->y = 0.0;
	else if  (0 == strcmp(ev->key, "End"))
	    gld->move->y = 0.0;
	else if  (0 == strcmp(ev->key, "space"))
	    gld->move->jump = 0.0;
	else
	    printf("Unexpected up keystroke - %s\n", ev->key);
    }
    else
	printf("Camera input not ready\n");
}

// Elm style event callback.
static Eina_Bool _cb_event_GL(void *data, Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
    GLData *gld = data;
    Eina_Bool processed = EINA_FALSE;

    switch (type)
    {
	case EVAS_CALLBACK_KEY_DOWN :
	{
	    _on_camera_input_down(gld, evas_object_evas_get(obj), obj, event_info);
	    processed = EINA_TRUE;
	    break;
	}

	case EVAS_CALLBACK_KEY_UP :
	{
	    _on_camera_input_up(gld, evas_object_evas_get(obj), obj, event_info);
	    processed = EINA_TRUE;
	    break;
	}

	default :
	    printf("Unknown GL input event.\n");
    }

    return processed;
}


void cameraAdd(Evas_Object *win, GLData *gld)
{
  // In this code, we are making our own camera, so grab it's input when we are focused.
  evas_object_event_callback_add(win, EVAS_CALLBACK_KEY_DOWN, _on_camera_input_down, gld);
  evas_object_event_callback_add(win, EVAS_CALLBACK_KEY_UP,   _on_camera_input_up, gld);
  elm_object_event_callback_add(win, _cb_event_GL, gld);
}
