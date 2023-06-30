#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <Windows.h>
#include <strsafe.h>
#include "../../Renderer/Headers/APIBase.h"

struct FileInfo {
	const char* name;
	HANDLE fileHandle;
};

class ResourceManager {
	std::vector<FileInfo> LoadedFiles;
	uint32_t totalFiles = 0;
	uint32_t totalMemory = 0;
	LPSYSTEM_INFO sysInfo;
	DWORD systemGranularity;

public: 
	ResourceManager();
	void loadFile(const char* path, std::function<void(std::pair<void*, int64_t>)>);
	std::shared_ptr<std::vector<VertexStruct>> loadObj(const char* path);
	FileInfo getFile(const char* fileName);
};