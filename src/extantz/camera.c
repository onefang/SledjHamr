#include "extantz.h"



#define  DEGREE_TO_RADIAN(x)     (((x) * M_PI) / 180.0)


static inline void euler_to_quaternion(Eina_Quaternion *out, Evas_Real x, Evas_Real y, Evas_Real z)
{
  // Assuming the angles are in radians.
  Evas_Real c1 = cos(y / 2.0);  // heading  (yaw/spin)  ..  y
  Evas_Real s1 = sin(y / 2.0);
  Evas_Real c2 = cos(z / 2.0);  // attitude (pitch)     ..  x
  Evas_Real s2 = sin(z / 2.0);
  Evas_Real c3 = cos(x / 2.0);  // bank     (roll)      ..  z
  Evas_Real s3 = sin(x / 2.0);
  Evas_Real c1c2 = c1 * c2;
  Evas_Real s1s2 = s1 * s2;

  out->w =c1c2 * c3 - s1s2 * s3;
  out->x =c1c2 * s3 + s1s2 * c3;
  out->y =s1 * c2 * c3 + c1 * s2 * s3;
  out->z =c1 * s2 * c3 - s1 * c2 * s3;
}


Eina_Bool animateCamera(Scene_Data *scene)
{
  Eina_Quaternion rotate, orient, result, move;
  Evas_Real x, y, z;
  Eo *n = scene->avatar_node;

  if (n)
  {
    evas_canvas3d_node_orientation_get(n, EVAS_CANVAS3D_SPACE_PARENT, &orient.x, &orient.y, &orient.z, &orient.w);
    euler_to_quaternion(&rotate, DEGREE_TO_RADIAN(scene->move->r), DEGREE_TO_RADIAN(scene->move->s), DEGREE_TO_RADIAN(scene->move->p));
    eina_quaternion_mul(&result, &orient, &rotate);
    eina_quaternion_normalized(&result, &result);
    evas_canvas3d_node_orientation_set(n, result.x, result.y, result.z, result.w);

    eina_quaternion_set(&move, -scene->move->x, scene->move->y, -scene->move->z, 0);
    evas_canvas3d_node_position_get(n, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
    eina_quaternion_mul(&rotate, &result, &move);
    eina_quaternion_conjugate(&result, &result);
    eina_quaternion_mul(&move, &rotate, &result);
    x += move.x;
    y += move.y;
    z += move.z;
    evas_canvas3d_node_position_set(n, x, y, z);
  }

  return EINA_TRUE;
}

static void _on_camera_input_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  cameraMove		*move	= data;
  Evas_Event_Key_Down	*ev	= event_info;

  if (move)
  {
    // TODO - Careful, gld->move MIGHT be read at the other end by another thread.  MIGHT, coz I really don't know at what point the camera animate routine is actually called.

    // Yes, we are dealing with the horrid Evas keyboard handling FUCKING STRING COMPARES!  Soooo ...
    // TODO - make this a hash lookup dammit.
    if (0 == strcmp(ev->key, "Escape"))
    {
    }
    else if (0 == strcmp(ev->key, "Left"))	move->s = 4.0;
    else if (0 == strcmp(ev->key, "Right"))	move->s = -4.0;
    else if (0 == strcmp(ev->key, "Up"))	move->z = -0.1;
    else if (0 == strcmp(ev->key, "Down"))	move->z = 0.1;
    else if (0 == strcmp(ev->key, "Prior"))	move->y = 0.1;	// Pg Up for humans.
    else if (0 == strcmp(ev->key, "Next"))	move->y = -0.1;	// Pg Dn for humans.
    else if (0 == strcmp(ev->key, "Home"))	move->x = -0.1;
    else if (0 == strcmp(ev->key, "End"))	move->x = 0.1;
    else if (0 == strcmp(ev->key, "space"))	move->jump = 1.0;
    else PW("Unexpected down keystroke - %s", ev->key);
  }
  else PE("Camera input not ready");
}

