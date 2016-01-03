#include "extantz.h"



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

void free_gear(Gear *gear)
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


void drawGears(GLData *gld)
{
    Evas_GL_API *gl = gld->glApi;
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
                  PE("Error compiling shader: %s", info);
                  free(info);
               }
          }
        gl->glDeleteShader(shader);
        return 0;
     }
   return shader;
}

void gears_init(GLData *gld)
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
		PE("Error linking program: %s", info);
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
