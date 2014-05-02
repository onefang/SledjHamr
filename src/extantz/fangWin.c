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
//	else if  (0 == strcmp(ev->key, "Prior"))
//	    ;
//	else if  (0 == strcmp(ev->key, "Next"))
//	    ;
//	else if  (0 == strcmp(ev->key, "Home"))
//	    ;
//	else if  (0 == strcmp(ev->key, "End"))
//	    ;
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
//	   else if  (0 == strcmp(ev->key, "Prior"))
//	    ;
//	else if  (0 == strcmp(ev->key, "Next"))
//	    ;
//	else if  (0 == strcmp(ev->key, "Home"))
//	    ;
//	else if  (0 == strcmp(ev->key, "End"))
//	    ;
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

// Elm inlined image windows needs this to change focus on mouse click.
// Evas style event callback.
static void _cb_mouse_down_elm(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
//    GLData *gld = data;
    Evas_Event_Mouse_Down *ev = event_info;

    if (1 == ev->button)
	elm_object_focus_set(obj, EINA_TRUE);
}

static void cb_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *orig = data;
   Evas_Coord x, y;
   Evas_Map *p;
   int i, w, h;

   if (!ev->buttons) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(obj,
                    x + (ev->cur.canvas.x - ev->prev.output.x),
                    y + (ev->cur.canvas.y - ev->prev.output.y));
   evas_object_image_size_get(orig, &w, &h);
   p = evas_map_new(4);
   evas_object_map_enable_set(orig, EINA_TRUE);
//   evas_object_raise(orig);
   for (i = 0; i < 4; i++)
     {
        Evas_Object *hand;
        char key[32];

        snprintf(key, sizeof(key), "h-%i\n", i);
        hand = evas_object_data_get(orig, key);
        evas_object_raise(hand);
        evas_object_geometry_get(hand, &x, &y, NULL, NULL);
        x += 15;
        y += 15;
        evas_map_point_coord_set(p, i, x, y, 0);
        if (i == 0) evas_map_point_image_uv_set(p, i, 0, 0);
        else if (i == 1) evas_map_point_image_uv_set(p, i, w, 0);
        else if (i == 2) evas_map_point_image_uv_set(p, i, w, h);
        else if (i == 3) evas_map_point_image_uv_set(p, i, 0, h);
     }
   evas_object_map_set(orig, p);
   evas_map_free(p);
}

static void create_handles(Evas_Object *obj)
{
   int i;
   Evas_Coord x, y, w, h;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   for (i = 0; i < 4; i++)
     {
        Evas_Object *hand;
        char buf[PATH_MAX];
        char key[32];

        hand = evas_object_image_filled_add(evas_object_evas_get(obj));
        evas_object_resize(hand, 31, 31);
        snprintf(buf, sizeof(buf), "%s/pt.png", elm_app_data_dir_get());
        evas_object_image_file_set(hand, buf, NULL);
        if (i == 0)      evas_object_move(hand, x     - 15, y     - 15);
        else if (i == 1) evas_object_move(hand, x + w - 15, y     - 15);
        else if (i == 2) evas_object_move(hand, x + w - 15, y + h - 15);
        else if (i == 3) evas_object_move(hand, x     - 15, y + h - 15);
        evas_object_event_callback_add(hand, EVAS_CALLBACK_MOUSE_MOVE, cb_mouse_move, obj);
        evas_object_show(hand);
        snprintf(key, sizeof(key), "h-%i\n", i);
        evas_object_data_set(obj, key, hand);
     }
}

