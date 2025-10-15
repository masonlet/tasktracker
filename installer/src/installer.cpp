#include "systemUtils.hpp"
#include "installerUtils.hpp"
#include "log.hpp"

int wmain() {
	if (FAILED(COMInitializer{}))
		return error(L"Failed to initialize COM library");

	if (!isAdmin()) {
		MessageBoxW(NULL, L"Must be ran as administrator", L"Administrator Required", MB_OK | MB_ICONERROR);
		return error(L"Program has not been run as an administrator");
	}

	return isTaskTrackerInstalled() ? deleteTaskTracker() : installTaskTracker();
}