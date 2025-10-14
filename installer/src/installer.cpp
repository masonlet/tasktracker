#pragma comment(lib, "Shell32.lib")

#include "resource.h"
#include "log.hpp"
#include "registryUtils.hpp"
#include "systemUtils.hpp"
#include "fileUtils.hpp"

#include <Windows.h>
#include <Winreg.h>
#include <shlobj_core.h>
#include <filesystem>
#include <array>
#include <chrono>
#include <iostream>
#include <fstream>

int wmain() {
	if (FAILED(COMInitializer{}))
		return error(L"Failed to initialize COM library");

	if (!isAdmin()) {
		MessageBoxW(NULL, L"Must be ran as administrator", L"Administrator Required", MB_OK | MB_ICONERROR);
		return error(L"Program has not been run as an administrator", true);
	}

	if (keyExists(REGISTRY_PATH) || fileExists(FILE_PATH)) {	
		if (keyExists(REGISTRY_PATH) && !deleteTasktrackerKeys()) 
			return error(L"Failed to delete keys", true);
		else message(L"Keys successfully removed");

		if (fileExists(EXE_PATH) && !deleteFile(EXE_PATH))
			return error(L"Failed to delete executable", true);
		else message(L"Executable successfully deleted");

		if (fileExists(FILE_PATH) && !deleteDirectory(FILE_PATH)) 
			return error(L"Failed to remove file", true);
		else message(L"File successfully removed");

		message(L"Uninstallation Completed", true);
	} else {
		if (!createTaskTrackerKeys() || !createDirectory(FILE_PATH) || !extractTaskTrackerExe(EXE_PATH))
			return error(L"Failed to install", true);

		message(L"Installation Completed", true);
	}

	return EXIT_SUCCESS;
}