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

#include "evas_macros.h"
#include "evas_3d_utils.h"	// TODO - Hopefully I can convince the authors to make this public.

#include "Runnr.h"


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
  vec3		pos;
  vec4		rot;
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
    script	*scrip;	// Not a typo, C++ is fussy about reusing names like this.
    void	*other;
  } details;
} Stuffs;


typedef struct _loveStuffs
{
  Stuffs	stuffs;
  Eina_Inarray	contents;	// Stuffs
} LoveStuffs;


/* Data flow

love server starts up
  scans sim disk structure looking for scripts in stuffs
    keep track of which script is in which stuffs
      hashed by stuffs uuid, store pointer to script structure
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


/*  More thoughts about directory structure

A major goal is to have the disk structure of a sim (and inventory) be
decently human readable.  The assets should be stored as normal files of
the appropriate type, so that they can be edited with normal editing
software like blender, gimp, a text editor, etc.

A major LL created problem is that in world objects can have the same
name, though stuff in an objects inventory is forced to have unique
names by automatically adding numbers to the names.  In inventory you
can have stuff with the same name to.  The same named objects don't have
to be the same object, they can be completely different.  A simple
object name to file name matching, with directories for contents, just
wont work.  Perhaps the automatic adding of numbers thing would be best? 
All names have to be coerced into operating system friendly names
anyway, so there still needs to be some sort of mapping between objects
and their names.  Not to mention URL escaping for names to.  Try to
combine both.

Should have a tool to validate a sims files.

Sooo, directory structure of a sim -

"sim name" -> sim%20name
  index.omg
    lists objects in the sim, with details about UUID, position, orientation, and size
    maps in world name <-> file name / URL name
      actually use file:// style URLs, so that we can pass them to clients,
      and have the posibility of pointing to assets on some other lspace server
      the name is a duplicate of what's inside the objects .omg file
        Should try to keep these in sync automaticallly
        sim index.omg files are managed by the love server,
        so fiddling with them outside is an expert thing to do
  "object name" -> object%20name.omg and a directory called object%20name
    this file name could also be munged if there are objects with duplicate names
    an external lspace server would return the .omg file when the objects URL is requested
    .omg file contains file names / URLs of other assets like actual mesh files and textures
      also UUID, in world name, description, owner UUID.
        owner UUID might be better dealt with elsewhere?
        coz links
    directory contains the contents of the object
      index.omg
        lists the stuffs in the objects contents, with UUID and type for each
        maps in world name <-> file name / URL name
      the asset files, plus things like compiled script binaries and temporaries
        might be better to move compiled scripts and generated Lua source to the LuaSL server's own private store?
        LuaSL needs to assign UUIDs to compiled script binaries, so it knows which running script is which
          coz there might be lots of copies of scripts
          on the other hand, each copy is uniquely named inside some objects contents,
          even if it's a link,
          so the UUID of the stuffs plus the UUID within the stuffs contents, for this copy / link of the script should suffice?
          except links

Yes, this means that files and directories can be links in the case of duplicate assets.
  Though the UUIDs inside .omg files would have to be unique?
    Only for script binaries I think.
  Would have to do copy on write for editing and other state changes, not including script state changes.
    So links to object .omg files would all have the same UUID.
    Which works well with using sha-1 content addressable assets.  B-)

People could even be free to use their own organising directory
structure.  A directory for ground level, and one for each sky box for
instance.  Relative paths inside .omg files sorts that all out.

Actually, file names / URLs in .omg files would mostly be relative to
the file name / URL of the sim, and all the way down, so relative to the
directory things are in.  It's up to the client to keep track of where
they are and build appropriate full URL / full path file names.

A sims index.omg file might be constantly updated for busy sims.
  Could generate it on the fly by the lspace server, based on the actual contents of the sim directory.
  Alternative lspace servers could just use a CMS system for all of this anyway.
    They would still use the same structure for URLs and .omg files.

Storing avatars in this way isn't such a good idea?
  Avatars could have their own index.omg dealing with shape, skin, clothes, attachments, etc.
  But instead of client asking for a list, avatar arrivals and departures are driven by love.
  Though try to keep avatars as "just ordinary object, but with a person controlling it".
  Still, they move around a lot.

Right now I'm using Lua style .omg files, but eet would be a better
choice I think.  Saves having to write a Lua table to C structure
system, which eet already has, sorta.  Lua tables are at least more
readable while I consider the design, but likely use eet instead when I
have to write code to read the suckers.

Might be useful to have a UUID -> asset file .cache directory, or
something similar?  Could save love server needing to keep that all in
memory all the time.  Plus, an issue with using sha-1 UUIDs and keeping
them inside the .omg files is that this changes the sha-1 hash.  Should
think about keeping uuid's outside of them.  Perhaps a .sha1 file along
side each .omg file and asset file?  Don't bother with links, if an
asset is a link, just look for the .sha1 file where the real file is. 
The UUIDs are mostly for keeping in memory as keys to stuff, coz they
can shrink down to a long.  On the other hand, duplicated objects still
need their own UUIDs to refer to each copy / link.  Sha-1 the relative
path to the link?  It all gets trickier when assets are on different
lspace servers, not to mention some OSes don't support links very well,
or at all.

objectName.omg + objectName.sha1	In each of these cases, the .sha1 file is the sha1 sum of the file it goes with, not of the file it points to.
objectName.lnk + objectName.sha1	.lnk is a relative file name to objectName.omg
objectName.url + objectName.sha1	.url is a URL to a the .omg on a different server
                                          Probably need to store other info in the .url file, sha1 of the original file, other web cache info.
                                          Which starts to encroach on web proxy / cache territory, which we may not want to do,
                                          since we want to use real ones instead of crappy LL cache.

.cache
  sha1 as file names of .lnk or .url files.

If the file system date stamps are different, then someone edited the file, recreate the sha1 files.
  Which implies that lengthy sha1 calculations need to touch the file once done.

*/

#endif
