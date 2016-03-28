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

#include "linden_common.h"
#include "m3math.h"
#include "m4math.h"
#include "llcamera.h"
#include "llpartdata.h"
#include "lldrawable.h"
#include "llface.h"
#include "llvovolume.h"
#include "pipeline.h"
#include "glib.h"	// For G_BEGIN_DECLS
#include "gl.h"
#include "mimesh.h"
#include "g3d/plugins.h"

void cmdline_printchat(std::string message);


G3DContext* mimesh::context;

void mimesh::startup(void)
{
    GSList *item;
   
    context = g3d_context_new();

    // Print the details.
    for (item = context->plugins; item != NULL; item = item->next)
    {
	G3DPlugin *plugin = (G3DPlugin *) item->data;
	gchar **pext = plugin->extensions;
	std::string message;

	if (plugin->type == G3D_PLUGIN_IMPORT)
	    message = "Model";
	else
	    message = "Image";
	message += " loader " + std::string(plugin->name) + " - " + std::string(plugin->desc_func(context)) + "\n  For file extensions : ";

	while(*pext)
	{
	    message += std::string(*pext) + " ";
	    pext ++;
	}
	LL_WARNS("startup") << message << LL_ENDL;
	cmdline_printchat(message);
    }
}

void mimesh::load(LLViewerObject* object, std::string URL)
{
    if ((URL.substr(0, 7).compare("file://") == 0) || (URL.substr(0, 7).compare("http://") == 0))
    {
	if ((NULL == object->mimeshModel) || (0 != object->mimeshURL.compare(URL)))
	{
	    // Also check if it's not been moved to the volume.
	    if (object->getVolume() && (object->getVolume()->mimeshModel))
	    {
		if (0 == object->mimeshURL.compare(URL))
		    return;
	    }
	    object->mimeshURL = URL;
	    std::string temp_URL = URL;
	    if (temp_URL.substr(0, 7).compare("file://") == 0)
		temp_URL = temp_URL.substr(6);
	    if (NULL != context)
	    {
		G3DModel* model;

		model = g3d_model_load(context, temp_URL.c_str());
		if (model)
		{
		    texture_load_all_textures(model);
		    cmdline_printchat("Loaded model " + temp_URL);
		    if (object->mimeshModel)
		    {
			cmdline_printchat("Freeing old  model.");
			g3d_model_free(object->mimeshModel);
		    }
		    // OK, this is BEFORE the volume is set in the VeiwerObject.  Grumble.
		    object->mimeshModel = model;
		}
		else
		    cmdline_printchat("Failed to load model "  + URL);
	    }
	    else
		cmdline_printchat("No G3D context!");
	}
    }
    else
	object->mimeshURL.clear();
}


