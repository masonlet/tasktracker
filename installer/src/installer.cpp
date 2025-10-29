#include "systemUtils.hpp"
#include "installerUtils.hpp"
#include "log.hpp"

int wmain() {
	if (!TaskTracker::SystemUtils::isAdmin()) 
		return MessageBoxW(NULL, L"Must be ran as administrator", L"Error", MB_OK | MB_ICONERROR), EXIT_FAILURE;

	TaskTracker::SystemUtils::COMInitializer t{};
	return TaskTracker::Installer::isTaskTrackerInstalled() 
		? TaskTracker::Installer::deleteTaskTracker() 
		: TaskTracker::Installer::installTaskTracker();
}