#pragma comment(lib, "Shell32.lib")

/*
Program name: Task Tracker
Purpose: Changes a folders icon to the selected buttons icon
Author: Mason L'Etoile
Date: May 04, 2025
Version: 1.0.0
*/

#include <iostream>
#include <filesystem>
#include <fstream>
#include <shlobj.h>

static int error(const std::wstring& error) {
	std::wcout << L"TaskTracker [Error]: " << error << L'\n';

	std::wcout << L"Press Enter to continue...";
	std::wcin.get();
	return EXIT_FAILURE;
}

static void refreshIcon(const std::filesystem::path& path) {
	//a lot of this is probably redundant, windows does not like refreshing and I do not like hard restarting explorer so it takes a long time
	system("ie4uinit.exe -ClearIconCache");

	SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW, path.c_str(), NULL);
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_PATHW | SHCNF_FLUSH, path.c_str(), NULL);
	SHChangeNotify(SHCNE_ATTRIBUTES, SHCNF_PATHW | SHCNF_FLUSH, path.c_str(), NULL);

	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
	SHFlushSFCache();

	system("ie4uinit.exe -show");
}

static bool isAdmin() {
	bool isElevated = false;
	HANDLE hToken = nullptr;

	if( OpenProcessToken( GetCurrentProcess( ),TOKEN_QUERY,&hToken ) ) {
		TOKEN_ELEVATION elevation{};
        DWORD elevationSize = sizeof( TOKEN_ELEVATION );

        if( GetTokenInformation( hToken, TokenElevation, &elevation, sizeof( elevation ), &elevationSize ) ) {
            isElevated = elevation.TokenIsElevated;
        }
    }
    if( hToken ) {
        CloseHandle( hToken );
    }
    return isElevated;
}

int wmain(int argc, wchar_t* argv[]){
	if (!isAdmin())
		return error(L"Program has not been run as an administrator");
	if (argc != 3) 
		return error(L"Invalid argument amount\nUsage: TaskTracker.exe <folder_path> <icon_path>");

	std::filesystem::path folderPath = std::filesystem::path(argv[1]);
	if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath)) 
		return error(L"Folder path \"" + folderPath.wstring() + L"\" is invalid");

	std::filesystem::path desktopIniPath = folderPath / "desktop.ini";
	if (std::filesystem::exists(desktopIniPath)) {
		SetFileAttributesW(desktopIniPath.c_str(), FILE_ATTRIBUTE_NORMAL);
		DeleteFileW(desktopIniPath.c_str());
		refreshIcon(folderPath);
	} 

	std::filesystem::path iconPath = std::filesystem::path(argv[2]);
	if (iconPath == "C:\\Windows\\System32\\shell32.dll,-4")
		return EXIT_SUCCESS;

	std::ofstream desktopIniFile(desktopIniPath);
	if (!desktopIniFile) 
		return error(L"Failed to create desktopIni file at " + desktopIniPath.wstring());
		
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

	refreshIcon(folderPath);
	return EXIT_SUCCESS;
}