/*
    What to do with the data?

LLVeiwerObject contains a pointer to a LLDrawable

LLDrawable has our LLVOVolume
    contains a LLViewerObject, which apparently can be cast to a LLVOVolume
	LLVOVolume  inherits from LLViewerObject <- LLPrimitive <- LLXform
	LLVOAvataer inherits from LLViewerObject and LLCharacter
	    LLPrimitive contains a protected pointer to LLVolume mVolumep, with a public getVolume().

LLVOVolume->mVolumeChanged = TRUE;

LLVOVolume::processUpdateMesssages calls the parent (LLViewerObject) version first.
    THAT loads our model.
Modified LLVOVolume::processUpdateMesssages to stuff the model into llVolumeParams.
    Which calls LLVOVolume::setVolume()
     	Which calls LLPrimitive::setVolume() only thing that does outside of LLPrimitive (oh and llvotextbubble).
	Then for sculpties it calls LLVOVolume->sculpt() which does the actual work.

We want to intercept LLVolume->sculpt() to load our mesh data instead.
Actually load our mesh data before sculpt(), then disable that from ever being called.

LLVolumeParams::mSculptID is the sculptie map texture UUID.

LLVolume->mMesh gets the original prim and sculpty data.
llmath/LLVolume->mMesh is - 	std::vector<Point> mMesh;  // Can treat it like an array.
mMesh is protected.
Point contains a LLVector3 -	Point->mPos (-0.5 to 0.5)
LLVolumeFace->mVertices gets a copy.
LLFace->mVertexBuffer gets a copy.
    Does the actual drawing (pushing vertices and such to GL).
    Has pointers to LLDrawable and LLViewerObject.

Hmmm, does everything assume that a LLVolume has a path and a profile?

G3DModel
    gint32 glflags = 
	// G3D_FLAG_GL_SPECULAR |
	G3D_FLAG_GL_SHININESS |
	G3D_FLAG_GL_ALLTWOSIDE |
	G3D_FLAG_GL_TEXTURES;

    for(f = 1.0; f >= 0.0; f -= 0.2)
    {
	G3DFloat min_a = f, max_a = f + 0.2;
	gl_draw_objects(glflags, model->objects, min_a, max_a);

	    GSList *olist = model->objects;
	    while(olist != NULL)
	    {
		int i;
		G3DObject object = (G3DObject *) olist->data;
		olist = olist->next;
		dont_render = FALSE;
		// don't render invisible objects.
		if(object->hide) continue;

		for(i = 0; i < object->_num_faces; i++)
		{
		    gl_draw_face(glflags, object, i, min_a, max_a, &dont_render, &init);

			// draw triangles
			for(j = 0; j < 3; j++)
			{
			    if((glflags & G3D_FLAG_GL_TEXTURES) && (object->_flags[i] & G3D_FLAG_FAC_TEXMAP))
			    {
				glTexCoord2f(	object->_tex_coords[(i * 3 + j) * 2 + 0],
						object->_tex_coords[(i * 3 + j) * 2 + 1]);
			    }

			    glNormal3f(	object->_normals[(i * 3 + j) * 3 + 0],
					object->_normals[(i * 3 + j) * 3 + 1],
					object->_normals[(i * 3 + j) * 3 + 2]);
			    glVertex3f(	object->vertex_data[object->_indices[i * 3 + j] * 3 + 0],
					object->vertex_data[object->_indices[i * 3 + j] * 3 + 1],
					object->vertex_data[object->_indices[i * 3 + j] * 3 + 2]);
			} // 1 .. 3 
		} // all faces.

		// handle sub-objects recursively.
		gl_draw_objects(glflags, object->objects, min_a, max_a);
	    }
    }
*/


static int countTriangles(GSList *objects)
{
    int result = 0;

    while(objects != NULL)
    {
	G3DObject *object = (G3DObject *) objects->data;

	objects = objects->next;
	if (object->hide) continue;
	result += object->_num_faces;
	result += countTriangles(object->objects);
    }
    return result;
}


