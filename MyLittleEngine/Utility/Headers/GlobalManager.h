#pragma once
#include "../../Resource Management/Headers/ResourceManager.h"

class GlobalManager {
	ResourceManager* resourceManager;

public: 
	GlobalManager();
	ResourceManager* getResourceManager();
};