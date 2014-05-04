#include "extantz.h"


static Scene_Data ourScene;


static const float cube_vertices[] =
{
   /* Front */
   -1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,

   /* Back */
    1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  1.0,
   -1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Left */
   -1.0,  1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  1.0,
   -1.0,  1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Right */
    1.0,  1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Top */
   -1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,
   -1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
    1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Bottom */
    1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  1.0,
   -1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  0.0,
};

static const unsigned short cube_indices[] =
{
   /* Front */
   0,   1,  2,  2,  1,  3,

   /* Back */
   4,   5,  6,  6,  5,  7,

   /* Left */
   8,   9, 10, 10,  9, 11,

   /* Right */
   12, 13, 14, 14, 13, 15,

   /* Top */
   16, 17, 18, 18, 17, 19,

   /* Bottom */
   20, 21, 22, 22, 21, 23
};

static const unsigned int pixels0[] =
{
   0xff0000ff, 0xff0000ff, 0xffff0000, 0xffff0000,
   0xff0000ff, 0xff0000ff, 0xffff0000, 0xffff0000,
   0xff00ff00, 0xff00ff00, 0xff000000, 0xff000000,
   0xff00ff00, 0xff00ff00, 0xff000000, 0xff000000,
};

static const unsigned int pixels1[] =
{
   0xffff0000, 0xffff0000, 0xff00ff00, 0xff00ff00,
   0xffff0000, 0xffff0000, 0xff00ff00, 0xff00ff00,
   0xff0000ff, 0xff0000ff, 0xffffffff, 0xffffffff,
   0xff0000ff, 0xff0000ff, 0xffffffff, 0xffffffff,
};


typedef struct _vec4
{
    float   x;
    float   y;
    float   z;
    float   w;
} vec4;

typedef struct _vec3
{
    float   x;
    float   y;
    float   z;
} vec3;

typedef struct _vec2
{
    float   x;
    float   y;
} vec2;

typedef struct _vertex
{
    vec3    position;
    vec3    normal;
    vec3    tangent;
    vec4    color;
    vec3    texcoord;
} vertex;

static int		vertex_count = 0;
static vertex	       *sphere_vertices = NULL;

static int		index_count = 0;
static unsigned short  *sphere_indices = NULL;

static inline vec3
_normalize(const vec3 *v)
{
    double  l = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    vec3    vec;

    vec.x = v->x / l;
    vec.y = v->y / l;
    vec.z = v->z / l;

    return vec;
}


