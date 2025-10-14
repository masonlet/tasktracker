#include "registryUtils.hpp"
#include "systemUtils.hpp"
#include "resource.h"
#include "log.hpp"

#include <fstream>

bool fileExists(const std::filesystem::path& path) {
	return std::filesystem::exists(path);
}

bool deleteFile(const std::filesystem::path& path) {
	if (fileExists(path) && !std::filesystem::remove(path)) 
		return error(L"Failed to remove file at " + path.wstring());

	return fileExists(path)
		? log(L"Failed to remove file at " + path.wstring(), true)
		: log(L"File Removed at " + path.wstring());
}
bool deleteDirectory(const std::filesystem::path& path) {
	if (fileExists(path) && !std::filesystem::remove_all(path)) 
		return error(L"Failed to remove directory at " + path.wstring());

	return fileExists(path)
		? log(L"Failed to remove directory at " + path.wstring(), true)
		: log(L"Directory Removed at " + path.wstring());
}

bool createDirectory(const std::filesystem::path& path) {
	if (!fileExists(path) && !std::filesystem::create_directories(path))
		return error(L"Failed to create TaskTracker folder at " + path.wstring());

	return fileExists(path)
		? log(L"Folder added at " + path.wstring())
		: log(L"Failed to create folder at " + path.wstring(), true);
}
bool extractTaskTrackerExe(const std::filesystem::path& toPath) {
	HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(TASKTRACKER_EXE), RT_RCDATA);
	if (!hResource) return error(L"Failed to find TaskTracker.exe resource");

	HGLOBAL hLoadedResource = LoadResource(NULL, hResource);
	if (!hLoadedResource) return error(L"Failed to load TaskTracker.exe resource");

	const void* pResourceData = LockResource(hLoadedResource);
	if (!pResourceData) return error(L"Failed to lock TaskTracker.exe resource");

	DWORD resourceSize = SizeofResource(NULL, hResource);
	if (resourceSize == 0) return error(L"TaskTracker.exe resource has zero size");

	std::ofstream outFile(toPath, std::ios::binary);
	if (!outFile) return error(L"Failed to create file at " + toPath.wstring());

	outFile.write(static_cast<const char*>(pResourceData), resourceSize);
	outFile.close();
	return outFile.good()
		? log(L"Successfully extracted TaskTracker.exe to " + toPath.wstring())
		: log(L"Failed to write TaskTracker.exe to " + toPath.wstring(), true);
}