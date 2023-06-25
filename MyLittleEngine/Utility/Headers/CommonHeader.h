#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <exception>
#include <Windows.Foundation.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include "../../Resource Management/Headers/ResourceManager.h"
#include "GlobalManager.h"


extern GlobalManager globalManager;