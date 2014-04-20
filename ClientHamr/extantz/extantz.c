#include "extantz.h"


int _log_domain = -1;

Eina_Hash *grids;
Eina_Hash *viewers;

static char *gridTest[][3] =
{
    {"3rd Rock Grid", "http://grid.3rdrockgrid.com:8002/", "http://grid.3rdrockgrid.com/3rg_login"},
    {"Infinite Grid", "http://grid.infinitegrid.org:8002/", "http://www.infinitegrid.org/loginscreen.php"},
    {"Second Life Grid", "https://login.agni.lindenlab.com/cgi-bin/login.cgi", "http://secondlife.com/"},
    {NULL, NULL, NULL}
};

static char *accountTest[][3] =
{
    {"3rd Rock Grid", "onefang rejected", "password"},
    {"Infinite Grid", "infinite onefang", "MyB1GSecrit"},
    {"Infinite Grid", "onefang rejected", "MySecrit"},
    {NULL, NULL, NULL}
};


static char *viewerTest[][3] =
{
    {"Imprudence",	"1.4.0 beta 3", ""},
    {"Kokua",		"3.4.4.25633", ""},
    {"meta-impy",	"1.4.0 beta 1.5", ""},
    {"SL",		"v3", ""},
    {NULL, NULL, NULL}
};


static Elm_Genlist_Item_Class *grid_gic = NULL;
static Elm_Genlist_Item_Class *account_gic = NULL;
static Elm_Genlist_Item_Class *viewer_gic = NULL;

//static const char *img1 = PACKAGE_DATA_DIR "/images/plant_01.jpg";
//static const char *img2 = PACKAGE_DATA_DIR "/images/sky_01.jpg";
static const char *img3 = PACKAGE_DATA_DIR "/images/rock_01.jpg";


#define EPHYSICS_TEST_THEME "extantz"


#if DO_GEARS
//--------------------------------//
// Gear Stuff.

static GLfloat *vert(GLfloat *p, GLfloat x, GLfloat y, GLfloat z, GLfloat *n)
{
   p[0] = x;
   p[1] = y;
   p[2] = z;
   p[3] = n[0];
   p[4] = n[1];
   p[5] = n[2];

   return p + 6;
}

/*  Draw a gear wheel.  You'll probably want to call this function when
 *  building a display list since we do a lot of trig here.
 *
 *  Input:  inner_radius - radius of hole at center
 *          outer_radius - radius at center of teeth
 *          width - width of gear
 *          teeth - number of teeth
 *          tooth_depth - depth of tooth
 */
static Gear *make_gear(GLData *gld, GLfloat inner_radius, GLfloat outer_radius, GLfloat width, GLint teeth, GLfloat tooth_depth)
{
   GLint i;
   GLfloat r0, r1, r2;
   GLfloat da;
   GLfloat *v;
   Gear *gear;
   double s[5], c[5];
   GLfloat normal[3];
   const int tris_per_tooth = 20;
   Evas_GL_API *gl = gld->glApi;

   gear = (Gear*)malloc(sizeof(Gear));
   if (gear == NULL)
     return NULL;

   r0 = inner_radius;
   r1 = outer_radius - tooth_depth / 2.0;
   r2 = outer_radius + tooth_depth / 2.0;

   da = 2.0 * M_PI / teeth / 4.0;

   gear->vertices = calloc(teeth * tris_per_tooth * 3 * 6, sizeof *gear->vertices);
   s[4] = 0;
   c[4] = 1;
   v = gear->vertices;
   for (i = 0; i < teeth; i++)
     {
        s[0] = s[4];
        c[0] = c[4];
        s[1] = sin(i * 2.0 * M_PI / teeth + da);
        c[1] = cos(i * 2.0 * M_PI / teeth + da);
        s[2] = sin(i * 2.0 * M_PI / teeth + da * 2);
        c[2] = cos(i * 2.0 * M_PI / teeth + da * 2);
        s[3] = sin(i * 2.0 * M_PI / teeth + da * 3);
        c[3] = cos(i * 2.0 * M_PI / teeth + da * 3);
        s[4] = sin(i * 2.0 * M_PI / teeth + da * 4);
        c[4] = cos(i * 2.0 * M_PI / teeth + da * 4);

        normal[0] = 0.0;
        normal[1] = 0.0;
        normal[2] = 1.0;

        v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);

        v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], width * 0.5, normal);
        v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], width * 0.5, normal);
        v = vert(v, r0 * c[4], r0 * s[4], width * 0.5, normal);

        v = vert(v, r0 * c[4], r0 * s[4], width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], width * 0.5, normal);
        v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], -width * 0.5, normal);

        normal[0] = 0.0;
        normal[1] = 0.0;
        normal[2] = -1.0;

        v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);

        v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], -width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], -width * 0.5, normal);
        v = vert(v, r1 * c[0], r1 * s[0], -width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], -width * 0.5, normal);
        v = vert(v, r2 * c[1], r2 * s[1], -width * 0.5, normal);

        v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);

        v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);
        v = vert(v, r1 * c[0], r1 * s[0], -width * 0.5, normal);
        v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);
        v = vert(v, r2 * c[1], r2 * s[1], -width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], -width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], -width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);

        v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);
     }

   gear->count = (v - gear->vertices) / 6;

   gl->glGenBuffers(1, &gear->vbo);
   gl->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);
   gl->glBufferData(GL_ARRAY_BUFFER, gear->count * 6 * 4, gear->vertices, GL_STATIC_DRAW);


   return gear;
}

static void free_gear(Gear *gear)
{
    free(gear->vertices);
    free(gear);
    gear = NULL;
}

static void multiply(GLfloat *m, const GLfloat *n)
{
   GLfloat tmp[16];
   const GLfloat *row, *column;
   div_t d;
   int i, j;

   for (i = 0; i < 16; i++)
     {
        tmp[i] = 0;
        d = div(i, 4);
        row = n + d.quot * 4;
        column = m + d.rem;
        for (j = 0; j < 4; j++)
          tmp[i] += row[j] * column[j * 4];
     }
   memcpy(m, &tmp, sizeof tmp);
}

