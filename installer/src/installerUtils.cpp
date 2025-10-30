#include "installerUtils.hpp"

#include "registryUtils.hpp"
#include "fileUtils.hpp"
#include "resource.h"
#include "log.hpp"

#include <fstream>

namespace TaskTracker::Installer {
	bool isTaskTrackerInstalled() {
		return (RegistryUtils::registryKeyExists(RegistryUtils::REGISTRY_PATH) 
			   || FileUtils::fileExists(SystemUtils::getFilePath(), L"Installer", true)
		) ? Log::info(L"Installer", L"Uninstalling Task Tracker\n")
			: Log::info(L"Installer", L"Installing Task Tracker\n", false);
	}

	bool extractTaskTrackerExe(const Path& toPath) {
		HRSRC resource = FindResource(NULL, MAKEINTRESOURCE(TASKTRACKER_EXE), RT_RCDATA);
		if (!resource) return Log::error(L"Installer", L"Failed to find TaskTracker.exe resource");

		HGLOBAL resourceLoaded = LoadResource(NULL, resource);
		if (!resourceLoaded) return Log::error(L"Installer", L"Failed to load TaskTracker.exe resource");

		LPVOID resourceData = LockResource(resourceLoaded);
		if (!resourceData) return Log::error(L"Installer", L"Failed to lock TaskTracker.exe resource");

		DWORD resourceSize = SizeofResource(NULL, resource);
		if (resourceSize == 0) return Log::error(L"Installer", L"TaskTracker.exe resource has zero size");

		std::ofstream outFile(toPath, std::ios::binary);
		if (!outFile) return Log::error(L"Installer", L"Failed to create file at " + toPath.wstring());

		outFile.write(static_cast<const char*>(resourceData), resourceSize);
		outFile.close();
		return outFile.good()
			? Log::info(L"Installer", L"Successfully extracted TaskTracker.exe to " + toPath.wstring() + L'\n')
			: Log::error(L"Installer", L"Failed to extract TaskTracker.exe to " + toPath.wstring() + L'\n');
	}
	int installTaskTracker() {
		if (!RegistryUtils::createTaskTrackerKeys())
			return Log::exitError(L"Installer", L"Failed to create Task Tracker Keys");

		if (!FileUtils::createDirectory(SystemUtils::getFilePath(), L"Installer"))
			return Log::exitError(L"Installer", L"Failed to create Task Tracker Directory");

		if (!extractTaskTrackerExe(SystemUtils::getExePath()))
			return Log::exitError(L"Installer", L"Failed to extract Task Tracker EXE");

		return Log::exitSuccess(L"Installer", L"Installation Completed");
	}

	int deleteTaskTracker() {
		if (!RegistryUtils::deleteTaskTrackerKeys())
			return Log::exitError(L"Installer", L"Failed to delete keys");

		if (!FileUtils::deleteFile(SystemUtils::getExePath(), L"Installer"))
			return Log::exitError(L"Installer", L"Failed to delete executable at " + SystemUtils::getExePath().wstring());

		if (!FileUtils::deleteDirectory(SystemUtils::getFilePath(), L"Installer"))
			return Log::exitError(L"Installer", L"Failed to delete directory at " + SystemUtils::getFilePath().wstring());

		return Log::exitSuccess(L"Installer", L"Uninstallation Completed");
	}
}