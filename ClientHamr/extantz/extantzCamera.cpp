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
// TODO - Hmmm, Where's CursorControl come from?  Ah, passed to the scene manager constructor, it's a GUI thing that we need to replace with an EFL thing.
ICameraSceneNode *addExtantzCamera(ISceneManager* sm, ISceneNode* parent, f32 rotateSpeed, f32 moveSpeed, s32 id, bool noVerticalMovement, f32 jumpSpeed, bool invertMouseY, bool makeActive)
{
	ICameraSceneNode* node = sm->addCameraSceneNode(parent, core::vector3df(), core::vector3df(0, 0, 100), id, makeActive);
	if (node)
	{
//		ISceneNodeAnimator* anm = new extantzCamera(CursorControl, rotateSpeed, moveSpeed, jumpSpeed, noVerticalMovement, invertMouseY);
		ISceneNodeAnimator* anm = new extantzCamera(NULL, rotateSpeed, moveSpeed, jumpSpeed, noVerticalMovement, invertMouseY);

		// Bind the node's rotation to its target. This is consistent with 1.4.2 and below.
		node->bindTargetAndRotation(true);
		node->addAnimator(anm);
		anm->drop();
	}

	return node;
}


//! constructor
extantzCamera::extantzCamera(gui::ICursorControl* cursorControl, f32 rotateSpeed, f32 moveSpeed, f32 jumpSpeed, bool noVerticalMovement, bool invertY)
    : CursorControl(cursorControl), MaxVerticalAngle(88.0f), MoveSpeed(moveSpeed), RotateSpeed(rotateSpeed), JumpSpeed(jumpSpeed),
    MouseYDirection(invertY ? -1.0f : 1.0f), LastAnimationTime(0), firstUpdate(true), NoVerticalMovement(noVerticalMovement)
{
	#ifdef _DEBUG
	setDebugName("extantzCamera");
	#endif

//	if (CursorControl)
//		CursorControl->grab();

	allKeysUp();
}


//! destructor
extantzCamera::~extantzCamera()
{
//	if (CursorControl)
//		CursorControl->drop();
}


#if 0
// TODO - get rid of this, it's an Irrlicht callback, and I'm replacing it all with EFL.
// Leaving it here for the moment as an example.
bool extantzCamera::OnEvent(const SEvent& evt)
{
	switch(evt.EventType)
	{
	case EET_KEY_INPUT_EVENT:
		for (u32 i=0; i<KeyMap.size(); ++i)
		{
			if (KeyMap[i].KeyCode == evt.KeyInput.Key)
			{
				CursorKeys[KeyMap[i].Action] = evt.KeyInput.PressedDown;
				return true;
			}
		}
		break;

	case EET_MOUSE_INPUT_EVENT:
		if (evt.MouseInput.Event == EMIE_MOUSE_MOVED)
		{
//			CursorPos = CursorControl->getRelativePosition();
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}
#endif

void extantzCamera::animateNode(ISceneNode* node, u32 timeMs)
{
	if (!node || node->getType() != ESNT_CAMERA)
		return;

	ICameraSceneNode* camera = static_cast<ICameraSceneNode*>(node);

	if (firstUpdate)
	{
		camera->updateAbsolutePosition();
//		if (CursorControl )
//		{
//			CursorControl->setPosition(0.5f, 0.5f);
//			CursorPos = CenterCursor = CursorControl->getRelativePosition();
//		}

		LastAnimationTime = timeMs;

		firstUpdate = false;
	}

	// If the camera isn't the active camera, and receiving input, then don't process it.
	if(!camera->isInputReceiverEnabled())
	{
		allKeysUp();
		return;
	}

	scene::ISceneManager * smgr = camera->getSceneManager();
	if(smgr && smgr->getActiveCamera() != camera)
		return;

	// get time
	f32 timeDiff = (f32) ( timeMs - LastAnimationTime );
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
			relativeRotation.Y -= (0.5f - CursorPos.X) * RotateSpeed;
			relativeRotation.X -= (0.5f - CursorPos.Y) * RotateSpeed * MouseYDirection;

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

	// TODO - There is no turning left or right, or the other things I'll need.  So might as well create my own thing to replace this.
	if (CursorKeys[EKA_MOVE_FORWARD])
		pos += movedir * timeDiff * MoveSpeed;

	if (CursorKeys[EKA_MOVE_BACKWARD])
		pos -= movedir * timeDiff * MoveSpeed;

	// strafing

	core::vector3df strafevect = target;
	strafevect = strafevect.crossProduct(camera->getUpVector());

	if (NoVerticalMovement)
		strafevect.Y = 0.0f;

	strafevect.normalize();

	if (CursorKeys[EKA_STRAFE_LEFT])
		pos += strafevect * timeDiff * MoveSpeed;

	if (CursorKeys[EKA_STRAFE_RIGHT])
		pos -= strafevect * timeDiff * MoveSpeed;

	// For jumping, we find the collision response animator attached to our camera
	// and if it's not falling, we tell it to jump.
	if (CursorKeys[EKA_JUMP_UP])
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
					collisionResponse->jump(JumpSpeed);
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


void extantzCamera::allKeysUp()
{
	for (u32 i=0; i<EKA_COUNT; ++i)
		CursorKeys[i] = false;
}


//! Sets the rotation speed
void extantzCamera::setRotateSpeed(f32 speed)
{
	RotateSpeed = speed;
}


//! Sets the movement speed
void extantzCamera::setMoveSpeed(f32 speed)
{
	MoveSpeed = speed;
}


//! Gets the rotation speed
f32 extantzCamera::getRotateSpeed() const
{
	return RotateSpeed;
}


// Gets the movement speed
f32 extantzCamera::getMoveSpeed() const
{
	return MoveSpeed;
}


//! Sets whether vertical movement should be allowed.
void extantzCamera::setVerticalMovement(bool allow)
{
	NoVerticalMovement = !allow;
}


//! Sets whether the Y axis of the mouse should be inverted.
void extantzCamera::setInvertMouse(bool invert)
{
	if (invert)
		MouseYDirection = -1.0f;
	else
		MouseYDirection = 1.0f;
}


ISceneNodeAnimator* extantzCamera::createClone(ISceneNode* node, ISceneManager* newManager)
{
	extantzCamera *newAnimator = new extantzCamera(CursorControl, RotateSpeed, MoveSpeed, JumpSpeed, NoVerticalMovement);
	return newAnimator;
}


} // namespace scene
} // namespace irr

