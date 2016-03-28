/*
    This is a wrapper around the libg3d mesh loading library.

    Copyright (c) 2010, Dawid Seikel.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MI_MIMESH_H
#define MI_MIMESH_H

#include "g3d/g3d.h"

class LLViewerObject;
class LLVolume;
class LLFace;

class mimesh
{
public:
    static void startup(void);
    static void load(LLViewerObject *object, std::string URL);
    static void getData(LLVolume *volume);
    static void render(LLViewerObject *object);
    static void unload(LLViewerObject* object);
    static void unload(LLVolume* volume);
    static void shutdown(void);

private:
    static G3DContext* context;
};
#endif
