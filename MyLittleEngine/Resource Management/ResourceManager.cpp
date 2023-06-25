#include "Headers/ResourceManager.h"


ResourceManager::ResourceManager()
{
	LoadedFiles.resize(0);
	sysInfo = new SYSTEM_INFO();
	GetSystemInfo(sysInfo);
	systemGranularity = sysInfo->dwAllocationGranularity;
	totalMemory = 0;
	totalFiles = 0;
}

void ResourceManager::loadFile(const char* path, std::function<void(std::pair<void*, int64_t>)> func)
{
	LoadedFiles.resize(LoadedFiles.size() + 1);
	
	HANDLE fileHandle;

	fileHandle = CreateFile(path,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

	HANDLE mapFile = CreateFileMapping(fileHandle,
				NULL,
				PAGE_READONLY,
				0,
				0,
				path);
	void* fileData = MapViewOfFile(mapFile,
						FILE_MAP_READ,
						0,
						0,
						0);

	LARGE_INTEGER fileSize;
	GetFileSizeEx(fileHandle, &fileSize);

	DWORD openFile = GetLastError();
	if (fileData ==  NULL)
	{
		MessageBox(NULL, "Failed to load file!", "Error", MB_OK);
		return;
	}
	LoadedFiles.at(LoadedFiles.size() - 1).name = path;

	std::pair<void*, int64_t> filePair(fileData, fileSize.QuadPart);
	func(filePair);
}

FileInfo ResourceManager::getFile(const char* fileName)
{
	for (int i = 0; i < LoadedFiles.size(); i++)
	{
		if (strcmp(LoadedFiles.at(i).name, fileName) == 0)
		{
			return LoadedFiles.at(i);
		}
	}
	return FileInfo();
}
