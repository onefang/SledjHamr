#include "extantz.h"


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

Eina_Bool _animate_scene(globals *ourGlobals)
{
  static float earthAngle = 0.0f;
  static int   sonicFrame = 0;
  ExtantzStuffs *e;

  Scene_Data *scene = ourGlobals->scene;

  EINA_CLIST_FOR_EACH_ENTRY(e, &ourGlobals->stuffs, ExtantzStuffs, node)
  {
    if (e->animateStuffs)  e->animateStuffs(e);
  }

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
    evas_3d_node_orientation_angle_axis_set(earthAngle, 0.0, 1.0, 0.0)
    );

  return EINA_TRUE;
}

static void _sonic_setup(globals *ourGlobals, Scene_Data *scene)
{
  char buf[PATH_MAX];

  // Setup an MD2 mesh.
  snprintf(buf, sizeof(buf), "%s/sonic.png", prefix_data_get());
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

  snprintf(buf, sizeof(buf), "%s/sonic.md2", prefix_data_get());
  scene->mesh2 = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas,
    evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, buf, NULL),
    evas_3d_mesh_frame_material_set(0, scene->material2),
    evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG)
    );

  scene->mesh2_node = eo_add_custom(EVAS_3D_NODE_CLASS, ourGlobals->evas, evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH),
    eo_key_data_set("Name", "sonic", NULL),
    evas_3d_node_position_set(0.0, 0.0, 0.0),
    evas_3d_node_orientation_set(-0.7071067811865475, 0.0, 0.0, 0.7071067811865475),
    evas_3d_node_mesh_add(scene->mesh2)
    );

  eo_do(scene->root_node, evas_3d_node_member_add(scene->mesh2_node));
}

static void _earth_setup(globals *ourGlobals, Scene_Data *scene)
{
  char buf[PATH_MAX];

  // Setup earth material.
  snprintf(buf, sizeof(buf), "%s/EarthDiffuse.png", prefix_data_get());
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

  scene->mesh3_node = eo_add_custom(EVAS_3D_NODE_CLASS, ourGlobals->evas, evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH),
    eo_key_data_set("Name", "earth", NULL),
    evas_3d_node_position_set(0.0, 0.0, 0.0),
    evas_3d_node_mesh_add(scene->mesh3)
  );

  eo_do(scene->root_node, evas_3d_node_member_add(scene->mesh3_node));
}

/* For now lets just pretend we got stuffs sent from our love.

Stuffs =
{
  name = "Test sim",
  description = 'onefangs test SledjHamr sim.',
--  owner = '12345678-1234-4321-abcd-0123456789ab',
  details = 
  {
    stuffs = 
    {
      Mesh = 
      {
        fileName = 'onefang%27s%20test%20bed.omg',
        pos = {0.0, 4.0, 10.0},
        size = {1.0, 1.0, 1.0},
        rot = {1.0, 0.0, 0.0, 0.0},
      },
      Mesh = 
      {
        fileName = 'sonic.omg',
        pos = {0.0, 0.0, 0.0},
        size = {1.0, 1.0, 1.0},
        rot = {-0.7071067811865475, 0.0, 0.0, 0.7071067811865475},
      },
      Mesh = 
      {
        fileName = 'earth.omg',
        pos = {0.0, 0.0, 0.0},
        size = {1.0, 1.0, 1.0},
        rot = {1.0, 0.0, 0.0, 0.0},
      },
    },
  },
}


Stuffs =
{
  name = "onefang's test bed",
  description = 'Just a pretend bed with MLP scripts for testing SledjHamr.',
--  owner = '12345678-1234-4321-abcd-0123456789ab',
  details = 
  {
    Mesh = 
    {
      kind = 'cube',
      materials = {'normal_lego.png'},
    },
  },
}


Stuffs =
{
  name = "onefang's left testicle",
  description = 'Just a pretend world for testing SledjHamr.',
--  owner = '12345678-1234-4321-abcd-0123456789ab',
  details = 
  {
    Mesh = 
    {
      kind = 'sphere',
-- insert prim parameters here
      materials = {'EarthDiffuse.png'},
    },
  },
}


Stuffs =
{
  name = "Sonic the bed hog.",
  description = 'Just a pretend avatar for testing SledjHamr.',
--  owner = '12345678-1234-4321-abcd-0123456789ab',
  details = 
  {
    Mesh = 
    {
      fileName = 'sonic.md2',
      materials = {'sonic.png'},
    },
  },
}

*/

