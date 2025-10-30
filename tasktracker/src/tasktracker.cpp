#pragma comment(lib, "Shell32.lib")

#include "systemUtils.hpp"
#include "log.hpp"

#include <fstream>

int wmain(int argc, wchar_t* argv[]){
	if (argc < 2) return TaskTracker::Log::exitError(L"Task Tracker", L"Missing arguments.\nUse --help for usage info.");

	const std::wstring_view arg1{ argv[1] };
	if (arg1 == L"--help" || arg1 == L"-h") 
		return TaskTracker::Log::exitSuccess(L"Task Tracker",
			L"Usage: TaskTracker.exe <folder_path> <icon_path>\n"
			L"Example: TaskTracker.exe \"C:\\Projects\" \"C:\\Icons\\project.ico\""
		);

	if (argc != 3) 
		return TaskTracker::Log::exitError(L"Task Tracker", L"Invalid argument amount\nUsage: TaskTracker.exe <folder_path> <icon_path>");

	const TaskTracker::Path& folder{ argv[1] };
	if (!TaskTracker::SystemUtils::isValidPath(folder, L"Task Tracker"))
		return TaskTracker::Log::exitError(L"Task Tracker", L"Failed to set folder icon");

	const TaskTracker::Path desktopIni{ folder / "desktop.ini" };
	if (!TaskTracker::SystemUtils::deleteDesktopIni(desktopIni, L"Task Tracker"))
		return TaskTracker::Log::exitError(L"Task Tracker", L"Failed to delete desktop.ini file");

	const TaskTracker::Path& icon{ argv[2] };
	if (icon.wstring() == TaskTracker::SystemUtils::DEFAULT_ICON_PATH)
		return TaskTracker::SystemUtils::exitAndRefresh(folder, L"Task Tracker");

	std::wofstream desktopIniFile(desktopIni);
	if (!desktopIniFile) 
		return TaskTracker::Log::exitError(L"Task Tracker", L"Failed to create desktopIni file at " + desktopIni.wstring());
	
	desktopIniFile << L"[.ShellClassInfo]\n" 
								 << L"IconResource=" << icon.wstring() << L'\n'
								 << L"[ViewState]\n"
								 << L"Mode=\n"
								 << L"Vid=\n"
								 << L"FolderType=Generic\n";
	desktopIniFile.close();

	if (!SetFileAttributesW(folder.c_str(), FILE_ATTRIBUTE_SYSTEM))
		return TaskTracker::Log::exitError(L"Task Tracker", L"Failed to set folder attributes");
	if (!SetFileAttributesW(desktopIni.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
		return TaskTracker::Log::exitError(L"Task Tracker", L"Failed to set desktop.ini attributes");

	TaskTracker::Log::info(L"Task Tracker", L"Folder icon set successfully");
	return TaskTracker::SystemUtils::exitAndRefresh(folder, L"Task Tracker");
}