/* SL / OS camera controls
 *  up / down / w / s		moves avatar forward / backward
 *      shifted version does the same
 *      double tap triggers run mode / or fast fly mode
 *          Running backwards turns your avatar to suit, walking does not.
 *  left / right / a / d	rotates avatar left / right, strafes in mouselook
 *      shifted version turns the avatar to walk sideways, so not really a strafe.
 *      So not sure if the "strafe" in mouse look turns the avatar as well?
 *  PgDn / c			crouch while it is held down	move up in flight mode
 *  PgUp			jump				move down in flight mode
 *  Home			toggle flying
 *  End				Nothing?
 *  Esc				return to third person view
 *  m				toggle mouse look
 *  mouse wheel			move view closer / further away from current focused object or avatar
 *  Alt left click		focus on some other object
 *  Ins				???
 *  Del				???
 *  BS				???
 *  Tab				???
 *
 *  Mouse look is just first person view, moving mouse looks left / right / up / down.
 *      Not sure if the avatar rotates with left / right, but that's likely.
 *
 *  mouse moves With the left mouse button held down -
 *                              left / right		up / down
 *                              ---------------------------------
 *  for avatar			swings avatar around	zoom in and out of avatar
 *  for object			nothing
 *  alt				orbit left / right	zoom in and out
 *  alt ctrl			orbit left / right	orbit up / down
 *  alt shift			orbit left / right	zoom in and out
 *  alt ctrl shift		shift view left / right / up / down
 *  ctrl			Nothing?
 *  shift			Nothing?
 *  ctrl shift			Nothing?
 *
 *  Need to also consider when looking at a moving object / avatar.
 *
 *  I think there are other letter keys that duplicate arrow keys and such.  I'll look for them later, but I don't use them.
 *  No idea what the function keys are mapped to, but think it's various non camera stuff.
 *  I'm damn well leaving the Win/Command and Menu keys for the OS / window manager.  lol
 *  Keypad keys?  Not interested, I don't have them.
 *  Print Screen / SysRq, Pause / Break, other oddball keys, also not interested.
 *  NOTE - gonna have an easily programmable "bind key to command" thingy, like E17s, so that can deal with other keys.
 *      Should even let them be saveable so people can swap them with other people easily.
 *
 *  TODO - implement things like space mouse, sixaxis, phone as controller, joysticks, data gloves, etc.
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
 *          And now in Evas_3D, it's different again.
 */

static void _on_camera_input_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  cameraMove		*move	= data;
  Evas_Event_Key_Up	*ev	= event_info;

  if (move)
  {
    // TODO - Careful, gld->move MIGHT be read at the other end by another thread.  MIGHT, coz I really don't know at what point the camera animate routine is actually called.

    // Yes, we are dealing with the horrid Evas keyboard handling FUCKING STRING COMPARES!  Soooo ...
    // TODO - make this a hash lookup dammit.
    if (0 == strcmp(ev->key, "Escape"))
    {
    }
    else if (0 == strcmp(ev->key, "Left"))	move->s = 0.0;
    else if (0 == strcmp(ev->key, "Right"))	move->s = 0.0;
    else if (0 == strcmp(ev->key, "Up"))	move->z = 0.0;
    else if (0 == strcmp(ev->key, "Down"))	move->z = 0.0;
    else if (0 == strcmp(ev->key, "Prior"))	move->y = 0.0;
    else if (0 == strcmp(ev->key, "Next"))	move->y = 0.0;
    else if (0 == strcmp(ev->key, "Home"))	move->x = 0.0;
    else if (0 == strcmp(ev->key, "End"))	move->x = 0.0;
    else if (0 == strcmp(ev->key, "space"))	move->jump = 0.0;
    else PW("Unexpected up keystroke - %s", ev->key);
  }
  else PE("Camera input not ready");
}

/* While it's true that image is an Elm image, seems this Elm input event callback doesn't work.
// Elm style event callback.
static Eina_Bool _cb_event_GL(void *data, Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
  Eina_Bool processed = EINA_FALSE;

  switch (type)
  {
    case EVAS_CALLBACK_KEY_DOWN :
    {
      _on_camera_input_down(data, evas_object_evas_get(obj), obj, event_info);
      processed = EINA_TRUE;
      break;
    }

    case EVAS_CALLBACK_KEY_UP :
    {
      _on_camera_input_up(data, evas_object_evas_get(obj), obj, event_info);
      processed = EINA_TRUE;
      break;
    }

    default :
      PE("Unknown GL input event.");
  }

  return processed;
}
*/

Eo *cameraAdd(Evas *evas, Scene_Data *scene, Evas_Object *image)
{
  Eo   *result = NULL;
  Eo *camera;

  camera = eo_add(EVAS_CANVAS3D_CAMERA_CLASS, evas,
    evas_canvas3d_camera_projection_perspective_set(eoid, 90.0, 1.0, 1.0, 1024.0)
  );

  result = eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
    evas_canvas3d_node_constructor(eoid, EVAS_CANVAS3D_NODE_TYPE_CAMERA),
    evas_canvas3d_node_camera_set(eoid, camera),
    evas_canvas3d_node_position_set(eoid, 0.0, 4.0, 8.0),
//    evas_canvas3d_node_look_at_set(eoid, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 20.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0),
    evas_canvas3d_node_orientation_set(eoid, 0.0, 0.0, 0.0, 1.0)
  );
//  eo_unref(camera);

  evas_canvas3d_node_member_add(scene->root_node, result);
  evas_canvas3d_scene_camera_node_set(scene->scene, result);

  scene->move = calloc(1, sizeof(cameraMove));
  // In this code, we are making our own camera, so grab it's input when we are focused.
  evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_camera_input_down, scene->move);
  evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_UP,   _on_camera_input_up,   scene->move);
  // While it's true that image is an Elm image, seems this Elm input event callback doesn't work.
//  elm_object_event_callback_add(image, _cb_event_GL, NULL);

  return result;
}
