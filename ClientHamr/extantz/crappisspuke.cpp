
#include <irrlicht.h>
#include "extantz.h"
#include "CDemo.h"


SExposedVideoData videoData;

IAnimatedMeshSceneNode *node;
CDemo *myDemo;
// This is the movement speed in units per second.
const f32 MOVEMENT_SPEED = 5.f;
// In order to do framerate independent movement, we have to know
// how long it was since the last frame
u32 then;

#ifdef  __cplusplus
extern "C" {
#endif

EAPI int startIrr(GLData *gld)
{
	SIrrlichtCreationParameters params;
	IrrlichtDevice	*device;
	IVideoDriver	*driver;
	ISceneManager	*smgr;
	bool additive = true;

	if (!gld->useIrr)
	    return 1;		// Return 1 so that the caller stops asking on each frame.

#if USE_IRR
	void *display = NULL;
	unsigned long sfc = 0;
	void *ctx = NULL;
//	Evas_GL_API *gl = gld->glApi;

#if USE_DEMO
	myDemo = new CDemo(gld, additive);
#endif

/* Raster says -
4. evas exposes an opengl-es2 api. any existing engine needs to be adapted to
use this. that's pretty much the end of that. if the engine doesn't have a
gles2 port.. it will need one. once it has one, then it is a simple matter of
replacing all the gl calls as follows:

glDrawArrays() -> api->glDrawArrays()
glBindBuffer() -> api->glBindBuffer()

you could make the port switchable with a macro:

#ifdef EVASGL
#define EG() my_evas_gl_api->
#else
#define EG()
#endif

then fix up all the gl calls to be

EG()glDrawArrays()
EG()glBindBuffer()

etc.

doing the above allows evas to decide how to share context. it may allocate a
separate context or share its own. either way as far as the evasgl api user is
concerned.. they get their own private context to play with. if it does NOT do
the above (use the api exposed by evas gl) then wrapping can't context switches
can't work. all gl calls HAVE to go through the wrapped api to work right. this
is because we can't REPLACE the internals of the gl driver which otherwise
would be managing context and state all internally and we have zero access to
that - especially with closed drivers. we'd end up writing a proxy gl library
which conflicts with real gl symbol-wise (thus taking over and replacing
normal gl calls) and i know i have no interest in maintaining a separate
libGLwhatever.so that is an exact copy of gl and it's api's just to wrap it
when we expose that wrapper without symbol complications via evas gl.

5. the engine will need to be adapted so the draw function is callable - eg by
elm_glview. then it's easy to switch where rendering happens. evas offers a fast
path to avoid buffer copies and make the gl view draw part of the evas
rendering path directly. this would offer almost zero overhead vs doing it
directly with egl/gles etc. to your backbuffer yourself, BUT gets you the bonus
of having your 3d view as part of a larger scenegraph. combine 2 or 3 of them
in a single window. overlay with evas objects or elm widgets for hud etc. all
for free. this also implies the engine has to integrate to the efl mainloop
etc. of course.
*/


	sfc     = ecore_evas_window_get(gld->ee);
	// This is the way Raster wants me to do things, but these functions are not actually available.  Pffft
//	ctx     = gl->glGetCurrentContext();
//	display = gl->glGetCurrentDisplay();
	ctx     = glXGetCurrentContext();
	display = glXGetCurrentDisplay();
	/* For using a pre existing X11 window (with optional OpenGL). */
	videoData = SExposedVideoData();
	videoData.OpenGLLinux.X11Display = display;	// void * - Connection to the X server.
	videoData.OpenGLLinux.X11Window  = sfc;		// unsigned long - Specifies a GLX drawable. Must be either an X window ID or a GLX pixmap ID.
	videoData.OpenGLLinux.X11Context = ctx;		// void * - Specifies a GLX rendering context that is to be attached to drawable.

	/*
	The most important function of the engine is the createDevice()
	function. The IrrlichtDevice is created by it, which is the root
	object for doing anything with the engine. createDevice() has 7
	parameters:

	- deviceType: Type of the device. This can currently be the Null-device,
	   one of the two software renderers, D3D8, D3D9, or OpenGL. In this
	   example we use EDT_SOFTWARE, but to try out, you might want to
	   change it to EDT_BURNINGSVIDEO, EDT_NULL, EDT_DIRECT3D8,
	   EDT_DIRECT3D9, or EDT_OPENGL.

	- windowSize: Size of the Window or screen in FullScreenMode to be
	   created. In this example we use 640x480.

	- bits: Amount of color bits per pixel. This should be 16 or 32. The
	   parameter is often ignored when running in windowed mode.

	- fullscreen: Specifies if we want the device to run in fullscreen mode
	   or not.

	- stencilbuffer: Specifies if we want to use the stencil buffer (for
	   drawing shadows).

	- vsync: Specifies if we want to have vsync enabled, this is only useful
	   in fullscreen mode.

	- eventReceiver: An object to receive events. We do not want to use this
	   parameter here, and set it to 0.

	Always check the return value to cope with unsupported drivers,
	dimensions, etc.
	*/

	params.DeviceType = EIDT_X11;	// EIDT_BEST might be preferable.
	if (ctx)
		params.DriverType = video::EDT_OPENGL;
	else
		params.DriverType = video::EDT_BURNINGSVIDEO;
	params.WindowSize = dimension2d<u32>(gld->sfc_w, gld->sfc_h);
	params.Bits = 32;		// Ignored in windowed mode?
	params.ZBufferBits = 16;	// Default 16.
	params.Fullscreen = false;	// The default anyway.
	params.Stencilbuffer = false;	// For shadows.
	params.Vsync = false;
	params.AntiAlias=true;
	params.WithAlphaChannel = true;
	params.IgnoreInput = true;
	params.EventReceiver = myDemo;	// Probably useless, EFL might not let Irrlicht grab the input.
	params.WindowId = (void *) videoData.OpenGLLinux.X11Window;
	params.VideoData = &videoData;

	device = createDeviceEx(params);

	if (!device)
		return 0;
	gld->device = device;

	/*
	Get a pointer to the VideoDriver and the SceneManager so that we do not always have to write
	device->getVideoDriver() or device->getSceneManager().
	*/
	driver = device->getVideoDriver();	gld->driver = driver;
	smgr = device->getSceneManager();	gld->smgr   = smgr;

	// FIXME - this is what makes the window vanish in EFL 1.8, but worked fine in 1.7 I think.
//	device->setResizable(true);
	driver->OnResize(dimension2d<u32>(gld->img_w, gld->img_h));
	// Just gives me a blank screen.  grrrr
//	driver->setViewPort(rect<s32>(0, 0, gld->img_w, gld->img_h));

	// set ambient light
	smgr->setAmbientLight (video::SColorf(0x00c0c0c0));

#if USE_DEMO
	myDemo->setup(gld);
#else
	/*
	To show something interesting, we load a Quake 2 model and display it.
	We only have to get the Mesh from the Scene Manager with getMesh() and add
	a SceneNode to display the mesh with addAnimatedMeshSceneNode(). We
	check the return value of getMesh() to become aware of loading problems
	and other errors.

	Instead of writing the filename sydney.md2, it would also be possible
	to load a Maya object file (.obj), a complete Quake3 map (.bsp) or any
	other supported file format. By the way, that cool Quake 2 model
	called sydney was modelled by Brian Collins.
	*/
	IAnimatedMesh* mesh = smgr->getMesh("media/Irrlicht/sydney.md2");
	if (!mesh)
	{
		device->drop();
		return 0;
	}
	node = smgr->addAnimatedMeshSceneNode(mesh);

	/*
	To let the mesh look a little bit nicer, we change its material. We
	disable lighting because we do not have a dynamic light in here, and
	the mesh would be totally black otherwise. Then we set the frame loop,
	such that the predefined STAND animation is used. And last, we apply a
	texture to the mesh. Without it the mesh would be drawn using only a
	color.
	*/
	if (node)
	{
//		node->setMaterialFlag(EMF_LIGHTING, false);
		node->setMD2Animation(scene::EMAT_STAND);
		node->setMaterialTexture(0, driver->getTexture("media/Irrlicht/sydney.bmp"));
	}

	/*
	To look at the mesh, we place a camera into 3d space at the position
	(0, 30, -40). The camera looks from there to (0,5,0), which is
	approximately the place where our md2 model is.
	*/
	smgr->addCameraSceneNode(0, vector3df(50, 70, -65), vector3df(0, 50, 0));
#endif

	then = device->getTimer()->getTime();
#endif
    return 1;
}

EAPI void drawIrr_start(GLData *gld)
{
    if (gld->useIrr)
    {
	IrrlichtDevice	*device = gld->device;
	IVideoDriver	*driver = gld->driver;
	ISceneManager	*smgr   = gld->smgr;

	// Increase virtual timer time, instead of device->run() if doing our own input processing.
	device->getTimer()->tick();

	// Work out a frame delta time.
	const u32 now = device->getTimer()->getTime();
//	const f32 frameDeltaTime = (f32)(now - then) / 1000.f; // Time in seconds
	then = now;


#if USE_DEMO
	myDemo->preDraw(gld, now);
#else
	core::vector3df nodePosition = node->getPosition();
//	nodePosition.Y -= MOVEMENT_SPEED * frameDeltaTime;
	node->setPosition(nodePosition);
#endif

	/*
	Anything can be drawn between a beginScene() and an endScene()
	call. The beginScene() call clears the screen with a color and
	the depth buffer, if desired. Then we let the Scene Manager and
	the GUI Environment draw their content. With the endScene()
	call everything is presented on the screen.
	*/
	driver->beginScene(true, true, SColor(255, 255, 255, 255), videoData, NULL);	// This does the context change, then clearBuffers()

	smgr->drawAll();
    }
}

EAPI void drawIrr_end(GLData *gld)
{
    IVideoDriver	*driver = gld->driver;

    if (gld->useIrr)
	driver->endScene();
}

EAPI void finishIrr(GLData *gld)
{
    IrrlichtDevice	*device = gld->device;

    /*
    After we are done with the render loop, we have to delete the Irrlicht
    Device created before with createDevice(). In the Irrlicht Engine, you
    have to delete all objects you created with a method or function which
    starts with 'create'. The object is simply deleted by calling ->drop().
    See the documentation at irr::IReferenceCounted::drop() for more
    information.
    */
    if (gld->useIrr)
	device->drop();
}


#ifdef  __cplusplus
}
#endif

