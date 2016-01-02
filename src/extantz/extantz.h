#ifndef _EXTANTZ_H_
#define _EXTANTZ_H_

#define USE_EVAS_3D 1
#define USE_IRR     0
#define USE_DEMO    1
#define DO_GEARS    0
#define USE_ELM_IMG 1


#include "LumbrJack.h"
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
  Evas             *evas;
  Evas_Object      *image;		// Our Elm image.
  Eo    *scene;
  Eo     *root_node;
  Eo     *camera_node;
  Eo     *light_node;

  Eo    *light;

  char             sim[PATH_MAX];
  Eina_Clist       stuffs, loading;

  cameraMove       *move;

  Evas_Object_Event_Cb clickCb;
  lua_State        *L;
} Scene_Data;

typedef void (* aniStuffs)(void *stuffs);

typedef enum
{
  ES_NORMAL = -1,
  ES_PRE = 1,
  ES_PART,
  ES_LOADED,
  ES_RENDERED,
  ES_TRASHED
} ES_Stages;

typedef struct _extantzStuffs
{
  Stuffs	stuffs;
  Scene_Data	*scene;
  Eo	*mesh_node;	// Multiple Evas_3D_Mesh's can be in one Evas_3D_Node
  // Can't use in arrays here, can't find the element sizes of incomplete types.
  Eina_Array	*mesh;		// Evas_3D_Mesh
  Eina_Array	*materials;	// Evas_3D_Material
  Eina_Array	*textures;	// Evas_3D_Texture
  Eina_Accessor *aMesh;
  Eina_Accessor *aMaterial;
  Eina_Accessor *aTexture;
  aniStuffs	animateStuffs;
  Eina_Clist	node;
  ES_Stages	stage;
  int		fake;
} ExtantzStuffs;


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
  int		logDom;		// Our logging domain.

  winFang	*mainWindow;
  int		scr_w, scr_h;	// The size of the screen.
  int		win_w, win_h;	// The size of the window.
  int		win_x, win_y;	// The position of the window.
  int		running : 1;

  Ecore_Animator  *animator;

  GLData gld;
  Scene_Data	*scene;

  EPhysics_World *world;

  winFang	*files;
  GuiLua	*purkle;
  GuiLua	*LSLGuiMess;

  Connection	*server;
  char		uuid[42];

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

Eina_Bool animateScene(globals *ourGlobals);

Scene_Data *scenriAdd(Evas_Object *win);
Eo *cameraAdd(Evas *evas, Scene_Data *scene, Evas_Object *win);
Eina_Bool animateCamera(Scene_Data *scene);
Eina_Bool animateScene(globals *ourGlobals);
void scenriDel(Scene_Data *scene);
void stuffsSetup(ExtantzStuffs *stuffs, Scene_Data *scene, int fake);
ExtantzStuffs *addStuffs(char *uuid, char *name, char *description, char *owner,
  char *file, MeshType type, double px, double py, double pz, double rx, double ry, double rz, double rw);
void addMaterial(ExtantzStuffs *e, int face, TextureType type, char *file);

winFang *filesAdd(globals *ourGlobals, char *path, Eina_Bool multi, Eina_Bool save);
void     filesShow(winFang *me, Evas_Smart_Cb func, void *data);
winFang *woMan_add(globals *ourGlobals);


#ifdef __cplusplus
}
#endif

#endif
