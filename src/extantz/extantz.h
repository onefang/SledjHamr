#define USE_PHYSICS 1
#define USE_IRR     0
#define USE_DEMO    1
#define DO_GEARS    0


#include "SledjHamr.h"
#include "LumbrJack.h"
#include <elm_widget_glview.h>
#include <Evas_GL.h>
#include <EPhysics.h>
#include "extantzCamera.h"


#ifdef GL_GLES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#else
# include <GL/glext.h>
# include <GL/glx.h>
#endif


#ifdef __cplusplus
/*
In the Irrlicht Engine, everything can be found in the namespace 'irr'. So if
you want to use a class of the engine, you have to write irr:: before the name
of the class. For example to use the IrrlichtDevice write: irr::IrrlichtDevice.
To get rid of the irr:: in front of the name of every class, we tell the
compiler that we use that namespace from now on, and we will not have to write
irr:: anymore.
*/
using namespace irr;

/*
There are 5 sub namespaces in the Irrlicht Engine. Take a look at them, you can
read a detailed description of them in the documentation by clicking on the top
menu item 'Namespace List' or by using this link:
http://irrlicht.sourceforge.net/docu/namespaces.html
Like the irr namespace, we do not want these 5 sub namespaces now, to keep this
example simple. Hence, we tell the compiler again that we do not want always to
write their names.
*/
using namespace core;
using namespace scene;
using namespace video;

extern "C"{
#else

// Irrlicht stuff.  It's C++, so we gotta use incomplete types.
typedef struct IrrlichtDevice IrrlichtDevice;
typedef struct IVideoDriver IVideoDriver;
typedef struct ISceneManager ISceneManager;
typedef struct ICameraSceneNode ICameraSceneNode;

#endif



typedef struct _Gear Gear;
typedef struct _GLData GLData;

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
GLData	*gld;		// Just a temporary evil hack to pass gld to _grid_sel_cb().
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



struct _Gear
{
   GLfloat *vertices;
   GLuint vbo;
   int count;
};



typedef struct _Scene_Data
{
  Evas_Object      *image;		// Our Elm image.
  Evas_3D_Scene    *scene;
  Evas_3D_Node     *root_node;
  Evas_3D_Node     *camera_node;
  Evas_3D_Node     *light_node;

  Evas_3D_Camera   *camera;
  Evas_3D_Light    *light;

  Evas_3D_Mesh     *mesh;
  Evas_3D_Node     *mesh_node;
  Evas_3D_Material *material0;
  Evas_3D_Material *material1;
  Evas_3D_Texture  *texture0;
  Evas_3D_Texture  *texture1;
  Evas_3D_Texture  *texture_normal;

  Evas_3D_Mesh     *mesh2;
  Evas_3D_Node     *mesh2_node;
  Evas_3D_Material *material2;
  Evas_3D_Texture  *texture2;

  Evas_3D_Mesh     *mesh3;
  Evas_3D_Node     *mesh3_node;
  Evas_3D_Material *material3;
  Evas_3D_Texture  *texture_diffuse;

} Scene_Data;

// GL related data here.
struct _GLData
{
    Evas_Object	*win, *winwin;

    Ecore_Evas	*ee;
    Evas	*canvas;

    Evas_Object		*elmGl;
    Evas_GL_API		*glApi;

    GLuint	program;
    GLuint	vtx_shader;
    GLuint	fgmt_shader;
    int		scr_w, scr_h;	// The size of the screen.
    int		win_w, win_h;	// The size of the window.
    int		win_x, win_y;	// The position of the window.
    int		sfc_w, sfc_h;	// This is what Irrlicht is using, size of the GL image surface / glview.
    int		img_w, img_h;	// Size of the viewport.  DON'T reuse sfc_* here.  Despite the fact that sfc_* is only used in the init when Irricht is disabled?  WTF?
    int		useIrr : 1;
    int		doneIrr : 1;
    int		gearsInited : 1;
    int		resized : 1;

    Evas_Object	*bx;//, *r1;
    Ecore_Animator  *animator;

    IrrlichtDevice	*device;
    IVideoDriver	*driver;
    ISceneManager	*smgr;
    ICameraSceneNode	*camera;

    cameraMove		*move;

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

typedef struct _globals
{
  Evas		*evas;
  Evas_Object	*win;		// Our Elm window.
  Evas_Object	*tb;		// Our Elm toolbar.
  Eina_Clist	widgets;	// Our windows widgets.
  int		logDom;		// Our logging domain.

  struct _GLData gld;
  Scene_Data	*scene;
} globals;

extern globals ourGlobals;


void gears_init(GLData *gld);
void drawGears(GLData *gld);
void free_gear(Gear *gear);

EPhysics_World *ephysicsAdd(GLData *gld);

EAPI int startIrr(GLData *gld);
EAPI void drawIrr_start(GLData *gld);
EAPI void drawIrr_end(GLData *gld);
EAPI void finishIrr(GLData *gld);

EAPI void Evas_3D_Demo_add(globals *ourGlobals);
Eina_Bool _animate_scene(void *data);
void Evas_3D_Demo_fini(globals *ourGlobals);

void cameraAdd(Evas_Object *win, GLData *gld);

Evas_Object *fang_win_add(GLData *gld);
void fang_win_complete(GLData *gld, Evas_Object *win, int x, int y, int w, int h);
void overlay_add(GLData *gld);

void chat_add(GLData *gld);
void woMan_add(GLData *gld);


#ifdef __cplusplus
}
#endif