static void
_sphere_init(int precision)
{
    int		    i, j;
    unsigned short *index;

    vertex_count = (precision + 1) * (precision + 1);
    index_count = precision * precision * 6;

    /* Allocate buffer. */
    sphere_vertices = malloc(sizeof(vertex) * vertex_count);
    sphere_indices = malloc(sizeof(unsigned short) * index_count);

    for (i = 0; i <= precision; i++)
    {
	double lati = (M_PI * (double)i) / (double)precision;
	double y = cos(lati);
	double r = fabs(sin(lati));

	for (j = 0; j <= precision; j++)
	{
	    double longi = (M_PI * 2.0 * j) / precision;
	    vertex *v = &sphere_vertices[i * (precision  + 1) + j];

	    if (j == 0 || j == precision)
		v->position.x = 0.0;
	    else
		v->position.x = r * sin(longi);

	    v->position.y = y;

	    if (j == 0 || j == precision)
		v->position.z = r;
	    else
		v->position.z = r * cos(longi);

	    v->normal = v->position;

	    if (v->position.x > 0.0)
	    {
		v->tangent.x = -v->normal.y;
		v->tangent.y =  v->normal.x;
		v->tangent.z =  v->normal.z;
	    }
	    else
	    {
		v->tangent.x =  v->normal.y;
		v->tangent.y = -v->normal.x;
		v->tangent.z =  v->normal.z;
	    }

	    v->color.x = v->position.x;
	    v->color.y = v->position.y;
	    v->color.z = v->position.z;
	    v->color.w = 1.0;

	    if (j == precision)
		v->texcoord.x = 1.0;
	    else if (j == 0)
		v->texcoord.x = 0.0;
	    else
		v->texcoord.x = (double)j / (double)precision;

	    if (i == precision)
		v->texcoord.y = 1.0;
	    else if (i == 0)
		v->texcoord.y = 0.0;
	    else
		v->texcoord.y = 1.0 - (double)i / (double)precision;
	}
    }

    index = &sphere_indices[0];

    for (i = 0; i < precision; i++)
    {
	for (j = 0; j < precision; j++)
	{
	    *index++ = i * (precision + 1) + j;
	    *index++ = i * (precision + 1) + j + 1;
	    *index++ = (i + 1) * (precision + 1) + j;

	    *index++ = (i + 1) * (precision + 1) + j;
	    *index++ = i * (precision + 1) + j + 1;
	    *index++ = (i + 1) * (precision + 1) + j + 1;
	}
    }

    for (i = 0; i < index_count; i += 3)
    {
	vertex *v0 = &sphere_vertices[sphere_indices[i + 0]];
	vertex *v1 = &sphere_vertices[sphere_indices[i + 1]];
	vertex *v2 = &sphere_vertices[sphere_indices[i + 2]];

	vec3	e1, e2;
	float	du1, du2, dv1, dv2, f;
	vec3	tangent;

	e1.x = v1->position.x - v0->position.x;
	e1.y = v1->position.y - v0->position.y;
	e1.z = v1->position.z - v0->position.z;

	e2.x = v2->position.x - v0->position.x;
	e2.y = v2->position.y - v0->position.y;
	e2.z = v2->position.z - v0->position.z;

	du1 = v1->texcoord.x - v0->texcoord.x;
	dv1 = v1->texcoord.y - v0->texcoord.y;

	du2 = v2->texcoord.x - v0->texcoord.x;
	dv2 = v2->texcoord.y - v0->texcoord.y;

	f = 1.0 / (du1 * dv2 - du2 * dv1);

	tangent.x = f * (dv2 * e1.x - dv1 * e2.x);
	tangent.y = f * (dv2 * e1.y - dv1 * e2.y);
	tangent.z = f * (dv2 * e1.z - dv1 * e2.z);

	v0->tangent = tangent;
    }

    for (i = 0; i <= precision; i++)
    {
	for (j = 0; j <= precision; j++)
	{
	    if (j == precision)
	    {
		vertex *v = &sphere_vertices[i * (precision  + 1) + j];
		v->tangent = sphere_vertices[i * (precision + 1)].tangent;
	    }
	}
    }
}


Eina_Bool _animate_scene(globals *ourGlobals)
{
  static float angle = 0.0f;
  static float earthAngle = 0.0f;
  static int   frame = 0;
  static int   inc   = 1;
  static int   sonicFrame = 0;
  Evas_Real x, y, z, w;
  EPhysics_Quaternion *quat   = ephysics_quaternion_new();
  EPhysics_Quaternion *quat1  = ephysics_quaternion_new();
  EPhysics_Quaternion *result = ephysics_quaternion_new();

  Scene_Data *scene = ourGlobals->scene;

  // Animate cube.
  angle += 0.5;
  if (angle > 360.0)		angle -= 360.0f;

  frame += inc;
  if (frame >= 20) inc = -1;
  else if (frame <= 0) inc = 1;

  eo_do(scene->mesh_node,
    evas_3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, 1.0),
    evas_3d_node_mesh_frame_set(scene->mesh, frame)
    );

  // Animate sonic.
  sonicFrame += 32;
  if (sonicFrame > 256 * 50)	sonicFrame = 0;
  eo_do(scene->mesh2_node,
    evas_3d_node_mesh_frame_set(scene->mesh2, sonicFrame)
    );

  // Animate earth.
  earthAngle += 0.3;
  if (earthAngle > 360.0)	earthAngle -= 360.0f;
  eo_do(scene->mesh3_node,
    evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 0.0)
    );

  // Camera movement.
  ephysics_quaternion_euler_set(quat1, ourGlobals->gld.move->r, ourGlobals->gld.move->s, ourGlobals->gld.move->t);
  eo_do(scene->camera_node, evas_3d_node_orientation_get(EVAS_3D_SPACE_PARENT, &x, &y, &z, &w));
  ephysics_quaternion_set(quat, x, y, z, w);
  ephysics_quaternion_multiply(quat, quat1, result);
  ephysics_quaternion_normalize(result);
  ephysics_quaternion_get(result, &x, &y, &z, &w);
  eo_do(scene->camera_node, evas_3d_node_orientation_set(x, y, z, w));

  eo_do(scene->camera_node, evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
  x -= ourGlobals->gld.move->x;
  y -= ourGlobals->gld.move->y;
  z -= ourGlobals->gld.move->z;
  eo_do(scene->camera_node, evas_3d_node_position_set(x, y, z));

  free(result);
  free(quat1);
  free(quat);

  return EINA_TRUE;
}


