#pragma once
#include "../../Resource Management/Headers/ResourceManager.h"
#include "../../Renderer/Headers/Camera.h"
#include "Timer.h"

class GlobalManager {

	ResourceManager* resourceManager;
	Camera* camera;
	Timer* timer;
	GlobalManager();
	static GlobalManager* instance;

public: 
	static GlobalManager& getInstance() 
	{
		if (!instance) {
			instance = new GlobalManager();
		}
		return *instance;
	}


	ResourceManager* getResourceManager();
	Camera* getCamera();
	Timer* getTimer();
	void setCamera(Camera* camera);
	void setTimer(Timer* timer);
};
