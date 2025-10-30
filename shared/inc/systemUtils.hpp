#pragma once

#include "filePath.h"

#include <shlobj_core.h>

namespace TaskTracker::SystemUtils {
	struct COMInitializer {
		COMInitializer() {
			if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
				throw std::runtime_error("COM initializer failed");
		}
		~COMInitializer() { CoUninitialize(); }
	};

	constexpr std::wstring_view EXE_NAME = L"TaskTracker.exe";
	constexpr std::wstring_view DEFAULT_ICON_PATH = L"C:\\Windows\\System32\\shell32.dll,-4";

	Path getProgramFilesPath();
	inline Path getFilePath() { return getProgramFilesPath() / "Task Tracker"; }
	inline Path getExePath() { return getFilePath() / EXE_NAME; }

	bool isAdmin();

	int exitAndRefresh(const Path& path, const std::wstring_view& caller);
	bool isValidPath(const Path& path, const std::wstring_view& caller);
	bool deleteDesktopIni(const Path& path, const std::wstring_view& caller);
}