#ifndef _EXTANTZ_H_
#define _EXTANTZ_H_

#define USE_PHYSICS 1
#define USE_IRR     0
#define USE_DEMO    0
#define DO_GEARS    0


#include "winFang.h"
#include "GuiLua.h"
#include "scenri.h"

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


#if DO_GEARS
typedef struct _Gear
{
   GLfloat *vertices;
   GLuint vbo;
   int count;
} Gear;
#endif

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



typedef struct _Scene_Data
{
  Evas_Object      *image;		// Our Elm image.
  Evas_3D_Scene    *scene;
  Evas_3D_Node     *root_node;
  Evas_3D_Node     *camera_node;
  Evas_3D_Node     *light_node;

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

// Elm GL view related data here.
typedef struct _GLData
{
    Evas_Object	*winwin;
    Evas_Object	*elmGl;
    Evas_GL_API	*glApi;

    int		sfc_w, sfc_h;	// This is what Irrlicht is using, size of the GL image surface / glview.
    int		img_w, img_h;	// Size of the viewport.  DON'T reuse sfc_* here.  Despite the fact that sfc_* is only used in the init when Irricht is disabled?  WTF?
    int		useIrr : 1;
    int		doneIrr : 1;
    int		gearsInited : 1;
    int		resized : 1;

    IrrlichtDevice	*device;
    IVideoDriver	*driver;
    ISceneManager	*smgr;
    ICameraSceneNode	*camera;

    cameraMove		*move;

#if DO_GEARS
    GLuint	program;
    GLuint	vtx_shader;
    GLuint	fgmt_shader;

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
#endif
} GLData;

typedef struct _globals
{
#if USE_IRR
  Ecore_Evas	*ee;
#endif
  Evas	*evas;
  Evas_Object	*win;		// Our Elm window.
  Evas_Object	*tb;		// Our Elm toolbar.
  Evas_Object	*bx;		// Our box.
  int		logDom;		// Our logging domain.

  int		scr_w, scr_h;	// The size of the screen.
  int		win_w, win_h;	// The size of the window.
  int		win_x, win_y;	// The position of the window.

  Ecore_Animator  *animator;

  GLData gld;
  Scene_Data	*scene;

  Eina_Clist	winFangs;

  winFang	*files;
} globals;

extern globals ourGlobals;


#if DO_GEARS
void gears_init(GLData *gld);
void drawGears(GLData *gld);
void free_gear(Gear *gear);
#endif

EPhysics_World *ephysicsAdd(globals *ourGlobals);

#if USE_IRR
EAPI int startIrr(globals *ourGlobals);
EAPI void drawIrr_start(globals *ourGlobals);
EAPI void drawIrr_end(globals *ourGlobals);
EAPI void finishIrr(globals *ourGlobals);
#endif

void overlay_add(globals *ourGlobals);

EAPI void Evas_3D_Demo_add(globals *ourGlobals);
Eina_Bool _animate_scene(globals *ourGlobals);
void Evas_3D_Demo_fini(globals *ourGlobals);

Scene_Data *scenriAdd(globals *ourGlobals);
Evas_3D_Node *cameraAdd(globals *ourGlobals, Scene_Data *scene, Evas_Object *win);
Eina_Bool animateCamera(globals *ourGlobals);

winFang *chat_add(globals *ourGlobals);
winFang *filesAdd(globals *ourGlobals, char *path, Eina_Bool multi, Eina_Bool save);
void     filesShow(winFang *me, Evas_Smart_Cb func, void *data);
winFang *woMan_add(globals *ourGlobals);


#ifdef __cplusplus
}
#endif

#endif
