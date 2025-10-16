#pragma comment(lib, "Shell32.lib")

#include "fileUtils.hpp"
#include "log.hpp"

#include <fstream>
#include <shlobj.h>

static int exitAndRefresh(const Path& path) {
	//a lot of this is probably redundant, windows does not like refreshing and I do not like hard restarting explorer so it takes a long time
	system("ie4uinit.exe -ClearIconCache");

	SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW, path.c_str(), NULL);
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

	SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_PATHW | SHCNF_FLUSH, path.c_str(), NULL);
	SHChangeNotify(SHCNE_ATTRIBUTES, SHCNF_PATHW | SHCNF_FLUSH, path.c_str(), NULL);

	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>("Environment"), SMTO_ABORTIFHUNG, 5000, NULL);

	SHFlushSFCache();

	system("ie4uinit.exe -show");
	return EXIT_SUCCESS;
}

int wmain(int argc, wchar_t* argv[]){
	if (argc != 3) return error(L"Invalid argument amount\nUsage: TaskTracker.exe <folder_path> <icon_path>");

	const Path folderPath{ argv[1] };
	if (!fileExists(folderPath) || !isDirectory(folderPath)) 
		return error(L"Folder path \"" + folderPath.wstring() + L"\" is invalid");

	const Path desktopIniPath{ folderPath / "desktop.ini" };
	if (fileExists(desktopIniPath)) {
		SetFileAttributesW(desktopIniPath.c_str(), FILE_ATTRIBUTE_NORMAL);
		DeleteFileW(desktopIniPath.c_str());
	} 

	const Path iconPath{ argv[2] };
	if (iconPath.wstring() == L"C:\\Windows\\System32\\shell32.dll,-4")
		return exitAndRefresh(folderPath);

	std::ofstream desktopIniFile(desktopIniPath);
	if (!desktopIniFile) return error(L"Failed to create desktopIni file at " + desktopIniPath.wstring());
	
	desktopIniFile << "[.ShellClassInfo]\n" 
				   << "IconResource=" << iconPath.string() << '\n'
				   << "[ViewState]\n"
				   << "Mode=\n"
				   << "Vid=\n"
				   << "FolderType=Generic\n";
	desktopIniFile.close();

	if (!SetFileAttributesW(folderPath.c_str(), FILE_ATTRIBUTE_SYSTEM)) 
		return error(L"Failed to set folder attributes");

	if (!SetFileAttributesW(desktopIniPath.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) 
		return error(L"Failed to set desktop.ini attributes");

	return exitAndRefresh(folderPath);
}