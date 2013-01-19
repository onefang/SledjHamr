// This is a Demo of the Irrlicht Engine (c) 2006 by N.Gebhardt.
// This file is not documented.

#ifndef __C_DEMO_H_INCLUDED__
#define __C_DEMO_H_INCLUDED__

#ifdef _IRR_WINDOWS_
#include <windows.h>
#endif

const int CAMERA_COUNT = 7;

class CDemo : public IEventReceiver
{
public:

	CDemo(GLData *gld, bool additive);

	~CDemo();

	void setup(GLData *gld);
	void preDraw(GLData *gld, u32 now);

	virtual bool OnEvent(const SEvent& event);

private:

	void createLoadingScreen();
	void loadSceneData();
	void switchToNextScene();
	void shoot();
	void createParticleImpacts();

	bool additive;
	IrrlichtDevice *device;

	struct SParticleImpact
	{
		u32 when;
		core::vector3df pos;
		core::vector3df outVector;
	};

	int currentScene;

	scene::IQ3LevelMesh* quakeLevelMesh;
	scene::ISceneNode* quakeLevelNode;
	scene::ISceneNode* skyboxNode;
	scene::IAnimatedMeshSceneNode* model1;
	scene::IAnimatedMeshSceneNode* model2;
	scene::IParticleSystemSceneNode* campFire;

	scene::IMetaTriangleSelector* metaSelector;
	scene::ITriangleSelector* mapSelector;

	s32 sceneStartTime;
	s32 timeForThisScene;

	core::array<SParticleImpact> Impacts;
};

#endif

