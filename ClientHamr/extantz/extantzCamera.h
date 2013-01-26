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

typedef struct
{
    float	x, y, z;
    float	r, s, t;
    float	jump;
} cameraMove;

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
    ICameraSceneNode *addExtantzCamera(ISceneManager* sm, ISceneNode* parent, f32 rotateSpeed, f32 moveSpeed, s32 id, bool noVerticalMovement, f32 jumpSpeed, bool invertMouseY, bool makeActive);

	class extantzCamera : public ISceneNodeAnimator
	{
	public:

		//! Constructor
//		extantzCamera(gui::ICursorControl* cursorControl, f32 rotateSpeed = 100.0f, f32 moveSpeed = .5f, f32 jumpSpeed=0.f, bool noVerticalMovement=false, bool invertY=false);
		extantzCamera(f32 rotateSpeed = 100.0f, f32 moveSpeed = .5f, f32 jumpSpeed=0.f, bool noVerticalMovement=false, bool invertY=false);

		//! Destructor
		virtual ~extantzCamera();

		//! Animates the scene node, currently only works on cameras
		virtual void animateNode(ISceneNode* node, u32 timeMs);

		//! Returns the speed of movement in units per second
		virtual f32 getMoveSpeed() const;

		//! Sets the speed of movement in units per second
		virtual void setMoveSpeed(f32 moveSpeed);

		//! Returns the rotation speed
		virtual f32 getRotateSpeed() const;

		//! Set the rotation speed
		virtual void setRotateSpeed(f32 rotateSpeed);

		//! Sets whether vertical movement should be allowed.
		virtual void setVerticalMovement(bool allow);

		//! Sets whether the Y axis of the mouse should be inverted.
		/** If enabled then moving the mouse down will cause
		the camera to look up. It is disabled by default. */
		virtual void setInvertMouse(bool invert);

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

		cameraMove	move;

	private:
		f32 MaxVerticalAngle;

		f32 MoveSpeed;
		f32 RotateSpeed;
		f32 JumpSpeed;
		// -1.0f for inverted mouse, defaults to 1.0f
		f32 MouseYDirection;

		s32 LastAnimationTime;

		core::position2d<f32> CenterCursor, CursorPos;

		bool firstUpdate;
		bool NoVerticalMovement;
	};
};
};
#endif


#endif // __EXTANTZ_CAMERA_H_INCLUDED__

