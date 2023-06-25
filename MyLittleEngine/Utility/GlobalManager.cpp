#include "Headers/GlobalManager.h"

GlobalManager::GlobalManager()
{
	resourceManager = new ResourceManager();
}

ResourceManager* GlobalManager::getResourceManager()
{
	return resourceManager;
}