static void rotate(GLfloat *m, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   double s, c;

   s = sin(angle);
   c = cos(angle);
   GLfloat r[16] =
     {
        x * x * (1 - c) + c,     y * x * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
        x * y * (1 - c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s, 0,
        x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,     0,
        0, 0, 0, 1
     };

   multiply(m, r);
}

static void translate(GLfloat *m, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat t[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  x, y, z, 1 };

   multiply(m, t);
}

static void draw_gear(GLData *gld, Gear *gear, GLfloat *m, GLfloat x, GLfloat y, GLfloat angle, const GLfloat *color)
{
   Evas_GL_API *gl = gld->glApi;
   GLfloat tmp[16];

   memcpy(tmp, m, sizeof tmp);
   translate(tmp, x, y, 0);
   rotate(tmp, 2 * M_PI * angle / 360.0, 0, 0, 1);
   gl->glUniformMatrix4fv(gld->proj_location, 1, GL_FALSE, tmp);
   gl->glUniform3fv(gld->light_location, 1, gld->light);
   gl->glUniform4fv(gld->color_location, 1, color);

   gl->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

   gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), NULL);
   gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLfloat *) 0 + 3);
   gl->glEnableVertexAttribArray(0);
   gl->glEnableVertexAttribArray(1);
   gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, gear->count);
}
#endif


static void gldata_init(GLData *gld)
{
    gld->useEGL = USE_EGL;
    gld->useIrr = USE_IRR;

    gld->view_rotx = -20.0;
    gld->view_roty = -30.0;
    gld->view_rotz = 0.0;
    gld->angle = 0.0;

    gld->light[0] = 1.0;
    gld->light[1] = 1.0;
    gld->light[2] = -5.0;
}

//-------------------------//


#if DO_GEARS
static const char vertex_shader[] =
   "uniform mat4 proj;\n"
   "attribute vec4 position;\n"
   "attribute vec4 normal;\n"
   "varying vec3 rotated_normal;\n"
   "varying vec3 rotated_position;\n"
   "vec4 tmp;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = proj * position;\n"
   "   rotated_position = gl_Position.xyz;\n"
   "   tmp = proj * normal;\n"
   "   rotated_normal = tmp.xyz;\n"
   "}\n";

 static const char fragment_shader[] =
   "#ifdef GL_ES\n"
   "precision mediump float;\n"
   "#endif\n"
   "uniform vec4 color;\n"
   "uniform vec3 light;\n"
   "varying vec3 rotated_normal;\n"
   "varying vec3 rotated_position;\n"
   "vec3 light_direction;\n"
   "vec4 white = vec4(0.5, 0.5, 0.5, 1.0);\n"
   "void main()\n"
   "{\n"
   "   light_direction = normalize(light - rotated_position);\n"
   "   gl_FragColor = color + white * dot(light_direction, rotated_normal);\n"
   "}\n";

static GLuint load_shader(GLData *gld, GLenum type, const char *shader_src)
{
   Evas_GL_API *gl = gld->glApi;
   GLuint shader;
   GLint compiled = 0;

   // Create the shader object
   if (!(shader = gl->glCreateShader(type))) return 0;
   gl->glShaderSource(shader, 1, &shader_src, NULL);
   // Compile the shader
   gl->glCompileShader(shader);
   gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

   if (!compiled)
     {
        GLint len = 0;

        gl->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 1)
          {
             char *info = malloc(sizeof(char) * len);

             if (info)
               {
                  gl->glGetShaderInfoLog(shader, len, NULL, info);
                  printf("Error compiling shader:\n"
                         "%s\n", info);
                  free(info);
               }
          }
        gl->glDeleteShader(shader);
        return 0;
     }
   return shader;
}

static void gears_init(GLData *gld)
{
    Evas_GL_API *gl = gld->glApi;
    GLint linked = 0;

//    char msg[512];

    gl->glEnable(GL_CULL_FACE);
    gl->glEnable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);

    // Load the vertex/fragment shaders
    gld->vtx_shader  = load_shader(gld, GL_VERTEX_SHADER, vertex_shader);
    gld->fgmt_shader = load_shader(gld, GL_FRAGMENT_SHADER, fragment_shader);

    // Create the program object
    if (!(gld->program = gl->glCreateProgram()))
	return;

    gl->glAttachShader(gld->program, gld->vtx_shader);
    gl->glAttachShader(gld->program, gld->fgmt_shader);

    // Bind shader attributes.
    gl->glBindAttribLocation(gld->program, 0, "position");
    gl->glBindAttribLocation(gld->program, 1, "normal");

    // Link the program
    gl->glLinkProgram(gld->program);
    gld->glApi->glGetProgramiv(gld->program, GL_LINK_STATUS, &linked);

    if (!linked)
    {
	GLint len = 0;

	gld->glApi->glGetProgramiv(gld->program, GL_INFO_LOG_LENGTH, &len);
	if (len > 1)
	{
	    char *info = malloc(sizeof(char) * len);

	    if (info)
	    {
		gld->glApi->glGetProgramInfoLog(gld->program, len, NULL, info);
		printf("Error linking program:\n%s\n", info);
		free(info);
	    }
	}
	gld->glApi->glDeleteProgram(gld->program);
    }

    gl->glUseProgram(gld->program);
    gld->proj_location  = gl->glGetUniformLocation(gld->program, "proj");
    gld->light_location = gl->glGetUniformLocation(gld->program, "light");
    gld->color_location = gl->glGetUniformLocation(gld->program, "color");

    /* make the gears */
    gld->gear1 = make_gear(gld, 1.0, 4.0, 1.0, 20, 0.7);
    gld->gear2 = make_gear(gld, 0.5, 2.0, 2.0, 10, 0.7);
    gld->gear3 = make_gear(gld, 1.3, 2.0, 0.5, 10, 0.7);

    gld->gearsInited = EINA_TRUE;
}
#endif

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

