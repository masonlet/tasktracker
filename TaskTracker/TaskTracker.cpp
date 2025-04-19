#pragma comment(lib, "Shell32.lib")

/*
Program name: Task Tracker
Purpose: Changes a folders icon to the selected buttons icon
Author: Mason L'Etoile
Date: January 26, 2025
*/

#include <iostream>
#include <filesystem>
#include <fstream>
#include <shlobj.h>

static int error(const std::string& error) {
	std::cout << "TaskTracker [Error]: " << error << '\n';

	system("pause");
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

int main(int argc, char* argv[]){
	if (!isAdmin())
		return error("Program has not been run as an administrator");

	if (argc != 3) 
		return error("Invalid argument amount\nUsage: TaskTracker.exe <folder_path> <icon_path>\n");

	std::filesystem::path folderPath = std::filesystem::path(argv[1]);
	if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath)) 
		return error("FolderPath: " + folderPath.string() + " is invalid");

	std::filesystem::path iconPath = std::filesystem::path(argv[2]);
	
	std::filesystem::path desktopIniPath = folderPath / "desktop.ini";
	if (std::filesystem::exists(desktopIniPath)) {
		SetFileAttributesW(desktopIniPath.c_str(), FILE_ATTRIBUTE_NORMAL);
		DeleteFileW(desktopIniPath.c_str());
		refreshIcon(folderPath);
	} 

	if (iconPath == "C:\\Windows\\System32\\shell32.dll,-4")
		return EXIT_SUCCESS;

	std::ofstream desktopIni(desktopIniPath);
	if (!desktopIni) 
		return error("Error creating desktopIni file at " + desktopIniPath.string());
		
	desktopIni << "[.ShellClassInfo]\n";
	desktopIni << "IconResource=" << iconPath.string() << '\n';
	desktopIni << "[ViewState]\n";
	desktopIni << "Mode=\n";
	desktopIni << "Vid=\n";
	desktopIni << "FolderType=Generic\n";
	desktopIni.close();

	if (!SetFileAttributesW(folderPath.c_str(), FILE_ATTRIBUTE_SYSTEM)) 
		return error("Setting folder attributes");

	if (!SetFileAttributesW(desktopIniPath.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) 
		return error("Setting desktop.ini attributes");

	refreshIcon(folderPath);
}