static void
_camera_setup(globals *ourGlobals, Scene_Data *scene)
{
  scene->camera = eo_add(EVAS_3D_CAMERA_CLASS, ourGlobals->evas,
    evas_3d_camera_projection_perspective_set(60.0, 1.0, 1.0, 500.0)
    );

  scene->camera_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_CAMERA);
  eo_do(scene->camera_node,
    evas_3d_node_camera_set(scene->camera),
    evas_3d_node_position_set(50.0, 0.0, 20.0),
    evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 20.0, EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0)
    );

  eo_do(scene->root_node, evas_3d_node_member_add(scene->camera_node));
}

static void
_light_setup(globals *ourGlobals, Scene_Data *scene)
{
  scene->light = eo_add(EVAS_3D_LIGHT_CLASS, ourGlobals->evas,
    evas_3d_light_ambient_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_directional_set(EINA_TRUE)
    );

  scene->light_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_LIGHT);
  eo_do(scene->light_node,
    evas_3d_node_light_set(scene->light),
    evas_3d_node_position_set(1000.0, 0.0, 1000.0),
    evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0)
    );

  eo_do(scene->root_node, evas_3d_node_member_add(scene->light_node));

}

static void _cube_setup(globals *ourGlobals, Scene_Data *scene)
{
  char buf[PATH_MAX];

  // Setup cube materials.
  scene->texture0 = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
    evas_3d_texture_data_set(EVAS_3D_COLOR_FORMAT_RGBA, EVAS_3D_PIXEL_FORMAT_8888, 4, 4, &pixels0[0])
    );

  scene->texture1 = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
    evas_3d_texture_data_set(EVAS_3D_COLOR_FORMAT_RGBA, EVAS_3D_PIXEL_FORMAT_8888, 4, 4, &pixels1[0])
    );

  snprintf(buf, sizeof(buf), "%s/normal_lego.png", elm_app_data_dir_get());
  scene->texture_normal = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
    evas_3d_texture_file_set(buf, NULL)
    );

  scene->material0 = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas,
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

    evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_shininess_set(100.0),
    evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, scene->texture0)
  );

  scene->material1 = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas,
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

    evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_shininess_set(100.0),

    evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, scene->texture1),
    evas_3d_material_texture_set(EVAS_3D_MATERIAL_NORMAL, scene->texture_normal)
    );

  // Setup CUBE mesh.
  scene->mesh = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas,
    evas_3d_mesh_vertex_count_set(24),
    evas_3d_mesh_frame_add(0),

    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION, 12 * sizeof(float), &cube_vertices[ 0]),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,   12 * sizeof(float), &cube_vertices[ 3]),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,    12 * sizeof(float), &cube_vertices[ 6]),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD, 12 * sizeof(float), &cube_vertices[10]),

    evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]),
    evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),

    evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP),

    evas_3d_mesh_frame_material_set(0, scene->material0),

    evas_3d_mesh_frame_add(20),
    evas_3d_mesh_frame_material_set(20, scene->material1)
    );

  scene->mesh_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_MESH);
  eo_do(scene->mesh_node,
    eo_key_data_set("Name", "cube", NULL),
    evas_3d_node_position_set(40.0, 3.5, 23.0),
    evas_3d_node_mesh_add(scene->mesh)
    );

  eo_do(scene->root_node, evas_3d_node_member_add(scene->mesh_node));
}