static void _resize_winwin(GLData *gld)
{
    Evas_Coord x, y, w, h;

    evas_object_geometry_get(gld->elmGl, &x, &y, &w, &h);
    evas_object_move(elm_win_inlined_image_object_get (gld->winwin), x, y);
    evas_object_resize(elm_win_inlined_image_object_get(gld->winwin), w, h);
}

// Called from on_pixels (), or the Elm_gliew resize callback.
static void _resize(GLData *gld)
{
   Evas_GL_API *gl = gld->glApi;

    _resize_winwin(gld);

#if DO_GEARS
   GLfloat ar, m[16] = {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.1, 0.0,
      0.0, 0.0, 0.0, 1.0
   };

   // GL Viewport stuff. you can avoid doing this if viewport is all the
   // same as last frame if you want
   if (gld->img_w < gld->img_h)
     ar = gld->img_w;
   else
     ar = gld->img_h;

   m[0] = 0.1 * ar / gld->img_w;
   m[5] = 0.1 * ar / gld->img_h;
   memcpy(gld->proj, m, sizeof gld->proj);
#endif

   gl->glViewport(0, 0, (GLint) gld->img_w, (GLint) gld->img_h);
}

static void _resize_gl(Evas_Object *obj)
{
   int w, h;
   GLData *gld = evas_object_data_get(obj, "gld");

   elm_glview_size_get(obj, &w, &h);

   gld->img_w = w;
   gld->img_h = h;
   _resize(gld);
}

static void on_pixels(void *data, Evas_Object *obj)
{
    GLData *gld = data;
    Evas_GL_API *gl = gld->glApi;

    // get the image size in case it changed with evas_object_image_size_set()
    if (gld->r1)
    {
	Evas_Coord    w, h;

	// Poor mans resize check. coz Elm wont do it easily.
	evas_object_image_size_get(gld->r1, &w, &h);
	if ((gld->img_w != w) || (gld->img_h != h))
	{
	    // No idea where this crap came from.
	    //float new_w = ((float) gld->scr_w / ((float) gld->scr_w * (float) w));
	    //float new_h = ((float) gld->scr_h / ((float) gld->scr_h * (float) h));
	
	    //gld->sfc_w = new_w;
	    //gld->sfc_h = new_h;
	    //evas_object_image_fill_set(gld->r1, 0, 0, gld->sfc_w, gld->sfc_h);
	    gld->img_w = w;
	    gld->img_h = h;
	    gld->resized = 1;
	}
    }

    if (gld->useEGL)
    {
	// Yes, we DO need to do our own make current, coz aparently the Irrlicht one is useless.
	// Hopefully Elm_GL has done this for us by now.
	// Evas_GL needs it to.
	if (gld->ctx)
	    evas_gl_make_current(gld->evasGl, gld->sfc, gld->ctx);
    }

    if (!gld->doneIrr)
	gld->doneIrr = startIrr(gld);	// Needs to be after gld->win is shown, and needs to be done in the render thread.
#if DO_GEARS
    if (!gld->gearsInited)
	gears_init(gld);
#endif

    if (gld->resized)
	_resize(gld);

    drawIrr_start(gld);

#if DO_GEARS
    if (gld->useEGL)
    {
	static const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
	static const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
	static const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };
	GLfloat m[16];

	// Draw the gears.
	if (!gld->useIrr)
	{
	    gl->glClearColor(0.7, 0.0, 1.0, 1.0);
	    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	memcpy(m, gld->proj, sizeof m);
	rotate(m, 2 * M_PI * gld->view_rotx / 360.0, 1, 0, 0);
	rotate(m, 2 * M_PI * gld->view_roty / 360.0, 0, 1, 0);
	rotate(m, 2 * M_PI * gld->view_rotz / 360.0, 0, 0, 1);

	draw_gear(gld, gld->gear1, m, -3.0, -2.0, gld->angle, red);
	draw_gear(gld, gld->gear2, m, 3.1, -2.0, -2 * gld->angle - 9.0, green);
	draw_gear(gld, gld->gear3, m, -3.1, 4.2, -2 * gld->angle - 25.0, blue);
	gld->angle += 2.0;
    }
#endif

    drawIrr_end(gld);

#if USE_IR
#else
   // This might get done deep within drawIrr_end, but only if we are using Irrlicht.

   // Optional - Flush the GL pipeline
   gl->glFlush();
//   gl->glFinish();
#endif

    gld->resized = 0;
}

static void _draw_gl(Evas_Object *obj)
{
//   Evas_GL_API *gl = elm_glview_gl_api_get(obj);
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld) return;

   on_pixels(gld, obj);
}

// Callback from Evas, also used as the general callback for deleting the GL stuff.
static void _clean_gl(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    GLData *gld = data;
    Evas_GL_API *gl = gld->glApi;

    ecore_animator_del(gld->animator);

    if (gld->useEGL)
    {
	// Do a make_current before deleting all the GL stuff.
	evas_gl_make_current(gld->evasGl, gld->sfc, gld->ctx);

    }

    gl->glDeleteShader(gld->vtx_shader);
    gl->glDeleteShader(gld->fgmt_shader);
    gl->glDeleteProgram(gld->program);

    if (gld->evasGl)
    {
	// Irrlicht wants to destroy the context and surface, so only do this if Irrlicht wont.
	if (!gld->doneIrr)
	{
	    evas_gl_surface_destroy(gld->evasGl, gld->sfc);
	    evas_gl_context_destroy(gld->evasGl, gld->ctx);
	}
	// TODO - hope this is OK, considering the context and surface might get dealt with by Irrlicht.
	// Might be better to teach Irrlicht to not destroy shit it did not create.
	evas_gl_config_free(gld->cfg);
	evas_gl_free(gld->evasGl);
    }

    // TODO - Since this is created on the render thread, better hope this is being deleted on the render thread.
    finishIrr(gld);

#if DO_GEARS
   gl->glDeleteBuffers(1, &gld->gear1->vbo);
   gl->glDeleteBuffers(1, &gld->gear2->vbo);
   gl->glDeleteBuffers(1, &gld->gear3->vbo);

   free_gear(gld->gear1);
   free_gear(gld->gear2);
   free_gear(gld->gear3);
#endif
}

