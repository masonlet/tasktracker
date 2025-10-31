#include "taskTrackerUtils.hpp"

#pragma comment(lib, "Shell32.lib")

#include "systemUtils.hpp"
#include "log.hpp"

#include <fstream>

namespace TaskTracker {
	ArgResult parseArgs(const int argc, wchar_t* argv[]) {
		if (argc < 2) {
			Log::exitError(L"Task Tracker", L"Missing arguments.\nUse --help for usage info.");
			return { ParseResult::Error, std::nullopt };
		}

		if (argv[1] == L"--help" || argv[1] == L"-h") {
			Log::exitSuccess(L"Task Tracker",
				L"Usage: TaskTracker.exe <folder_path> <icon_path>\n"
				L"Example: TaskTracker.exe \"C:\\Projects\" \"C:\\Icons\\project.ico\""
			);
			return { ParseResult::ShowHelp, std::nullopt };
		}

		if (argc != 3) {
			Log::exitError(L"Task Tracker", L"Invalid argument amount\nUsage: TaskTracker.exe <folder_path> <icon_path>");
			return { ParseResult::Error, std::nullopt };
		}

		return { ParseResult::Success, ParsedArgs{Path{ argv[1] }, Path{ argv[2] } } };
	}

	int setFolderIcon(const Path& folder, const Path& icon) {
		if (!SystemUtils::isValidPath(folder, L"Task Tracker"))
			return Log::exitError(L"Task Tracker", L"Failed to set folder icon");

		const Path desktopIni{ folder / "desktop.ini" };
		if (!SystemUtils::deleteDesktopIni(desktopIni, L"Task Tracker"))
			return Log::exitError(L"Task Tracker", L"Failed to delete desktop.ini file");

		if (icon.wstring() == SystemUtils::DEFAULT_ICON_PATH)
			return SystemUtils::exitAndRefresh(folder, L"Task Tracker");

		std::wofstream desktopIniFile(desktopIni);
		if (!desktopIniFile)
			return Log::exitError(L"Task Tracker", L"Failed to create desktopIni file at " + desktopIni.wstring());

		desktopIniFile << L"[.ShellClassInfo]\n"
			<< L"IconResource=" << icon.wstring() << L'\n'
			<< L"[ViewState]\n"
			<< L"Mode=\n"
			<< L"Vid=\n"
			<< L"FolderType=Generic\n";
		desktopIniFile.close();

		if (!SetFileAttributesW(folder.c_str(), FILE_ATTRIBUTE_SYSTEM))
			return Log::exitError(L"Task Tracker", L"Failed to set folder attributes");
		if (!SetFileAttributesW(desktopIni.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
			return Log::exitError(L"Task Tracker", L"Failed to set desktop.ini attributes");

		Log::info(L"Task Tracker", L"Folder icon set successfully");
		return SystemUtils::exitAndRefresh(folder, L"Task Tracker");
	}
}