void stuffs_setup(ExtantzStuffs *stuffs, globals *ourGlobals, Scene_Data *scene, int fake)
{
  char buf[PATH_MAX];
  Material *m;
  Evas_3D_Texture  *t, *t1, *ti;
  Evas_3D_Material *mi, *mj;
  Evas_3D_Mesh     *me;

  // TODO - using Eina arrays of any sort seems a bit heavy, might be better to just count and realloc?
  stuffs->materials = eina_array_new(3);
  stuffs->mesh      = eina_array_new(3);
  stuffs->textures  = eina_array_new(3);
  stuffs->aMaterial = eina_array_accessor_new(stuffs->materials);
  stuffs->aMesh     = eina_array_accessor_new(stuffs->mesh);
  stuffs->aTexture  = eina_array_accessor_new(stuffs->textures);

// TODO - These examples just don't fit neatly into anything I can whip up quickly as a data format.
//        So just fake it for now, and expand the data format later.

  // Textures
  if (1 == fake)
  {
    t = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
      evas_3d_texture_data_set(EVAS_3D_COLOR_FORMAT_RGBA, EVAS_3D_PIXEL_FORMAT_8888, 4, 4, &pixels0[0])
      );
    eina_array_push(stuffs->textures, t);

    t1 = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
      evas_3d_texture_data_set(EVAS_3D_COLOR_FORMAT_RGBA, EVAS_3D_PIXEL_FORMAT_8888, 4, 4, &pixels1[0])
      );
    eina_array_push(stuffs->textures, t1);
  }
  else if (2 == fake)
  {
  }
  else if (3 == fake)
  {
  }

  EINA_INARRAY_FOREACH(stuffs->stuffs.details.mesh->materials, m)
  {
    snprintf(buf, sizeof(buf), "%s/%s", prefix_data_get(), m->texture);
    ti = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas, evas_3d_texture_file_set(buf, NULL));
    eina_array_push(stuffs->textures, ti);
  }

  // Materials.
  if (1 == fake)
  {
    eina_accessor_data_get(stuffs->aTexture, 0, (void **) &t);
    mi = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas,
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

      evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
      evas_3d_material_shininess_set(100.0),
      evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, t)
    );
    eina_array_push(stuffs->materials, mi);

    eina_accessor_data_get(stuffs->aTexture, 1, (void **) &t1);
    eina_accessor_data_get(stuffs->aTexture, 2, (void **) &ti);
    mj = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas,
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

      evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
      evas_3d_material_shininess_set(100.0),

      evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, t1),
      evas_3d_material_texture_set(EVAS_3D_MATERIAL_NORMAL, ti)
    );
    eina_array_push(stuffs->materials, mj);
  }
  else if (2 == fake)
  {
  }
  else if (3 == fake)
  {
  }

  // Meshes
  // TODO - Write real generic cube and sphere stuff later.
  if (MT_CUBE == stuffs->stuffs.details.mesh->type)
  {
    eina_accessor_data_get(stuffs->aMaterial, 0, (void **) &mi);
    eina_accessor_data_get(stuffs->aMaterial, 1, (void **) &mj);
    me = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas,
      evas_3d_mesh_vertex_count_set(24),
      evas_3d_mesh_frame_add(0),

      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION, 12 * sizeof(float), &cube_vertices[ 0]),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,   12 * sizeof(float), &cube_vertices[ 3]),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,    12 * sizeof(float), &cube_vertices[ 6]),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD, 12 * sizeof(float), &cube_vertices[10]),

      evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]),
      evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),

      evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP),

      evas_3d_mesh_frame_material_set(0, mi),

      evas_3d_mesh_frame_add(20),
      evas_3d_mesh_frame_material_set(20, mj)
    );
    eina_array_push(stuffs->mesh, me);
    stuffs->animateStuffs = (aniStuffs) _animateCube;
  }
  else if (MT_CUBE == stuffs->stuffs.details.mesh->type)
  {
  }
  else
  {
  }

  eina_accessor_data_get(stuffs->aMesh, 0, (void **) &me);
  stuffs->mesh_node = eo_add_custom(EVAS_3D_NODE_CLASS, ourGlobals->evas, evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH),
    eo_key_data_set("Name", stuffs->stuffs.name, NULL),
    evas_3d_node_position_set(stuffs->stuffs.details.mesh->pos.x, stuffs->stuffs.details.mesh->pos.y, stuffs->stuffs.details.mesh->pos.z),
    evas_3d_node_mesh_add(me)
    );

  eo_do(scene->root_node, evas_3d_node_member_add(stuffs->mesh_node));
  eina_clist_add_head(&(ourGlobals->stuffs), &(stuffs->node));
}

Material bedMat;
Mesh     bedMesh;
ExtantzStuffs eStuffs;

void Evas_3D_Demo_add(globals *ourGlobals)
{

  bedMat.face = -1;	// face -1 means "all of them I think".
  bedMat.type = TT_NORMAL;
  sprintf(bedMat.texture, "normal_lego.png");

  sprintf(bedMesh.fileName, "onefang%%27s%%20test%%20bed.omg");
  bedMesh.type = MT_CUBE;
  bedMesh.pos.x = 0.0;  bedMesh.pos.y = 4.0;  bedMesh.pos.z = 10.0;
  bedMesh.rot.x = 1.0;  bedMesh.rot.y = 0.0;  bedMesh.rot.z = 0.0;  bedMesh.rot.w = 0.0;
  bedMesh.materials = eina_inarray_new(sizeof(Material), 1);
  bedMesh.materials = eina_inarray_new(sizeof(Mesh), 1);
  eina_inarray_push(bedMesh.materials, &bedMat);

  sprintf(eStuffs.stuffs.UUID, FAKE_UUID);
  sprintf(eStuffs.stuffs.name, "onefang's test bed");
  sprintf(eStuffs.stuffs.description, "Just a pretend bed with MLP scripts for testing SledjHamr.");
  sprintf(eStuffs.stuffs.owner, "12345678-1234-4321-abcd-0123456789ab");
  eStuffs.stuffs.details.mesh = &bedMesh;

  ourGlobals->scene = scenriAdd(ourGlobals->win);

  stuffs_setup(&eStuffs, ourGlobals, ourGlobals->scene, 1);

  _sonic_setup(ourGlobals,  ourGlobals->scene);
  _earth_setup(ourGlobals,  ourGlobals->scene);
}

void Evas_3D_Demo_fini(globals *ourGlobals)
{
  free(sphere_vertices);
  free(sphere_indices);
}