static void getVerticesAndStuff(GSList *objects, std::vector<LLVolume::Point> mesh, LLVolumeFace& vf, int *i, LLVector3& face_min, LLVector3& face_max)
{
    while(objects != NULL)
    {
	G3DObject *object = (G3DObject *) objects->data;
	int j, k;
	int count = object->_num_faces;

	objects = objects->next;
	if (object->hide) continue;

	for (k = 0 ; k < count; k++)
	{
	    for (j = 0; j < 3; j++)
	    {
//		mesh[*i].mPos.mV[0] = object->vertex_data[	object->_indices	[k * 3 + j] * 3 + 0];
//		mesh[*i].mPos.mV[1] = object->vertex_data[	object->_indices	[k * 3 + j] * 3 + 1];
//		mesh[*i].mPos.mV[2] = object->vertex_data[	object->_indices	[k * 3 + j] * 3 + 2];
		mesh[*i].mPos = LLVector3(			object->vertex_data	[(k * 3 + j) * 3 + 0],	// F64() = G3DFloat() 
								object->vertex_data	[(k * 3 + j) * 3 + 1],
								object->vertex_data	[(k * 3 + j) * 3 + 2]);

		vf.mVertices[*i].mPosition = mesh[*i].mPos;
//		vf.mVertices[*i].mTexCoord = LLVector2(		object->_tex_coords	[(k * 3 + j) * 2 + 0],	// F32() = G3DFloat()
//								object->_tex_coords	[(k * 3 + j) * 2 + 1]);

		vf.mVertices[*i].mNormal = LLVector3(		object->_normals	[(k * 3 + j) * 3 + 0],	// F64() = G3DFloat()
								object->_normals	[(k * 3 + j) * 3 + 1],
								object->_normals	[(k * 3 + j) * 3 + 2]);

		vf.mVertices[*i].mBinormal = LLVector3(0,0,0);  // Um, do we have binormals?  Viewer does not seem to have them either.  shrugs

		if (0 == *i)
		{
			face_min = face_max = mesh[*i].mPos;
		}
		else
		{
			update_min_max(face_min, face_max, mesh[*i].mPos);
		}

		// Damn, at this point viewer wants quads, but the library uses triangles.
		// Those few times when the library deals with quads, the forth is just the next index.
		// No idea what the edges are for.  -1 seems to mean no edge, so go with that for now.
		vf.mIndices	[(*i) * 6 + j] = object->_indices[k * 3 + j];
		vf.mEdge	[(*i) * 6 + j] = -1;
	    }
	    vf.mIndices		[(*i) * 6 + 3] = object->_indices[k * 3 + 0];	// U16 = guint32 
	    vf.mIndices		[(*i) * 6 + 4] = object->_indices[k * 3 + 3];
	    vf.mIndices		[(*i) * 6 + 5] = object->_indices[k * 3 + 1];
	    vf.mEdge		[(*i) * 6 + 3] = -1;				// S32 = Fucked if I know.
	    vf.mEdge		[(*i) * 6 + 4] = -1;
	    vf.mEdge		[(*i) * 6 + 5] = -1;
	    (*i)++;
	}

	getVerticesAndStuff(object->objects, mesh, vf, i, face_min, face_max);
    }
}


