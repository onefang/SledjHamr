#include "extantz.h"


static void _animateCube(ExtantzStuffs *stuffs)
{
  static float angle = 0.0f;
  static int   frame = 0;
  static int   inc   = 1;
  Evas_3D_Mesh *m;

  eina_accessor_data_get(stuffs->aMesh, 0, (void **) &m);

  angle += 0.5;
  if (angle > 360.0)		angle -= 360.0f;

  frame += inc;
  if (frame >= 20) inc = -1;
  else if (frame <= 0) inc = 1;

  eo_do(stuffs->mesh_node,
    evas_3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, 1.0),
    evas_3d_node_mesh_frame_set(m, frame)
  );
}

static void _animateSphere(ExtantzStuffs *stuffs)
{
  static float earthAngle = 0.0f;

  earthAngle += 0.3;
  if (earthAngle > 360.0)	earthAngle -= 360.0f;
  eo_do(stuffs->mesh_node,
    evas_3d_node_orientation_angle_axis_set(earthAngle, 0.0, 1.0, 0.0)
  );
}

static void _animateSonic(ExtantzStuffs *stuffs)
{
  static int   sonicFrame = 0;
  Evas_3D_Mesh *m;

  eina_accessor_data_get(stuffs->aMesh, 0, (void **) &m);
  sonicFrame += 32;
  if (sonicFrame > 256 * 50)	sonicFrame = 0;
  eo_do(stuffs->mesh_node,
    evas_3d_node_mesh_frame_set(m, sonicFrame)
    );
}

void Evas_3D_Demo_add(globals *ourGlobals)
{
  char buf[PATH_MAX];
  ExtantzStuffs *eStuffs;

  ourGlobals->scene = scenriAdd(ourGlobals->win);

  // TODO - For now lets just pretend we got stuffs sent from our love.
  sprintf(buf, FAKE_UUID);
  eStuffs = addStuffs(buf, "onefang's test bed", 
    "Just a pretend bed with MLP scripts for testing SledjHamr.",
    "12345678-1234-4321-abcd-0123456789ab",
    "onefang%%27s%%20test%%20bed.omg",
    MT_CUBE,
    0.0, 4.0, 10.0,
    1.0, 0.0, 0.0, 0.0
  );
  addMaterial(eStuffs, -1, TT_NORMAL, "normal_lego.png");
  eStuffs->animateStuffs = (aniStuffs) _animateCube;
  stuffsSetup(eStuffs, ourGlobals, ourGlobals->scene, 1);

  sprintf(buf, FAKE_UUID);
  eStuffs = addStuffs(buf, "onefang's left testicle", 
    "Just a pretend world for testing SledjHamr.",
    "12345678-1234-4321-abcd-0123456789ab",
    "earth.omg",
    MT_SPHERE,
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0, 0.0
  );
  addMaterial(eStuffs, 1, TT_FACE, "EarthDiffuse.png");
  eStuffs->animateStuffs = (aniStuffs) _animateSphere;
  stuffsSetup(eStuffs, ourGlobals, ourGlobals->scene, 2);

  sprintf(buf, FAKE_UUID);
  eStuffs = addStuffs(buf, "Sonic the bed hog.", 
    "Just a pretend avatar for testing SledjHamr.",
    "12345678-1234-4321-abcd-0123456789ab",
    "sonic.md2",
    MT_MESH,
    0.0, 0.0, 0.0,
    -0.7071067811865475, 0.0, 0.0, 0.7071067811865475
  );
  addMaterial(eStuffs, -1, TT_FACE, "sonic.png");
  eStuffs->animateStuffs = (aniStuffs) _animateSonic;
  stuffsSetup(eStuffs, ourGlobals, ourGlobals->scene, 3);
}