// Callback from Elm, coz they do shit different.
static void _del_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld)
     {
        printf("Unable to get GLData. \n");
        return;
     }

    _clean_gl(gld, NULL, NULL, NULL);

   evas_object_data_del((Evas_Object*)obj, "gld");
}

// Callback for when the app quits.
static void _on_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    GLData *gld = data;

    evas_object_del(gld->win);
    free(gld);
    elm_exit();
}

// Callback from the animator.
static Eina_Bool doFrame(void *data)
{
    GLData *gld = data;

    // Mark the pixels as dirty, so they get rerendered each frame, then Irrlicht can draw it's stuff each frame.
    // This causes on_pixel to be triggered by Evas_GL, and _draw_gl for Elm_glview.
    if (gld->r1)
	evas_object_image_pixels_dirty_set(gld->r1, EINA_TRUE);
    if (gld->elmGl)
	elm_glview_changed_set(gld->elmGl);

    // If not using Evas_GL, we need to call on_pixel() manually.
    if (!gld->useEGL)
	on_pixels(gld, gld->r1);

    return EINA_TRUE;	// Keep calling us.
}

static void init_evas_gl(GLData *gld)
{
    if (!gld->useEGL)
	return;

    gld->sfc_w = gld->win_w;
    gld->sfc_h = gld->win_h;

    // Get the Evas / canvas from the elm window (that the Evas_Object "lives on"), which is itself an Evas_Object created by Elm, so not sure if it was created internally with Ecore_Evas.
    gld->canvas = evas_object_evas_get(gld->win);
    // An Ecore_Evas holds an Evas.
    // Get the Ecore_Evas that wraps an Evas.
    gld->ee = ecore_evas_ecore_evas_get(gld->canvas);	// Only use this on Evas that was created with Ecore_Evas.

#if USE_ELM_GL
    // Add a GLView
    gld->elmGl = elm_glview_add(gld->win);
    evas_object_size_hint_align_set(gld->elmGl, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(gld->elmGl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_glview_mode_set(gld->elmGl, 0 | ELM_GLVIEW_ALPHA | ELM_GLVIEW_DEPTH | ELM_GLVIEW_DIRECT);
    elm_glview_resize_policy_set(gld->elmGl, ELM_GLVIEW_RESIZE_POLICY_RECREATE);		// Destroy the current surface on a resize and create a new one.
    elm_glview_render_policy_set(gld->elmGl, ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);
//    elm_glview_render_policy_set(gld->elmGl, ELM_GLVIEW_RENDER_POLICY_ALWAYS);
    // These get called in the render thread I think.
    // None let me pass data, so this is why we are adding "gld" data to the object below.
    // Maybe we can use  elm_object_signal_callback_add or elm_object_item_signal_callback_add (edje signals)?
    //elm_glview_init_func_set(gld->elmGl, _init_gl);	// Not actually needed, it gets done in on_pixels.
    elm_glview_del_func_set(gld->elmGl, _del_gl);
    elm_glview_resize_func_set(gld->elmGl, _resize_gl);
    elm_glview_render_func_set(gld->elmGl, (Elm_GLView_Func_Cb) _draw_gl);

    // Not needed, the resize callback above deals with that.
    //elm_win_resize_object_add(gld->win, gld->elmGl);
    gld->glApi = elm_glview_gl_api_get(gld->elmGl);
    evas_object_data_set(gld->elmGl, "gld", gld);
    evas_object_show(gld->elmGl);
    elm_box_pack_end(gld->bx, gld->elmGl);
#else
    // get the evas gl handle for doing gl things
    gld->evasGl = evas_gl_new(gld->canvas);
    gld->glApi = evas_gl_api_get(gld->evasGl);

    // Set a surface config
    gld->cfg = evas_gl_config_new();
    gld->cfg->color_format = EVAS_GL_RGBA_8888;
    gld->cfg->depth_bits   = EVAS_GL_DEPTH_BIT_32;
    gld->cfg->stencil_bits = EVAS_GL_STENCIL_NONE;
    gld->cfg->options_bits = EVAS_GL_OPTIONS_DIRECT;

    // create a surface and context
    gld->sfc = evas_gl_surface_create(gld->evasGl, gld->cfg, gld->sfc_w, gld->sfc_h);
    gld->ctx = evas_gl_context_create(gld->evasGl, NULL);		// The second NULL is for sharing contexts I think, which might be what we want to do with Irrlicht.  It's not documented.

    // Set up the image object, a filled one by default.
    gld->r1 = evas_object_image_filled_add(gld->canvas);

   // attach important data we need to the object using key names. This just
   // avoids some global variables and means we can do nice cleanup. You can
   // avoid this if you are lazy
   // Not actually needed, with evas we can pass data pointers to stuff.
    //evas_object_data_set(gld->r1, "gld", gld);

    // when the object is deleted - call the on_del callback. like the above,
    // this is just being clean
    evas_object_event_callback_add(gld->r1, EVAS_CALLBACK_DEL, _clean_gl, gld);

    // set up an actual pixel size for the buffer data. it may be different
    // to the output size. any windowing system has something like this, just
    // evas has 2 sizes, a pixel size and the output object size
    evas_object_image_size_set(gld->r1, gld->sfc_w, gld->sfc_h);
    // Not actualy needed, as we create the image already filled.
    //evas_object_image_fill_set(gld->r1, 0, 0, gld->sfc_w, gld->sfc_h);

    // These two are not in the original example, but I get black r1 when I leave them out.
    evas_object_size_hint_align_set(gld->r1, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(gld->r1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    // set up the native surface info to use the context and surface created
    // above
    evas_gl_native_surface_get(gld->evasGl, gld->sfc, &(gld->ns));
    evas_object_image_native_surface_set(gld->r1, &(gld->ns));
    evas_object_image_pixels_get_callback_set(gld->r1, on_pixels, gld);

   // move the image object somewhere, resize it and show it. any windowing
   // system would need this kind of thing - place a child "window"
   // Hmm, no need to resize it anyway, it's sized above.
    evas_object_move(gld->r1, 0, 0);
    //evas_object_resize(gld->r1, gld->sfc_w, gld->sfc_h);
    elm_win_resize_object_add(gld->win, gld->r1);
    evas_object_show(gld->r1);
    elm_box_pack_end(gld->bx, gld->r1);

    evas_object_event_callback_add(gld->r1, EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_GL, gld);
//    evas_object_event_callback_add(gld->r1, EVAS_CALLBACK_KEY_DOWN, _on_camera_input_down, gld);
//    evas_object_event_callback_add(gld->r1, EVAS_CALLBACK_KEY_UP,   _on_camera_input_up, gld);
#endif

    // NOTE: if you delete r1, this animator will keep running trying to access
    // r1 so you'd better delete this animator with ecore_animator_del() or
    // structure how you do animation differently. you can also attach it like
    // evasGl, sfc, etc. etc. if this animator is specific to this object
    // only and delete it in the del handler for the obj.
    //
    // TODO - apparently the proper way to deal with the new async rendering is to have this animator do the dirty thing, and call the Irrlicht rendering stuff in the on_pixel call set above.
    //        That still leaves the problem of the Irrlicht setup being in the main thread.  Which also should be done in on_pixel, as that's done in the correct thread.
    
    // Jiggling this seems to produce a trade off between flickering and frame rate.  Nothing else changed the flickering.
    ecore_animator_frametime_set(0.04);	// Default is 1/30, or 0.033333
    gld->animator = ecore_animator_add(doFrame, gld);	// This animator will be called every frame tick, which defaults to 1/30 seconds.

    return;
}


//-------------------------//


static Evas_Object *_content_image_new(Evas_Object *parent, const char *img)
{
   Evas_Object *ic;

   ic = elm_icon_add(parent);
   elm_image_file_set(ic, img, NULL);
   return ic;
}

static void _promote(void *data, Evas_Object *obj , void *event_info )
{
   elm_naviframe_item_promote(data);
}

static char *_grid_label_get(void *data, Evas_Object *obj, const char *part)
{
    ezGrid *thisGrid = data;
    char buf[256];

    if (!strcmp(part, "elm.text"))
    {
	int count = eina_clist_count(&(thisGrid->accounts));

	if (0 == count)
	    snprintf(buf, sizeof(buf), "%s (no accounts)", thisGrid->name);
	else if (1 == count)
	    snprintf(buf, sizeof(buf), "%s (%d account)", thisGrid->name, count);
	else
	    snprintf(buf, sizeof(buf), "%s (%d accounts)", thisGrid->name, count);
    }
    else
	snprintf(buf, sizeof(buf), "%s", thisGrid->loginURI);
    return strdup(buf);
}

static Evas_Object *_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    ezGrid *thisGrid = data;
    Evas_Object *ic = elm_icon_add(obj);

    if (!strcmp(part, "elm.swallow.icon"))
	elm_icon_standard_set(ic, thisGrid->icon);

    evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
    return ic;
}

static char * _account_label_get(void *data, Evas_Object *obj, const char *part)
{
    ezAccount *thisAccount = data;
    char buf[256];

    buf[0] = '\0';
    if (!strcmp(part, "elm.text"))
	snprintf(buf, sizeof(buf), "%s", thisAccount->name);

    return strdup(buf);
}

static Evas_Object *_account_content_get(void *data, Evas_Object *obj, const char *part)
{
    ezAccount *thisAccount = data;
    Evas_Object *ic = elm_icon_add(obj);

    if (!strcmp(part, "elm.swallow.icon"))
	elm_icon_standard_set(ic, thisAccount->icon);

    evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
    return ic;
}

static char *_viewer_label_get(void *data, Evas_Object *obj, const char *part)
{
    ezViewer *thisViewer = data;
    char buf[256];

    if (!strcmp(part, "elm.text"))
	snprintf(buf, sizeof(buf), "%s", thisViewer->name);
    else
	snprintf(buf, sizeof(buf), "%s", thisViewer->version);
    return strdup(buf);
}

static Evas_Object *_viewer_content_get(void *data, Evas_Object *obj, const char *part)
{
    ezViewer *thisViewer = data;
    Evas_Object *ic = elm_icon_add(obj);

    if (!strcmp(part, "elm.swallow.icon"))
	elm_icon_standard_set(ic, thisViewer->icon);

    evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
    return ic;
}


static void _grid_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
    ezGrid *thisGrid = data;
    GLData *gld = thisGrid->gld;
    char buf[PATH_MAX];

//    sprintf(buf, "dillo -f -g '%dx%d+%d+%d' %s &", gld->win_w - (gld->win_w / 5), gld->win_h - 30, gld->win_w / 5, gld->win_y, thisGrid->splashPage);
    sprintf(buf, "uzbl -g '%dx%d+%d+%d' -u %s &", gld->win_w - (gld->win_w / 5), gld->win_h - 30, gld->win_w / 5, gld->win_y, thisGrid->splashPage);
    printf("%s   ### genlist obj [%p], item pointer [%p]\n", buf, obj, event_info);
// comment this out for now, busy dealing with input stuff, don't want to trigger this multiple times.
//    system(buf);
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
        snprintf(buf, sizeof(buf), "%s/images/pt.png", elm_app_data_dir_get());
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

static Evas_Object *_toolbar_menu_add(Evas_Object *win, Evas_Object *tb, char *label)
{
    Evas_Object *menu= NULL;
    Elm_Object_Item *tb_it;
//, *menu_it;

    tb_it = elm_toolbar_item_append(tb, NULL, label, NULL, NULL);
    elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
    // Priority is for when toolbar items are set to hide or menu when there are too many of them.  They get hidden or put on the menu based on priority.
    elm_toolbar_item_priority_set(tb_it, 9999);
    elm_toolbar_menu_parent_set(tb, win);
    menu = elm_toolbar_item_menu_get(tb_it);

    return menu;
}

static Evas_Object *fang_win_add(GLData *gld)
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

static void fang_win_complete(GLData *gld, Evas_Object *win, int x, int y, int w, int h)
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

static void overlay_add(GLData *gld)
{
    Evas_Object *bg;
//, *bx, *tb, *menu;
//    Elm_Object_Item *tb_it, *menu_it;

    // There many are reasons for this window.
    // The first is to cover the GL and provide something to click on to change focus.
    // The second is to provide something to click on for all the GL type clicking stuff that needs to be done.  In other words, no click through,we catch the clicks here.
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

static void chat_add(GLData *gld)
{
    Evas_Object *win, *bx, *en;

    win = fang_win_add(gld);

    bx = elm_box_add(win);
    elm_win_resize_object_add(win, bx);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

    en = elm_entry_add(win);
    elm_entry_scrollable_set(en, EINA_TRUE);
    evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_text_set(en, "History is shown here");
    elm_entry_editable_set(en, EINA_FALSE);
    evas_object_show(en);
    elm_box_pack_end(bx, en);

    en = elm_entry_add(win);
    elm_entry_scrollable_set(en, EINA_TRUE);
    evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_text_set(en, "");
    elm_entry_editable_set(en, EINA_TRUE);
    evas_object_show(en);
    elm_box_pack_end(bx, en);

    evas_object_show(bx);

    fang_win_complete(gld, win, 30, 500, gld->win_w / 3, gld->win_h / 3);
}


static void woMan_add(GLData *gld)
{
//    Evas_Object *win, *bg, *bx, *ic, *bb, *av, *en, *bt, *nf, *tab, *tb, *gridList, *viewerList, *menu;
    Evas_Object *win, *bx, *bt, *nf, *tab, *tb, *gridList, *viewerList, *menu;
    Elm_Object_Item *tb_it, *menu_it, *tab_it;
//    char buf[PATH_MAX];
    int i;

    win = fang_win_add(gld);

    bx = elm_box_add(win);
    elm_win_resize_object_add(win, bx);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

    // A tab thingy.
    tb = elm_toolbar_add(win);
    evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_SCROLL);

    // Menu.
    tb_it = elm_toolbar_item_append(tb, NULL, "Menu", NULL, NULL);
    elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
    // Priority is for when toolbar items are set to hide or menu when there are too many of them.  They get hidden or put on the menu based on priority.
    elm_toolbar_item_priority_set(tb_it, 9999);
    elm_toolbar_menu_parent_set(tb, win);
    menu = elm_toolbar_item_menu_get(tb_it);

    menu_it = elm_menu_item_add(menu, NULL, NULL, "edit", NULL, NULL);
    elm_menu_item_add(menu, menu_it, NULL, "preferences", NULL, NULL);
    menu_it = elm_menu_item_add(menu, NULL, NULL, "help", NULL, NULL);
    elm_menu_item_add(menu, menu_it, NULL, "about woMan", NULL, NULL);
    elm_menu_item_separator_add(menu, NULL);
    menu_it = elm_menu_item_add(menu, NULL, NULL, "advanced", NULL, NULL);
    elm_menu_item_add(menu, menu_it, NULL, "debug settings", NULL, NULL);

    // The toolbar needs to be packed into the box AFTER the menus are added.
    elm_box_pack_end(bx, tb);
    evas_object_show(tb);

    gridList = elm_genlist_add(win);
    grids = eina_hash_stringshared_new(free);

    grid_gic = elm_genlist_item_class_new();
    grid_gic->item_style = "double_label";
    grid_gic->func.text_get = _grid_label_get;
    grid_gic->func.content_get = _grid_content_get;
    grid_gic->func.state_get = NULL;
    grid_gic->func.del = NULL;
    for (i = 0; NULL != gridTest[i][0]; i++)
    {
	ezGrid *thisGrid = calloc(1, sizeof(ezGrid));
	
	if (thisGrid)
	{
	    eina_clist_init(&(thisGrid->accounts));
	    eina_clist_init(&(thisGrid->landmarks));
	    thisGrid->name		= gridTest[i][0];
	    thisGrid->loginURI		= gridTest[i][1];
	    thisGrid->splashPage 	= gridTest[i][2];
	    thisGrid->icon		= "folder";
	    thisGrid->gld		= gld;
	    thisGrid->item = elm_genlist_item_append(gridList, grid_gic, thisGrid, NULL, ELM_GENLIST_ITEM_TREE, _grid_sel_cb, thisGrid);
	    eina_hash_add(grids, thisGrid->name, thisGrid);
	}
    }

    account_gic = elm_genlist_item_class_new();
    account_gic->item_style = "default";
    account_gic->func.text_get = _account_label_get;
    account_gic->func.content_get = _account_content_get;
    account_gic->func.state_get = NULL;
    account_gic->func.del = NULL;
    for (i = 0; NULL != accountTest[i][0]; i++)
    {
	ezAccount *thisAccount = calloc(1, sizeof(ezAccount));
	ezGrid *grid = eina_hash_find(grids, accountTest[i][0]);
	
	if (thisAccount && grid)
	{
	    thisAccount->name		= accountTest[i][1];
	    thisAccount->password 	= accountTest[i][2];
	    thisAccount->icon		= "file";
	    elm_genlist_item_append(gridList, account_gic, thisAccount, grid->item, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	    eina_clist_add_tail(&(grid->accounts), &(thisAccount->grid));
	}
    }

    // Viewers stuff
    viewerList = elm_genlist_add(win);
    viewer_gic = elm_genlist_item_class_new();
    viewer_gic->item_style = "double_label";
    viewer_gic->func.text_get = _viewer_label_get;
    viewer_gic->func.content_get = _viewer_content_get;
    viewer_gic->func.state_get = NULL;
    viewer_gic->func.del = NULL;
    for (i = 0; NULL != viewerTest[i][0]; i++)
    {
	ezViewer *thisViewer = calloc(1, sizeof(ezViewer));
	
	if (thisViewer)
	{
	    thisViewer->name		= viewerTest[i][0];
	    thisViewer->version		= viewerTest[i][1];
	    thisViewer->path		= viewerTest[i][2];
	    thisViewer->icon		= "file";
	    thisViewer->item = elm_genlist_item_append(viewerList, viewer_gic, thisViewer, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}
    }

    // Toolbar pages
    nf = elm_naviframe_add(win);
    evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(nf, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(nf);
    tab = viewerList;				tab_it = elm_naviframe_item_push(nf, NULL, NULL, NULL, tab, NULL);	elm_naviframe_item_title_visible_set(tab_it, EINA_FALSE);	elm_toolbar_item_append(tb, NULL, "Viewers", _promote, tab_it);
    tab = _content_image_new(win, img3);	tab_it = elm_naviframe_item_push(nf, NULL, NULL, NULL, tab, NULL);	elm_naviframe_item_title_visible_set(tab_it, EINA_FALSE);	elm_toolbar_item_append(tb, NULL, "Landmarks", _promote, tab_it);
    tab = gridList;				tab_it = elm_naviframe_item_push(nf, NULL, NULL, NULL, tab, NULL);	elm_naviframe_item_title_visible_set(tab_it, EINA_FALSE);	elm_toolbar_item_append(tb, NULL, "Grids", _promote, tab_it);
    elm_box_pack_end(bx, nf);

#if USE_EO
    // Not ready for prime time yet, or I'm missing a step.  Causes it to hang after closing the window.
    // Slightly better now, it bitches instead of hanging.
    bt = eo_add(ELM_OBJ_BUTTON_CLASS, win);
    elm_object_text_set(bt, "Login");		// No eo interface for this that I can find.
    eo_do(bt, 
//		evas_obj_text_set("Login"),
		evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
		evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, 0.0),
		evas_obj_visibility_set(EINA_TRUE)
	);
#else
    bt = elm_button_add(win);
    elm_object_text_set(bt, "Login");
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
    evas_object_show(bt);
#endif
//    evas_object_smart_callback_add(bt, "clicked", NULL, NULL);
    elm_box_pack_end(bx, bt);
    evas_object_show(bx);

    fang_win_complete(gld, win, 30, 30, gld->win_w / 3, gld->win_h / 3);
}

EAPI_MAIN int elm_main(int argc, char **argv)
{
//    Evas_Object *bg, *menu, *bt, *tb;
    Evas_Object *menu, *tb;
    Elm_Object_Item *tb_it;
//, *menu_it;
    EPhysics_Body *boundary;
    EPhysics_World *world;
    EPhysics_Body *box_body1, *box_body2;
    Evas_Object *box1, *box2;
    GLData *gld = NULL;
//    char buf[PATH_MAX];
//    int i;
//    Eina_Bool gotWebKit = elm_need_web();	// Initialise ewebkit if it exists, or return EINA_FALSE if it don't.

    _log_domain = eina_log_domain_register("extantz", NULL);
    // Don't do this, we need to clean up other stuff do, so set a cleat up function below.
    //elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

    // If you want efl to handle finding your bin/lib/data dirs, you must do this below.
    elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
    elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
    elm_app_info_set(elm_main, "datadir", "images/sky_03.jpg");
    fprintf(stdout, "prefix was set to: %s\n", elm_app_prefix_dir_get());
    fprintf(stdout, "data directory is: %s\n", elm_app_data_dir_get());
    fprintf(stdout, "library directory is: %s\n", elm_app_lib_dir_get());
    fprintf(stdout, "locale directory is: %s\n", elm_app_locale_dir_get());

    // These are set via the elementary_config tool, which is hard to find.
    elm_config_finger_size_set(0);
    elm_config_scale_set(1.0);

    // alloc a data struct to hold our relevant gl info in
    if (!(gld = calloc(1, sizeof(GLData)))) return 1;
    gldata_init(gld);

    // Set the engine to opengl_x11, then open our window.
    if (gld->useEGL)
	elm_config_preferred_engine_set("opengl_x11");
    gld->win = elm_win_add(NULL, "extantz", ELM_WIN_BASIC);
    // Set preferred engine back to default from config
    elm_config_preferred_engine_set(NULL);

#if USE_PHYSICS
    if (!ephysics_init())
	return 1;
#endif

    elm_win_title_set(gld->win, "extantz virtual world manager");
    evas_object_smart_callback_add(gld->win, "delete,request", _on_done, gld);

    // Get the screen size.
    elm_win_screen_size_get(gld->win, &gld->win_x, &gld->win_y, &gld->scr_w, &gld->scr_h);
    gld->win_x = gld->win_x + (gld->scr_w / 3);
    gld->win_w = gld->scr_w / 2;
    gld->win_h = gld->scr_h - 30;

    // Note, we don't need an Elm_bg, the entire thing gets covered with the GL rendering surface anyway.
#if 0
    bg = elm_bg_add(gld->win);
    elm_bg_load_size_set(bg, gld->win_w, gld->win_h);
    elm_bg_option_set(bg, ELM_BG_OPTION_CENTER);
    snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", elm_app_data_dir_get());
    elm_bg_file_set(bg, buf, NULL);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(gld->win, bg);
    evas_object_show(bg);
#endif

    gld->bx = elm_box_add(gld->win);
    evas_object_size_hint_weight_set(gld->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(gld->bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_win_resize_object_add(gld->win, gld->bx);
    evas_object_show(gld->bx);

    overlay_add(gld);
    woMan_add(gld);
    chat_add(gld);

    // Gotta do this after adding the windows, otherwise the menu renders under the window.
    //   This sucks, gotta redefine this menu each time we create a new window?
    // Also, GL focus gets lost when any menu is used.  sigh

    // A toolbar thingy.
    tb = elm_toolbar_add(gld->win);
    evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_SCROLL);
    elm_toolbar_align_set(tb, 0.0);

    // Menus.
    menu = _toolbar_menu_add(gld->win, tb, "file");
    elm_menu_item_add(menu, NULL, NULL, "quit", _on_done, gld);

    menu = _toolbar_menu_add(gld->win, tb, "edit");
    elm_menu_item_add(menu, NULL, NULL, "preferences", NULL, NULL);

    menu = _toolbar_menu_add(gld->win, tb, "view");
    menu = _toolbar_menu_add(gld->win, tb, "world");
    menu = _toolbar_menu_add(gld->win, tb, "tools");

    menu = _toolbar_menu_add(gld->win, tb, "help");
    elm_menu_item_add(menu, NULL, NULL, "grid help", NULL, NULL);
    elm_menu_item_separator_add(menu, NULL);
    elm_menu_item_add(menu, NULL, NULL, "extantz blogs", NULL, NULL);
    elm_menu_item_add(menu, NULL, NULL, "extantz forum", NULL, NULL);
    elm_menu_item_separator_add(menu, NULL);
    elm_menu_item_add(menu, NULL, NULL, "about extantz", NULL, NULL);

    menu = _toolbar_menu_add(gld->win, tb, "advanced");
    elm_menu_item_add(menu, NULL, NULL, "debug settings", NULL, NULL);

    menu = _toolbar_menu_add(gld->win, tb, "god");

    // Other stuff in the toolbar.
    tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);
    elm_toolbar_item_separator_set(tb_it, EINA_TRUE);
    tb_it = elm_toolbar_item_append(tb, NULL, "restriction icons", NULL, NULL);
    tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);
    elm_toolbar_item_separator_set(tb_it, EINA_TRUE);
    tb_it = elm_toolbar_item_append(tb, NULL, "hop://localhost/Anarchadia 152, 155, 51 - Lost plot (Adult)", NULL, NULL);
    tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);
    elm_toolbar_item_separator_set(tb_it, EINA_TRUE);
    tb_it = elm_toolbar_item_append(tb, NULL, "date time:o'clock", NULL, NULL);

    // The toolbar needs to be packed into the box AFTER the menus are added.
    evas_object_show(tb);
    elm_box_pack_start(gld->bx, tb);

    // This does elm_box_pack_end(), so needs to be after the others.
    init_evas_gl(gld);

    evas_object_show(gld->bx);

#if USE_PHYSICS
    // ePhysics stuff.
    world = ephysics_world_new();
    ephysics_world_render_geometry_set(world, 0, 0, -50, gld->win_w, gld->win_h, 100);

    boundary = ephysics_body_bottom_boundary_add(world);
    ephysics_body_restitution_set(boundary, 1);
    ephysics_body_friction_set(boundary, 0);

    boundary = ephysics_body_top_boundary_add(world);
    ephysics_body_restitution_set(boundary, 1);
    ephysics_body_friction_set(boundary, 0);

    boundary = ephysics_body_left_boundary_add(world);
    ephysics_body_restitution_set(boundary, 1);
    ephysics_body_friction_set(boundary, 0);

    boundary = ephysics_body_right_boundary_add(world);
    ephysics_body_restitution_set(boundary, 1);
    ephysics_body_friction_set(boundary, 0);

    box1 = elm_image_add(gld->win);
    elm_image_file_set(box1, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "blue-cube");
    evas_object_move(box1, gld->win_w / 2 - 80, gld->win_h - 200);
    evas_object_resize(box1, 70, 70);
    evas_object_show(box1);

    box_body1 = ephysics_body_box_add(world);
    ephysics_body_evas_object_set(box_body1, box1, EINA_TRUE);
    ephysics_body_restitution_set(box_body1, 0.7);
    ephysics_body_friction_set(box_body1, 0);
    ephysics_body_linear_velocity_set(box_body1, -150, 200, 0);
    ephysics_body_angular_velocity_set(box_body1, 0, 0, 36);
    ephysics_body_sleeping_threshold_set(box_body1, 0.1, 0.1);

    box2 = elm_image_add(gld->win);
    elm_image_file_set(box2, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "purple-cube");
    evas_object_move(box2, gld->win_w / 2 + 10, gld->win_h - 200);
    evas_object_resize(box2, 70, 70);
    evas_object_show(box2);

    box_body2 = ephysics_body_box_add(world);
    ephysics_body_evas_object_set(box_body2, box2, EINA_TRUE);
    ephysics_body_restitution_set(box_body2, 0.7);
    ephysics_body_friction_set(box_body2, 0);
    ephysics_body_linear_velocity_set(box_body2, 80, -60, 0);
    ephysics_body_angular_velocity_set(box_body2, 0, 0, 360);
    ephysics_body_sleeping_threshold_set(box_body2, 0.1, 0.1);

    ephysics_world_gravity_set(world, 0, 0, 0);
#endif

    evas_object_move(gld->win, gld->win_x, gld->win_y);
    evas_object_resize(gld->win, gld->win_w, gld->win_h);
    evas_object_show(gld->win);

    _resize_winwin(gld);

    elm_run();

#if USE_PHYSICS
    ephysics_world_del(world);
    ephysics_shutdown();
#endif

    elm_shutdown();

    return 0;
}
ELM_MAIN()
