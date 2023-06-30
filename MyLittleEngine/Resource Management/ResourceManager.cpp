#include "Headers/ResourceManager.h"
#include <iostream>
#include <string>
#include <functional>

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

std::shared_ptr<std::vector<VertexStruct>> ResourceManager::loadObj(const char* path)
{
	LoadedFiles.resize(LoadedFiles.size() + 1);
	//Begin Loading File
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
	if (fileData == NULL)
	{
		MessageBox(NULL, "Failed to load file!", "Error", MB_OK);
		return nullptr;
	}
	LoadedFiles.at(LoadedFiles.size() - 1).name = path;

	//Process Data

	int fileSizeInt = fileSize.QuadPart;
	int textureCount = 0;
	int normalCount = 0;
	int totalOffset = 0;
	char* dataPointer = (char*)fileData;

	std::vector<VertexStruct> vertices;
	
	while (totalOffset < fileSizeInt)
	{
		char* secondPointer = dataPointer + 1;
		if ((*dataPointer == 'v') && *secondPointer == ' ')
		{
			dataPointer += 2;
			std::string x = "";
			std::string y = "";
			std::string z = "";
			while (*dataPointer != ' ')
			{
				x += *dataPointer;
				dataPointer += 1;
			}
			dataPointer += 1;
			while (*dataPointer != ' ')
			{
				y += *dataPointer;
				dataPointer += 1;
			}
			dataPointer += 1;
			while (*dataPointer != '\n' && totalOffset < fileSizeInt)
			{
				z += *dataPointer;
				dataPointer += 1;
				totalOffset = dataPointer - (char*)fileData;
			}
			//Vertex Struct declares in this order: Vertices, Normal, Color, UV
			VertexStruct newVertex = {
			Vector4(std::stof(x), std::stof(y), std::stof(z), 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 0.0f),
			Vector4(1.0f, 0.0f, 0.0f, 0.0f),
			Vector2(0.0f, 0.0f)
			};
			vertices.push_back(newVertex);
		}
		else if ((*dataPointer == 'v') && *secondPointer == 'n')
		{
			dataPointer += 3;
			std::string x = "";
			std::string y = "";
			std::string z = "";
			while (*dataPointer != ' ')
			{
				x += *dataPointer;
				dataPointer += 1;
			}
			dataPointer += 1;
			while (*dataPointer != ' ')
			{
				y += *dataPointer;
				dataPointer += 1;
			}
			dataPointer += 1;
			while (*dataPointer != '\n' && totalOffset < fileSizeInt)
			{
				z += *dataPointer;
				dataPointer += 1;
				totalOffset = dataPointer - (char*)fileData;
			}
			vertices[normalCount].normals = Vector4(std::stof(x), std::stof(y), std::stof(z), 0.0f);
			normalCount++;

		}
		else if ((*dataPointer == 'v') && *secondPointer == 't')
		{
			dataPointer += 3;
			std::string x = "";
			std::string y = "";
			while (*dataPointer != ' ')
			{
				x += *dataPointer;
				dataPointer += 1;
			}
			dataPointer += 1;
			while (*dataPointer != '\n' && totalOffset < fileSizeInt)
			{
				y += *dataPointer;
				dataPointer += 1;
				totalOffset = dataPointer - (char*)fileData;
			}
			vertices[textureCount].uv = Vector2(std::stof(x), std::stof(y));
			textureCount++;
		}
		dataPointer += 1;
		totalOffset = dataPointer - (char*)fileData;
	}

	LoadedFiles.at(LoadedFiles.size() - 1).fileHandle = fileData;
	return std::make_shared<std::vector<VertexStruct>>(vertices);

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
