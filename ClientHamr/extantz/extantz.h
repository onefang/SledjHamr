#include <Elementary.h>
#include <elm_widget_glview.h>
#include <Evas_GL.h>
#include <EPhysics.h>


#ifdef GL_GLES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#else
# include <GL/glext.h>
# include <GL/glx.h>
#endif


#ifdef __cplusplus
extern "C"{
#else

// Irrlicht stuff.  It's C++, so we gotta use incomplete types.
typedef struct IrrlichtDevice IrrlichtDevice;
typedef struct IVideoDriver IVideoDriver;
typedef struct ISceneManager ISceneManager;

#endif


#define USE_PHYSICS 1
#define USE_EGL     1
#define USE_IRR     1


typedef enum
{
    EZP_NONE,
    EZP_AURORA,
    EZP_OPENSIM,
    EZP_SECOND_LIFE,
    EZP_SLEDJHAMR,
    EZP_TRITIUM
} ezPlatform;

typedef struct
{
    char	*name;
    char	*version;	// Version string.
    char	*path;		// OS filesystem path to the viewer install.
    char	*icon;
    uint16_t	tag;		// The UUID of the texture used in the avatar bake hack.
    uint8_t	r, g, b;	// Colour used for the in world tag.
    Elm_Object_Item *item;
} ezViewer;

typedef struct
{
    Eina_Clist  accounts;
    Eina_Clist  landmarks;
    char	*name;
    char	*loginURI;
    char	*splashPage;
    char	*helperURI;
    char	*website;
    char	*supportPage;
    char	*registerPage;
    char	*passwordPage;
    char	*icon;
    ezPlatform	platform;
    ezViewer	*viewer;
    Elm_Object_Item *item;
} ezGrid;

typedef struct
{
    Eina_Clist	 grid;
    char	*name;
    char	*password;	// Think we need to pass unencrypted passwords to the viewer.  B-(
    char	*icon;
    ezViewer	*viewer;
} ezAccount;

typedef struct
{
    Eina_Clist	 grid;
    char	*name;
    char	*sim;
    char	*screenshot;
    short	x, y, z;
} ezLandmark;


typedef struct _Gear Gear;
typedef struct _GLData GLData;


struct _Gear
{
   GLfloat *vertices;
   GLuint vbo;
   int count;
};

// GL related data here.
struct _GLData
{
    Evas_Object	*win;

    Evas_GL_Context	*ctx;
    Evas_GL_Surface	*sfc;
    Evas_GL_Config 	*cfg;
    Evas_GL		*evasgl;
    Evas_GL_API		*glapi;

    GLuint	program;
    GLuint	vtx_shader;
    GLuint	fgmt_shader;
    int		sfc_w, sfc_h;
    int		useEGL : 1;
    int		useIrr : 1;
    int		doneIrr : 1;

    Evas_Object	*bx, *r1;
    Ecore_Animator  *animator;

    IrrlichtDevice	*device;	// IrrlichtDevice
    IVideoDriver	*driver;	// IVideoDriver
    ISceneManager	*smgr;		// ISceneManager

   // Gear Stuff
   GLfloat      view_rotx;
   GLfloat      view_roty;
   GLfloat      view_rotz;

   Gear        *gear1;
   Gear        *gear2;
   Gear        *gear3;

   GLfloat      angle;

   GLuint       proj_location;
   GLuint       light_location;
   GLuint       color_location;

   GLfloat      proj[16];
   GLfloat      light[3];
};


EAPI int startIrr(GLData *gld);
EAPI void drawIrr_start(GLData *gld);
EAPI void drawIrr_end(GLData *gld);
EAPI void finishIrr(GLData *gld);

#ifdef __cplusplus
}
#endif

