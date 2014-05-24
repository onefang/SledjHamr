// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __EXTANTZ_CAMERA_H_INCLUDED__
#define __EXTANTZ_CAMERA_H_INCLUDED__

#ifdef __cplusplus
#include <ISceneNodeAnimator.h>
#include <vector2d.h>
#include <position2d.h>
#include <SKeyMap.h>
#include <irrArray.h>
#include <ICameraSceneNode.h>

using namespace irr;
using namespace scene;

extern "C"{
#else
typedef struct extantzCamera extantzCamera;
typedef struct ICameraSceneNode ICameraSceneNode;
#endif


cameraMove *getCameraMove(ICameraSceneNode *camera);

#ifdef __cplusplus
}


//namespace irr::gui
//{
//	class ICursorControl;
//}


namespace irr
{
namespace scene
{
    ICameraSceneNode *addExtantzCamera(ISceneManager* sm, ISceneNode* parent, s32 id);

	class extantzCamera : public ISceneNodeAnimator
	{
	public:

		//! Constructor
//		extantzCamera(gui::ICursorControl* cursorControl);
		extantzCamera();

		//! Destructor
		virtual ~extantzCamera();

		//! Animates the scene node, currently only works on cameras
		virtual void animateNode(ISceneNode* node, u32 timeMs);

		//! This animator will receive events when attached to the active camera
		virtual bool isEventReceiverEnabled() const
		{
			return false;
		}

		//! Returns the type of this animator
		virtual ESCENE_NODE_ANIMATOR_TYPE getType() const
		{
			return ESNAT_CAMERA_FPS;
		}

		//! Creates a clone of this animator.
		/** Please note that you will have to drop
		(IReferenceCounted::drop()) the returned pointer once you're
		done with it. */
		virtual ISceneNodeAnimator* createClone(ISceneNode* node, ISceneManager* newManager=0);

		bool NoVerticalMovement;
		// -1.0f for inverted mouse, defaults to 1.0f
		f32 MouseYDirection;

		cameraMove	move;

	private:
		f32 MaxVerticalAngle;
		s32 LastAnimationTime;
//		core::position2d<f32> CenterCursor, CursorPos;
	};
};
};
#endif


#endif // __EXTANTZ_CAMERA_H_INCLUDED__