static void _sonic_setup(globals *ourGlobals, Scene_Data *scene)
{
  char buf[PATH_MAX];

  // Setup an MD2 mesh.
  snprintf(buf, sizeof(buf), "%s/sonic.png", elm_app_data_dir_get());
  scene->texture2 = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
    evas_3d_texture_file_set(buf, NULL),
    evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST),
    evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT)
    );

  scene->material2 = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas,
    evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, scene->texture2),

    evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

    evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_shininess_set(50.0)
  );

  snprintf(buf, sizeof(buf), "%s/sonic.md2", elm_app_data_dir_get());
  scene->mesh2 = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas,
    evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, buf, NULL),
    evas_3d_mesh_frame_material_set(0, scene->material2),
    evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG)
    );

  scene->mesh2_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_MESH);
  eo_do(scene->mesh2_node,
    eo_key_data_set("Name", "sonic", NULL),
    evas_3d_node_mesh_add(scene->mesh2)
    );

  eo_do(scene->root_node, evas_3d_node_member_add(scene->mesh2_node));
}

static void _earth_setup(globals *ourGlobals, Scene_Data *scene)
{
  char buf[PATH_MAX];

  // Setup earth material.
  snprintf(buf, sizeof(buf), "%s/EarthDiffuse.png", elm_app_data_dir_get());
  scene->texture_diffuse = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
    evas_3d_texture_file_set(buf, NULL),
    evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_LINEAR, EVAS_3D_TEXTURE_FILTER_LINEAR)
  );

  scene->material3 = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas,
    evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, scene->texture_diffuse),

    evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

    evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_shininess_set(50.0)
  );

  // Setup earth mesh.
   _sphere_init(100);

  scene->mesh3 = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas,
    evas_3d_mesh_vertex_count_set(vertex_count),
    evas_3d_mesh_frame_add(0),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION, sizeof(vertex), &sphere_vertices[0].position),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,   sizeof(vertex), &sphere_vertices[0].normal),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TANGENT,  sizeof(vertex), &sphere_vertices[0].tangent),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,    sizeof(vertex), &sphere_vertices[0].color),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD, sizeof(vertex), &sphere_vertices[0].texcoord),

    evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, index_count, &sphere_indices[0]),
    evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
    evas_3d_mesh_frame_material_set(0, scene->material3),

    evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE)
  );

  scene->mesh3_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_MESH);
  eo_do(scene->mesh3_node,
    eo_key_data_set("Name", "earth", NULL),
    evas_3d_node_position_set(40.0, -3.5, 23.0),
    evas_3d_node_mesh_add(scene->mesh3)
  );

  eo_do(scene->root_node, evas_3d_node_member_add(scene->mesh3_node));
}


static void
_scene_setup(globals *ourGlobals, Scene_Data *scene)
{
  // TODO - I have no idea how this should work.
  // It seems the people that wrote the examples don't know either.  lol
//  scene->root_node = eo_add(EVAS_3D_NODE_CLASS, ourGlobals->evas, EVAS_3D_NODE_TYPE_NODE);
  scene->root_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_NODE);

  scene->scene = eo_add(EVAS_3D_SCENE_CLASS, ourGlobals->evas,
    evas_3d_scene_root_node_set(scene->root_node),
    evas_3d_scene_size_set(512, 512),
    evas_3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0)
  );

  _camera_setup(ourGlobals, scene);
  _light_setup(ourGlobals,  scene);
  _cube_setup(ourGlobals,   scene);
  _sonic_setup(ourGlobals,  scene);
  _earth_setup(ourGlobals,  scene);

  eo_do(scene->scene,
    evas_3d_scene_camera_node_set(scene->camera_node)
    );
}


