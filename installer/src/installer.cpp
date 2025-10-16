#include "systemUtils.hpp"
#include "installerUtils.hpp"
#include "log.hpp"

int wmain() {
	if (FAILED(COMInitializer{}))
		return error(L"Failed to initialize COM library");

	if (!isAdmin()) 
		return MessageBoxW(NULL, L"Must be ran as administrator", L"Error", MB_OK | MB_ICONERROR), EXIT_FAILURE;

	return isTaskTrackerInstalled() 
		? deleteTaskTracker() 
		: installTaskTracker();
}