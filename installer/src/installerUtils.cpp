#include "installerUtils.hpp"

#include "registryUtils.hpp"
#include "fileUtils.hpp"
#include "resource.h"
#include "log.hpp"

#include <fstream>

bool isTaskTrackerInstalled() {
	return registryKeyExists(REGISTRY_PATH) || fileExists(FILE_PATH);
}
bool extractTaskTrackerExe(const Path& toPath) {
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

int deleteTaskTracker() {
	if (registryKeyExists(REGISTRY_PATH) && !deleteTaskTrackerKeys())
		return error(L"Failed to delete keys");

	if (fileExists(EXE_PATH) && !deleteFile(EXE_PATH))
		return error(L"Failed to delete executable");

	if (fileExists(FILE_PATH) && !deleteDirectory(FILE_PATH))
		return error(L"Failed to remove file");

	log(L"File and executable deleted successfully");
	return success(L"Uninstallation Completed");
}

int installTaskTracker() {
	if (!createTaskTrackerKeys())
		return error(L"Failed to create Task Tracker Keys");

	if (!createDirectory(FILE_PATH))
		return error(L"Failed to create Task Tracker Directory");

	if (!extractTaskTrackerExe(EXE_PATH))
		return error(L"Failed to extract Task Tracker EXE");

	log(L"File and executable created successfully");
	return success(L"Installation Completed");
}