static void _on_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *o, void *einfo)
{
   Scene_Data *scene = data;
   Evas_Event_Mouse_Move *ev = einfo;
   Evas_Coord x, y, w, h;
   Evas_Coord obj_x, obj_y;
   int scene_w, scene_h;
   Evas_Real scene_x, scene_y;
   Evas_Real s, t;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Eina_Bool pick;
   char *name = NULL;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   obj_x = ev->cur.canvas.x - x;
   obj_y = ev->cur.canvas.y - y;

   eo_do(scene->scene, evas_3d_scene_size_get(&scene_w, &scene_h));

   scene_x = obj_x * scene_w / (Evas_Real)w;
   scene_y = obj_y * scene_h / (Evas_Real)h;

   eo_do(scene->scene, pick = evas_3d_scene_pick(scene_x, scene_y, &n, &m, &s, &t));
   if (pick)
     name = evas_object_data_get(n, "Name");
   // This is a raw Evas callback, on the Elm image internal Evas_Object.
   // So we need to get the Elm Image back from the raw Evas_Object.
   // Which is why we stuffed it in the scene structure.
   if (name)
   {
      elm_object_tooltip_text_set(scene->image, name);
      elm_object_tooltip_show(scene->image);
   }
   else
   {
      elm_object_tooltip_text_set(scene->image, "");
      elm_object_tooltip_hide(scene->image);
   }
}

static void _on_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *o, void *einfo)
{
   Scene_Data *scene = data;
   Evas_Event_Mouse_Down *ev = einfo;
   Evas_Coord x, y, w, h;
   Evas_Coord obj_x, obj_y;
   int scene_w, scene_h;
   Evas_Real scene_x, scene_y;
   Evas_Real s, t;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Eina_Bool pick;
   char *name = NULL;

   // Set the focus onto us.
   elm_object_focus_set(o, EINA_TRUE);

   evas_object_geometry_get(o, &x, &y, &w, &h);

   obj_x = ev->canvas.x - x;
   obj_y = ev->canvas.y - y;

   eo_do(scene->scene, evas_3d_scene_size_get(&scene_w, &scene_h));

   scene_x = obj_x * scene_w / (Evas_Real)w;
   scene_y = obj_y * scene_h / (Evas_Real)h;

   eo_do(scene->scene, pick = evas_3d_scene_pick(scene_x, scene_y, &n, &m, &s, &t));
   if (pick)
   {
     name = evas_object_data_get(n, "Name");
     printf("Picked     : ");
   }
   else
     printf("Not picked : ");
   if (NULL == name)
     name = "";

    printf("output(%d, %d) canvas(%d, %d) object(%d, %d) scene(%f, %f) texcoord(%f, %f) "
           "node(%p) %s mesh(%p)\n",
           ev->output.x, ev->output.y,
           ev->canvas.x, ev->canvas.y,
           obj_x, obj_y,
           scene_x, scene_y,
           s, t, n, name, m);
}

void Evas_3D_Demo_add(globals *ourGlobals)
{
  Evas_Object *obj, *temp;

  ourGlobals->scene = &ourScene;
  _scene_setup(ourGlobals, &ourScene);

  // Add an image object for 3D scene rendering.
  obj = eo_add(ELM_OBJ_IMAGE_CLASS, ourGlobals->win,
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    elm_obj_image_fill_outside_set(EINA_TRUE),
    evas_obj_visibility_set(EINA_TRUE),
    temp = elm_obj_image_object_get()
  );
  elm_object_tooltip_text_set(obj, "");
  elm_object_tooltip_hide(obj);
  ourScene.image = obj;

  eo_do(temp,
    evas_obj_image_scene_set(ourScene.scene)
  );
  // Elm can't seem to be able to tell us WHERE an image was clicked, so use raw Evas calbacks instead.
  evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, &ourScene);
  evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, &ourScene);

  cameraAdd(ourGlobals, obj);
  elm_win_resize_object_add(ourGlobals->win, obj);
//  elm_box_pack_end(ourGlobals->gld.bx, obj);

  ourGlobals->gld.move = calloc(1, sizeof(cameraMove));
}

void Evas_3D_Demo_fini(globals *ourGlobals)
{
  eo_unref(ourGlobals->scene->image);
  free(sphere_vertices);
  free(sphere_indices);
}