Evas_Object *fang_win_add(GLData *gld)
{
    Evas_Object *win, *bg;

    // In theory this should create an EWS window, in practice, I'm not seeing any difference.
    // Guess I'll have to implement my own internal window manager.  I don't think a basic one will be that hard.  Famous last words.
//    elm_config_engine_set("ews");
    win = elm_win_add(gld->win, "inlined", ELM_WIN_INLINED_IMAGE);
    // On mouse down we try to shift focus to the backing image, this seems to be the correct thing to force focus onto it's widgets.
    // According to the Elm inlined image window example, this is what's needed to.
    evas_object_event_callback_add(elm_win_inlined_image_object_get(win), EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_elm, gld);
    elm_win_alpha_set(win, EINA_TRUE);

    // Apparently transparent is not good enough for ELM backgrounds, so make it a rectangle.
    // Apparently coz ELM prefers stuff to have edjes.  A bit over the top if all I want is a transparent rectangle.
    bg = evas_object_rectangle_add(evas_object_evas_get(win));
    evas_object_color_set(bg, 50, 0, 100, 100);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, bg);
    evas_object_show(bg);

    return win;
}

void fang_win_complete(GLData *gld, Evas_Object *win, int x, int y, int w, int h)
{
    // image object for win is unlinked to its pos/size - so manual control
    // this allows also for using map and other things with it.
    evas_object_move(elm_win_inlined_image_object_get(win), x, y);
    // Odd, it needs to be resized twice.  WTF?
    evas_object_resize(win, w, h);
    evas_object_resize(elm_win_inlined_image_object_get(win), w, h);
    evas_object_show(win);
    create_handles(elm_win_inlined_image_object_get(win));
}

void overlay_add(GLData *gld)
{
    Evas_Object *bg;

    // There are many reasons for this window.
    // The first is to cover the GL and provide something to click on to change focus.
    // The second is to provide something to click on for all the GL type clicking stuff that needs to be done.  In other words, no click through, we catch the clicks here.
    //   So we can probably avoid the following issue -
    //     How to do click through?  evas_object_pass_events_set(rectangle, EINA_TRUE), and maybe need to do that to the underlaying window to?
    //     Though if the rectangle is entirely transparent, or even hidden, events might pass through anyway.
    //   Gotta have click through on the parts where there's no other window.
    // The third is to have the other windows live here.
    //   This idea doesn't work, as it breaks the damn focus again.
    //   Don't think it's needed anyway.
    // While on the subject of layers, need a HUD layer of some sort, but Irrlicht might support that itself.

    gld->winwin = elm_win_add(gld->win, "inlined", ELM_WIN_INLINED_IMAGE);
    // On mouse down we try to shift focus to the backing image, this seems to be the correct thing to force focus onto it's widgets.
    // According to the Elm inlined image window example, this is what's needed to.
    evas_object_event_callback_add(elm_win_inlined_image_object_get(gld->winwin), EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_elm, gld);
    // In this code, we are making our own camera, so grab it's input when we are focused.
    evas_object_event_callback_add(gld->winwin, EVAS_CALLBACK_KEY_DOWN, _on_camera_input_down, gld);
    evas_object_event_callback_add(gld->winwin, EVAS_CALLBACK_KEY_UP,   _on_camera_input_up, gld);
    elm_object_event_callback_add(gld->winwin, _cb_event_GL, gld);

    elm_win_alpha_set(gld->winwin, EINA_TRUE);
    // Apparently transparent is not good enough for ELM backgrounds, so make it a rectangle.
    // Apparently coz ELM prefers stuff to have edjes.  A bit over the top if all I want is a transparent rectangle.
    bg = evas_object_rectangle_add(evas_object_evas_get(gld->winwin));
    evas_object_color_set(bg, 0, 0, 0, 0);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(gld->winwin, bg);
    evas_object_show(bg);

    // image object for win is unlinked to its pos/size - so manual control
    // this allows also for using map and other things with it.
    evas_object_move(elm_win_inlined_image_object_get(gld->winwin), 0, 0);
    // Odd, it needs to be resized twice.  WTF?
    evas_object_resize(gld->winwin, gld->win_w, gld->win_h);
    evas_object_resize(elm_win_inlined_image_object_get(gld->winwin), gld->win_w, gld->win_h);
    evas_object_show(gld->winwin);
}
