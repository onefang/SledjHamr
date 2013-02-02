// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

// The above is the copyright notice for CSceneNodeAnimatorCameraFPS.cpp,
// According to the Irrlicht docs, that's just a demo and you are supposed to use it as an example for writing your own FPS style camera.
// I'll be writing my own camera code, that includes first person, third person, and free camera styles.
// I'll start with CSceneNodeAnimatorCameraFPS.cpp and morph it until it suits me.
// As such, I expect lots of Nikolaus Gebhardt's code to go away.
// To be replaced by my code, which will be copyright and licensed under the same license as the rest of extantz.

// Initally I'll make it SecondLife like, coz that's what my muscle memory is used to.
// It will get extended and made generic though.

#include "extantzCamera.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "Keycodes.h"
#include "ICursorControl.h"
#include "ICameraSceneNode.h"
#include "ISceneNodeAnimatorCollisionResponse.h"

namespace irr
{
namespace scene
{

// Irrlicht hard codes a reference to the original FPS camera code inside it's scene manager.  This is that code extracted so we can be more flexible.
// TODO - Hmmm, Where's CursorControl come from?  Ah, passed to the scene manager constructor, it's a GUI thing that we need to replace with an EFL thing.  But only for mouselook mode.
ICameraSceneNode *addExtantzCamera(ISceneManager* sm, ISceneNode* parent, s32 id)
{
	ICameraSceneNode* node = sm->addCameraSceneNode(parent, core::vector3df(), core::vector3df(0, 0, 100), id, true);
	if (node)
	{
//		ISceneNodeAnimator* anm = new extantzCamera(CursorControl);
		ISceneNodeAnimator* anm = new extantzCamera();

		// Bind the node's rotation to its target. This is consistent with 1.4.2 and below.
		node->bindTargetAndRotation(true);
		node->addAnimator(anm);
		anm->drop();
	}

	return node;
}


//! constructor
//extantzCamera::extantzCamera(gui::ICursorControl* cursorControl)
//    : CursorControl(cursorControl), MaxVerticalAngle(88.0f), MoveSpeed(0.4f), RotateSpeed(100.0f), JumpSpeed(3.0f),
extantzCamera::extantzCamera()
    : MaxVerticalAngle(88.0f), MouseYDirection(1.0f), LastAnimationTime(0), NoVerticalMovement(false)
{
	#ifdef _DEBUG
	setDebugName("extantzCamera");
	#endif

	move.MoveSpeed = 0.1f;
	move.RotateSpeed = 1.0f;
	move.JumpSpeed = 3.0f;

//	if (CursorControl)
//		CursorControl->grab();
}


//! destructor
extantzCamera::~extantzCamera()
{
//	if (CursorControl)
//		CursorControl->drop();
}


/* Have a moveRotate array of floats.
 * X, Y, Z, and whatever the usual letters are for rotations.  lol
 * Each one means "move or rotate this much in this direction".
 * Where 1.0 means "what ever the standard move is if that key is held down".
 * So a keyboard move would just change it's part to 1.0 or -1.0 on key down,
 *   and back to 0.0 on key up.  Or 2.0 / -2.0 if in run mode.
 *   Which would even work in fly mode.
 * A joystick could be set to range over -2.0 to 2.0, and just set it's part directly.
 * A mouse look rotate, well will come to that when we need to.  B-)
 *   I think setting the x or y to be the window position of the mouse (-1.0 to 1.0) should do it.
 *   Or not.  meh
 */

void extantzCamera::animateNode(ISceneNode* node, u32 timeMs)
{
	if (!node || node->getType() != ESNT_CAMERA)
		return;

	ICameraSceneNode* camera = static_cast<ICameraSceneNode*>(node);

	if (0 == LastAnimationTime)
	{
		camera->updateAbsolutePosition();
//		if (CursorControl )
//		{
//			CursorControl->setPosition(0.5f, 0.5f);
//			CursorPos = CenterCursor = CursorControl->getRelativePosition();
//		}

		LastAnimationTime = timeMs;
	}

	// If the camera isn't the active camera, and receiving input, then don't process it.
	// TODO - it never is, coz we are bypassing that, but can we replace this with something else?
	if(!camera->isInputReceiverEnabled())
	{
//		return;
	}

	scene::ISceneManager * smgr = camera->getSceneManager();
	if(smgr && smgr->getActiveCamera() != camera)
		return;

	// get time
	f32 timeDiff = (f32) (timeMs - LastAnimationTime);
	LastAnimationTime = timeMs;

	// update position
	core::vector3df pos = camera->getPosition();

	// Update rotation
	core::vector3df target = (camera->getTarget() - camera->getAbsolutePosition());
	core::vector3df relativeRotation = target.getHorizontalAngle();

#if 0
	if (CursorControl)
	{
		if (CursorPos != CenterCursor)
		{
			relativeRotation.Y -= (0.5f - CursorPos.X) * move.RotateSpeed;
			relativeRotation.X -= (0.5f - CursorPos.Y) * move.RotateSpeed * MouseYDirection;

			// X < MaxVerticalAngle or X > 360-MaxVerticalAngle

			if (relativeRotation.X > MaxVerticalAngle*2 &&
				relativeRotation.X < 360.0f-MaxVerticalAngle)
			{
				relativeRotation.X = 360.0f-MaxVerticalAngle;
			}
			else
			if (relativeRotation.X > MaxVerticalAngle &&
				relativeRotation.X < 360.0f-MaxVerticalAngle)
			{
				relativeRotation.X = MaxVerticalAngle;
			}

			// Do the fix as normal, special case below
			// reset cursor position to the centre of the window.
			CursorControl->setPosition(0.5f, 0.5f);
			CenterCursor = CursorControl->getRelativePosition();

			// needed to avoid problems when the event receiver is disabled
			CursorPos = CenterCursor;
		}

		// Special case, mouse is whipped outside of window before it can update.
		video::IVideoDriver* driver = smgr->getVideoDriver();
		core::vector2d<u32> mousepos(u32(CursorControl->getPosition().X), u32(CursorControl->getPosition().Y));
		core::rect<u32> screenRect(0, 0, driver->getScreenSize().Width, driver->getScreenSize().Height);

		// Only if we are moving outside quickly.
		bool reset = !screenRect.isPointInside(mousepos);

		if(reset)
		{
			// Force a reset.
			CursorControl->setPosition(0.5f, 0.5f);
			CenterCursor = CursorControl->getRelativePosition();
			CursorPos = CenterCursor;
 		}
	}
#else
	relativeRotation.Y -= move.r * move.RotateSpeed;
	relativeRotation.X -= move.s * move.RotateSpeed * MouseYDirection;

	// X < MaxVerticalAngle or X > 360-MaxVerticalAngle

	if ((relativeRotation.X > (MaxVerticalAngle * 2)) && (relativeRotation.X < (360.0f - MaxVerticalAngle)))
		relativeRotation.X = 360.0f - MaxVerticalAngle;
	else if ((relativeRotation.X > MaxVerticalAngle) && (relativeRotation.X < (360.0f - MaxVerticalAngle)))
		relativeRotation.X = MaxVerticalAngle;
#endif

	// set target

	target.set(0,0, core::max_(1.f, pos.getLength()));
	core::vector3df movedir = target;

	core::matrix4 mat;
	mat.setRotationDegrees(core::vector3df(relativeRotation.X, relativeRotation.Y, 0));
	mat.transformVect(target);

	if (NoVerticalMovement)
	{
		mat.setRotationDegrees(core::vector3df(0, relativeRotation.Y, 0));
		mat.transformVect(movedir);
	}
	else
	{
		movedir = target;
	}

	movedir.normalize();

	pos += movedir * timeDiff * move.MoveSpeed * move.x;

	// strafing
	core::vector3df strafevect = target;
	strafevect = strafevect.crossProduct(camera->getUpVector());

	if (NoVerticalMovement)
		strafevect.Y = 0.0f;

	strafevect.normalize();

	pos += strafevect * timeDiff * move.MoveSpeed * move.y;

	// For jumping, we find the collision response animator attached to our camera
	// and if it's not falling, we tell it to jump.
	if (0.0 < move.jump)
	{
		const ISceneNodeAnimatorList& animators = camera->getAnimators();
		ISceneNodeAnimatorList::ConstIterator it = animators.begin();
		while(it != animators.end())
		{
			if(ESNAT_COLLISION_RESPONSE == (*it)->getType())
			{
				ISceneNodeAnimatorCollisionResponse * collisionResponse =
					static_cast<ISceneNodeAnimatorCollisionResponse *>(*it);

				if(!collisionResponse->isFalling())
					collisionResponse->jump(move.JumpSpeed);
			}

			it++;
		}
	}

	// write translation
	camera->setPosition(pos);

	// write right target
	target += pos;
	camera->setTarget(target);
}


ISceneNodeAnimator* extantzCamera::createClone(ISceneNode* node, ISceneManager* newManager)
{
//	extantzCamera *newAnimator = new extantzCamera(CursorControl);
	extantzCamera *newAnimator = new extantzCamera();
	return newAnimator;
}

#ifdef  __cplusplus
extern "C" {
#endif

cameraMove *getCameraMove(ICameraSceneNode *camera)
{
    cameraMove *cm = NULL;

    if (camera)
    {
	const ISceneNodeAnimatorList &animators = camera->getAnimators();
	ISceneNodeAnimatorList::ConstIterator it = animators.begin();
	while(it != animators.end())
	{
		// TODO - We assume FPS == extantzCamera, coz Irrlicht hard codes the camera types in an enum, which is a pain to add to from outside.
		if(ESNAT_CAMERA_FPS == (*it)->getType())
		{
			extantzCamera *ec = static_cast<extantzCamera *>(*it);

			cm = &(ec->move);
		}

		it++;
	}
    }
    return cm;
}

#ifdef  __cplusplus
}
#endif


} // namespace scene
} // namespace irr

