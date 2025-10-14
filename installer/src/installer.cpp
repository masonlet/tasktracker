#pragma comment(lib, "Shell32.lib")

#include "resource.h"
#include "log.hpp"
#include "registryUtils.hpp"
#include "systemUtils.hpp"
#include "fileUtils.hpp"

#include <fstream>

static bool extractTaskTrackerExe(const Path& toPath) {
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

int wmain() {
	if (FAILED(COMInitializer{}))
		return error(L"Failed to initialize COM library");

	if (!isAdmin()) {
		MessageBoxW(NULL, L"Must be ran as administrator", L"Administrator Required", MB_OK | MB_ICONERROR);
		return error(L"Program has not been run as an administrator");
	}

	if (keyExists(REGISTRY_PATH) || fileExists(FILE_PATH)) {	
		if (keyExists(REGISTRY_PATH) && !deleteTaskTrackerKeys()) 
			return error(L"Failed to delete keys");
		else log(L"Keys successfully removed");

		if (fileExists(EXE_PATH) && !deleteFile(EXE_PATH))
			return error(L"Failed to delete executable");
		else log(L"Executable successfully deleted");

		if (fileExists(FILE_PATH) && !deleteDirectory(FILE_PATH)) 
			return error(L"Failed to remove file");
		else log(L"File successfully removed");

		log(L"Uninstallation Completed", true);
	} else {
		if (!createTaskTrackerKeys() || !createDirectory(FILE_PATH) || !extractTaskTrackerExe(EXE_PATH))
			return error(L"Failed to install");

		log(L"Installation Completed", true);
	}

	return EXIT_SUCCESS;
}