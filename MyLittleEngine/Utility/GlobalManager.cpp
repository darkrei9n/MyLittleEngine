#include "Headers/GlobalManager.h"

GlobalManager* GlobalManager::instance = nullptr;

GlobalManager::GlobalManager()
{
	resourceManager = new ResourceManager();
	camera = new Camera();
	timer = new Timer();
	timer->reset();
}

ResourceManager* GlobalManager::getResourceManager()
{
	return resourceManager;
}

Camera* GlobalManager::getCamera()
{
	return camera;
}

Timer* GlobalManager::getTimer()
{
	return timer;
}

void GlobalManager::setCamera(Camera* camera)
{
	this->camera = camera;
}

void GlobalManager::setTimer(Timer* timer)
{
	this->timer = timer;
}