#include "extantz.h"


#define EPHYSICS_TEST_THEME "extantz"

EPhysics_World *ephysicsAdd(globals *ourGlobals)
{
  EPhysics_Body *boundary;
  EPhysics_World *world;
  EPhysics_Body *box_body1, *box_body2;
  Evas_Object *box1, *box2;
  char buf[PATH_MAX];

  // ePhysics stuff.
  world = ephysics_world_new();
  ephysics_world_render_geometry_set(world, 0, 0, -50, ourGlobals->win_w, ourGlobals->win_h, 100);

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

  sprintf(buf, "%s/%s.edj", prefix_data_get(), EPHYSICS_TEST_THEME);
  box1 = eo_add(ELM_IMAGE_CLASS, ourGlobals->win,
    efl_file_set(buf, "blue-cube"),
    efl_gfx_size_set(70, 70),
    efl_gfx_position_set(ourGlobals->win_w / 2 - 80, ourGlobals->win_h - 200),
    efl_gfx_visible_set(EINA_TRUE)
  );

  box_body1 = ephysics_body_box_add(world);
  ephysics_body_evas_object_set(box_body1, box1, EINA_TRUE);
  ephysics_body_restitution_set(box_body1, 0.7);
  ephysics_body_friction_set(box_body1, 0);
  ephysics_body_linear_velocity_set(box_body1, -150, 20, 0);
  ephysics_body_angular_velocity_set(box_body1, 0, 0, 36);
  ephysics_body_sleeping_threshold_set(box_body1, 0.1, 0.1);
//  eo_unref(box1);

  sprintf(buf, "%s/%s.edj", prefix_data_get(), EPHYSICS_TEST_THEME);
  box2 = eo_add(ELM_IMAGE_CLASS, ourGlobals->win,
    efl_file_set(buf, "purple-cube"),
    efl_gfx_size_set(70, 70),
    efl_gfx_position_set(ourGlobals->win_w / 2 + 10, ourGlobals->win_h - 200),
    efl_gfx_visible_set(EINA_TRUE)
  );

  box_body2 = ephysics_body_box_add(world);
  ephysics_body_evas_object_set(box_body2, box2, EINA_TRUE);
  ephysics_body_restitution_set(box_body2, 0.7);
  ephysics_body_friction_set(box_body2, 0);
  ephysics_body_linear_velocity_set(box_body2, 800, -600, 0);
  ephysics_body_angular_velocity_set(box_body2, 0, 0, 360);
  ephysics_body_sleeping_threshold_set(box_body2, 0.1, 0.1);
//  eo_unref(box2);

  ephysics_world_gravity_set(world, 0, 0, 0);

  return world;
}