void mimesh::getData(LLVolume* volume)
{
    G3DModel *model = volume->mimeshModel;
    if ((model) && (model->objects))
    {
	int vertices = countTriangles(model->objects) * 3;
	int index = 0;

	LL_WARNS("getData") << "Transferring model data for " << std::string(model->filename) << " with " << vertices << " vertices." << LL_ENDL;

	/*
	sculpt_calc_mesh_resolution(sculpt_width, sculpt_height, sculpt_type, sculpt_detail, requested_sizeS, requested_sizeT);
		vertices = T * S / 4         = faces / 16 * 16 / 4   = T
		ratio = T / S                = faces / 16 / 16       = faces / 256
		s = sqrt(vertices / ratio)   = T / faces / 256      = faces / 16 / faces / 256   = 16 (sqrt = 4)
		s = llmax(s, 4);              // no degenerate sizes, please  = 4
		t = vertices / s;                                             = T / 4  = faces / 64
		t = llmax(t, 4);              // no degenerate sizes, please
		s = vertices / t;                                             = T / s	= faces / 16 / s
	*/
	S32 requested_sizeS = llmax (((int) sqrt(vertices)), 4);
	S32 requested_sizeT = llmax(vertices / requested_sizeS, 4);
	// create meshes with high LOD always
	F32 sculpt_detail = 4.0;

LL_WARNS("getData") << "mimesh 0" << LL_ENDL;
	volume->mPathp->generate(volume->mParams.getPathParams(), sculpt_detail, 0, TRUE, requested_sizeS);
	volume->mProfilep->generate(volume->mParams.getProfileParams(), volume->mPathp->isOpen(), sculpt_detail, 0, TRUE, requested_sizeT);

	S32 sizeS = volume->mPathp->mPath.size();         // we requested a specific size, now see what we really got
	S32 sizeT = volume->mProfilep->mProfile.size();   // we requested a specific size, now see what we really got

LL_WARNS("getData") << "mimesh 1   S " << requested_sizeS << "->" << sizeS << " T " << requested_sizeT << "->" << sizeT << " = " << (sizeS * sizeT) << LL_ENDL;
	volume->sNumMeshPoints -= volume->mMesh.size();
	volume->mMesh.resize(sizeS * sizeT);
	volume->sNumMeshPoints += volume->mMesh.size();
LL_WARNS("getData") << "mimesh 2" << LL_ENDL;
	volume->mFaceMask = LLVolumeFace::SIDE_MASK | LLVolumeFace::OUTER_MASK;  // Seems to be the default.
	volume->mSculptLevel = -2;
	volume->mimeshNeedData = FALSE;

LL_WARNS("getData") << "mimesh 3" << LL_ENDL;
	// Delete any existing faces so that they get regenerated
	volume->mVolumeFaces.clear();
LL_WARNS("getData") << "mimesh 4" << LL_ENDL;
//	volume->createVolumeFaces();
	S32 num_faces = volume->mProfilep->mFaces.size();
	volume->mVolumeFaces.resize(num_faces);

LL_WARNS("getData") << "mimesh 5    " << num_faces << LL_ENDL;
	// It's pretending to be a sculpty, so only one "face / side".
	// Initialize volume faces with parameter data
	for (S32 i = 0; i < num_faces; i++)
	{
LL_WARNS("getData") << "mimesh 5.0 " << LL_ENDL;
	    LLVolumeFace& vf = volume->mVolumeFaces[i];
	    LLProfile::Face& face = volume->mProfilep->mFaces[i];
	    LLVector3& face_min = vf.mExtents[0];
	    LLVector3& face_max = vf.mExtents[1];

	    vf.mTypeMask	= volume->mFaceMask;
            vf.mBeginS		= face.mIndex;
	    vf.mNumS		= face.mCount;
	    vf.mBeginT		= 0;
	    vf.mNumT		= volume->getPath().mPath.size();
	    vf.mID		= i;

	    // Just to confuse matters, S and T are reversed from what it is above.  Blame LL.
LL_WARNS("getData") << "mimesh 5.1   S " << vf.mBeginS << "->" << vf.mNumS << " T " << vf.mBeginT << "->" << vf.mNumT << LL_ENDL;
//	    vf.create(volume, FALSE);
	    S32 num_vertices = vf.mNumS * vf.mNumT;
	    S32 num_indices = (vf.mNumS - 1) * (vf.mNumT - 1) * 6;

LL_WARNS("getData") << "mimesh 5.2 " << LL_ENDL;
	    vf.mVertices.resize(vertices);
	    vf.mIndices.resize(num_indices);
	    vf.mEdge.resize(num_indices);

LL_WARNS("getData") << "mimesh 5.3 " << LL_ENDL;
	    vf.mCenter.clearVec();

LL_WARNS("getData") << "mimesh 5.4 " << LL_ENDL;
	    getVerticesAndStuff(model->objects, volume->mMesh, vf, &index, face_min, face_max);

LL_WARNS("getData") << "mimesh 5.5 " << LL_ENDL;
	    vf.mCenter = (face_min + face_max) * 0.5f;

LL_WARNS("getData") << "mimesh 5.6 " << LL_ENDL;
	}
LL_WARNS("getData") << "mimesh 6" << LL_ENDL;
    }
LL_WARNS("getData") << "mimesh 7" << LL_ENDL;
}


void mimesh::render(LLViewerObject* object)
{
    gl_draw_model(object->mimeshModel);
}

void mimesh::unload(LLViewerObject* object)
{
    cmdline_printchat("VO Deleting model data for " + std::string(object->mimeshModel->filename));
    g3d_model_free(object->mimeshModel);
    object->mimeshModel = NULL;
    object->mimeshURL.clear();
}

void mimesh::unload(LLVolume* volume)
{
    cmdline_printchat("V  Deleting model data for " + std::string(volume->mimeshModel->filename));
    g3d_model_free(volume->mimeshModel);
    volume->mimeshModel = NULL;
}

void mimesh::shutdown(void)
{
    g3d_context_free(context);
}
