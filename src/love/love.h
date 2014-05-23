#ifndef _LOVE_H_
#define _LOVE_H_



/*

What do we need as an internal world object format?

Can't call it Things, that's taken already.

"Stuffs" works though.  B-)

Love server needs to manage in world stuffs, and send changes to everyone.

Extantz needs to manage what's shown in world, and edit that stuffs.

Combining the data structures seems sane.  Letting the love server
define those data structures also seems sane, hence this file.


Extantz needs -
  what it looks like (Evas_3D stuff)
    "link sets"
      what is attached to what
    mesh
      vertices, triangles, ...
      local position and orientation relative to parent
    material
      textures
      colours, including alpha
    light
      shade mode
  where it is
    position, orientation
  animation stuff


common
  UUID
  name & description
  owner
  position, orientation
  "link sets"


love needs
  UUID
  name & description
  owner
  position, orientation
  "link sets"
    LL used a list of prims, we can do better I'm sure.
      Though for the sake of this experimental version, just use an array of stuffs.
  contents
    scripts, cards, other stuffs, animations, sounds, textures, ...
    content types

*/


/* Stuffs is a single "primitive" object, it can be -
  single mesh object - NOTE: Should include a list of materials.
    LL style prim, including tree, defined as a small bunch of data defining the type and parameters
    LL style mesh "file"
    single mesh file, collada, other mesh format
    height field, for land, or water
  LuaSL script
  "notecard", just a text file, with no limits
  animation, BVH, Alice's BVJ, or perhaps any other animation file format
  sounds, MP3, other sound formats
  textures, PNG, JPEG, JPEG2000, TIFF, GIF, other texture formats
  other LL stuff
    landmark
    clothing
    body part, shape, skin, hair, or eyes
    gesture
    calling card
*/

#include <Eina.h>

typedef struct _material
{
  int	face;
  //type?
  char	texture[PATH_MAX];
  //colour
  //alpha
  //other stuff
} Material;

typedef struct _mesh
{
  char		fileName[PATH_MAX];
  //type
//  Evas_Vec3	pos;
//  Evas_Vec4	rot;
  Eina_Inarray	materials;	// Material
  Eina_Inarray	parts;		// Mesh
} Mesh;

typedef struct _stuffs
{
  char UUID[32], *name, *description, owner[32];
  //type
  union
  {
    Mesh	*mesh;
//    script	*script;
    void	*other;
  } details;
} Stuffs;


typedef struct _loveStuffs
{
  Stuffs	stuffs;
  Eina_Inarray	contents;	// Stuffs
} LoveStuffs;

typedef struct _extantzStuffs
{
  Stuffs	stuffs;
//  Evas_3D_Mesh	*mesh;
//  Evas_3D_Node	*mesh_node;
  Eina_Inarray	*materials;	// Evas_3D_Material
  Eina_Inarray	*textures;	// Evas_3D_Texture
} ExtantzStuffs;


/* Data flow

love server starts up
  scans sim disk structure looking for scripts in stuffs
    keep track of which script is in which stuffs
    -> LuaSL compile script
    -> LuaSL load this saved state
    -> LuaSL run script

Extantz client starts up
  -> love login(name, password)
          loads user details
  <- love this is your user uuid
  get sim details from lspace at this URL (or local disk directory file://)
  -> lspace gimme the sim (or just pick up the index.omg file from disk)
  <- lspace index.omg
  figure out which mesh stuffs to load
    -> lspace gimme(uuid) (or load it from disk)
    <- lspace uuid.omg
    figure out which mesh and texture files to load
      -> lspace gimme this mesh file (or load it from disk)
      <- lspace mesh file
      -> lspace gimme this texture file (or load it from disk)
      <- lspace texture file

  user clicks on in world stuffs
    -> love touched(user uuid, stuffs.uuid)
            looks up stuffs, loops through scripts
            -> LuaSL script.uuid.touch_start() and the detected stuff

  user edits stuffs in world
    -> love get contents(stuffs.uuid)
            loads stuffs.omg from disk
            loads stuffs.index.omg
    <- love here's the list of contents Stuffs
    -> love change this bit
            changes that bit and stores on disk
            send update nails commands to everyone/thing watching
*/

#endif
