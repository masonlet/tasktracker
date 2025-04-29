#pragma comment(lib, "Shell32.lib")

/*
Program name: Installer
Purpose: Installs Task Tracker program to a Windows 10 / Windows 11 computer
Author: Mason L'Etoile
Date: April 29, 2025
Version: 1.0.1
*/

#include <Windows.h>
#include <Winreg.h>
#include<shlobj_core.h>
#include <filesystem>
#include <array>
#include <chrono>
#include <string>
#include <iostream>

constexpr std::wstring_view EXE_NAME = L"TaskTracker.exe";

static std::filesystem::path getProgramFilesPath() {
	wchar_t* path = nullptr;

	if (FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, &path))) {
		return L"C:\\Program Files\\";
	}

	std::filesystem::path result = path;
	CoTaskMemFree(path);
	return result;
}
static const std::filesystem::path FILE_PATH = getProgramFilesPath() / "Task Tracker";

const std::filesystem::path EXE_PATH = FILE_PATH / EXE_NAME;
const std::wstring REGISTRY_PATH   = L"Directory\\shell\\Task Tracker";

const std::wstring SHELL_PATH      = REGISTRY_PATH + L"\\shell";
const std::wstring FINISHED_PATH   = SHELL_PATH + L"\\Finished";
const std::wstring HIDDEN_PATH     = SHELL_PATH + L"\\Hidden";
const std::wstring UNFINISHED_PATH = SHELL_PATH + L"\\Unfinished";
const std::wstring DEFAULT_PATH    = SHELL_PATH + L"\\Default";

const std::wstring FINISHED_CMD_PATH   = FINISHED_PATH   + L"\\command";
const std::wstring HIDDEN_CMD_PATH     = HIDDEN_PATH     + L"\\command";
const std::wstring UNFINISHED_CMD_PATH = UNFINISHED_PATH + L"\\command";
const std::wstring DEFAULT_CMD_PATH    = DEFAULT_PATH    + L"\\command";

class RegKey {
	HKEY hKey = nullptr;

public:
	RegKey() = default;
    ~RegKey() { 
		if (hKey) RegCloseKey(hKey);
	}

	RegKey(const RegKey&) = delete;
	RegKey& operator=(const RegKey&) = delete;

	operator HKEY() const { 
		return hKey; 
	}

	HKEY* operator&() {
		return &hKey;
	}
};

static bool error(const std::wstring& error, bool pause = false) {
	std::wcout << L"TaskTracker Installer [Error]: " << error << L'\n';

	if (pause) {
		std::wcout << L"Press Enter to continue...";
		std::wcin.get();
	}
	return false;
}
static void message(const std::wstring& message, bool pause = false) {
	std::wcout << L"TaskTracker Installer [Info]: " << message << L'\n';

	if (pause) {
		std::wcout << L"Press Enter to continue...";
		std::wcin.get();
	}
}
static bool isAdmin() {
	bool isElevated = false;
	HANDLE hToken = nullptr;

	if( OpenProcessToken( GetCurrentProcess( ), TOKEN_QUERY, &hToken ) ) {
		TOKEN_ELEVATION elevation{};
        DWORD returnSize = sizeof( TOKEN_ELEVATION );

		if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &returnSize))
			isElevated = elevation.TokenIsElevated;
	} else
		return error(L"OpenProcessToken failed");
    if( hToken ) 
        CloseHandle( hToken );
    
    return isElevated;
}

static bool keyExists(const std::filesystem::path& path) {
	RegKey hKey;

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT, path.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return false;

	return true;
}
static bool fileExists(const std::filesystem::path& path) {
	return std::filesystem::exists(path);
}

static bool deleteFile(const std::filesystem::path& path) {
	if (fileExists(path))
		if (!std::filesystem::remove(path))
			return error(L"Failed to remove file at " + path.wstring());

	message(L"File Removed at " + path.wstring());
	return !std::filesystem::exists(path);
}
static bool deleteDirectory(const std::filesystem::path& path) {
	if (fileExists(path))
		if (!std::filesystem::remove_all(path))
			return error(L"Failed to remove directory at " + path.wstring());

	message(L"Directory Removed at " + path.wstring());
	return !std::filesystem::exists(path);
}

static bool deleteKey(const std::filesystem::path& path) {
	if (RegDeleteKeyW(HKEY_CLASSES_ROOT, path.c_str()) != ERROR_SUCCESS) {
		message(L"Failed to remove key at " + path.wstring());
		return false;
	}

	return true;
}
static bool deleteSubkeys(const std::filesystem::path* subkeyPaths, const size_t& count) {
	bool success = true;

	for (size_t i = 0; i < count; i++) {
		if (RegDeleteKeyW(HKEY_CLASSES_ROOT, subkeyPaths[i].c_str()) != ERROR_SUCCESS) {
			message(L"Failed to remove key at " + subkeyPaths[i].wstring());
			success = false;
		}
	}

	return success;
}
static bool deleteRegistryKeys() {
	const std::filesystem::path subkeyCmdPaths[] = { FINISHED_CMD_PATH, HIDDEN_CMD_PATH, UNFINISHED_CMD_PATH, DEFAULT_CMD_PATH };
	const std::filesystem::path subkeyPaths[] = { FINISHED_PATH, HIDDEN_PATH, UNFINISHED_PATH, DEFAULT_PATH };
	bool success = true;
	
	success = deleteSubkeys(subkeyCmdPaths, std::size(subkeyCmdPaths));
	success = deleteSubkeys(subkeyPaths, std::size(subkeyPaths));
	
	success = deleteKey(SHELL_PATH);
	success = deleteKey(REGISTRY_PATH);

	if (success) message(L"Program Keys Removed");
	else message(L"Partial / Entire failure of program key removal");

	return success;
}

static bool createKey(HKEY hKey, const std::wstring& path, RegKey& hKeyHandle, DWORD disposition) {
	LONG result = RegCreateKeyExW(hKey, path.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKeyHandle, &disposition);
	if (result != ERROR_SUCCESS) 
		return error(L"Failed to create registry key " + std::to_wstring(result) + L" at " + path);
	
	return true;
}
static bool setValue(RegKey& hKey, const std::wstring& valueName, const std::wstring& value) {
	LONG result = RegSetValueExW(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
	if (result != ERROR_SUCCESS)
		return error(L"Failed to set registry value " + std::to_wstring(result));

	return true;
}
static bool createRegistryKeys() {
	RegKey hKey;
	DWORD disposition{};

	//Base key
	if (!createKey(HKEY_CLASSES_ROOT, REGISTRY_PATH, hKey, disposition))
		return error(L"Failed to create Main Registry key");

	//Base Key Values
	if (!setValue(hKey, L"Icon", L"shell32.dll,-4") || !setValue(hKey, L"MUIVerb", L"Task Tracker") || !setValue(hKey, L"ExtendedSubCommandsKey", REGISTRY_PATH))
		return error(L"Failed to set Registry key values");

	struct SubKey {
		const std::wstring name;
		const std::filesystem::path path;
		const std::wstring iconPath;
	};
	const std::array<SubKey, 4> subCommands = {
		SubKey{L"Finished", FINISHED_PATH, L"C:\\Windows\\System32\\shell32.dll,-16810"},
		SubKey{L"Hidden", HIDDEN_PATH, L"C:\\Windows\\System32\\shell32.dll,-200"},
		SubKey{L"Unfinished", UNFINISHED_PATH, L"C:\\Windows\\System32\\shell32.dll,-240"},
		SubKey{L"Default", DEFAULT_PATH, L"C:\\Windows\\System32\\shell32.dll,-4"}
	};

	for (const SubKey& subCommand : subCommands) {
		RegKey subKey, folderKey;

		//Sub Key
		if (!createKey(HKEY_CLASSES_ROOT, subCommand.path, subKey, disposition))
			return error(L"Failed to create Registry subkey: " + subCommand.path.wstring());

		//Sub Key Values
		if (!setValue(subKey, L"MUIVerb", subCommand.name) ||
			!setValue(subKey, L"Icon", subCommand.iconPath))
			return error(L"Failed to set Registry subkey " + subCommand.path.wstring() + L" values");

		//Sub Folder
		std::filesystem::path folderPath = subCommand.path / "command";
		if (!createKey(HKEY_CLASSES_ROOT, folderPath, folderKey, disposition)) 
			return error(L"Failed to create Registry subkey " + folderPath.wstring());	

		//Sub Key Command
		const std::wstring cmd = L"\"" + EXE_PATH.wstring() + L"\" \"%V\" \"" + subCommand.iconPath + L"\"";
		if (!setValue(folderKey, L"", cmd))
			return error(L"Failed to set Registry subkey " + folderPath.wstring() + L" command");
	}	

	message(L"Program Keys Added");
	return true;
}

static bool createDirectory(const std::filesystem::path& path) {
	if (!std::filesystem::exists(path) && !std::filesystem::create_directories(path))
		return error(L"Failed to create TaskTracker folder at" + path.wstring());

	message(L"Folder added at " + path.wstring());
	return true;
}
static bool copyFile(const std::wstring& name, const std::filesystem::path& path) {
	try {
		std::filesystem::copy_file(name, path.wstring(), std::filesystem::copy_options::overwrite_existing);
	} catch (const std::filesystem::filesystem_error) {
		return error(L"Failed to copy executables to " + path.wstring());
	}

	message(L"Exe copied to " + FILE_PATH.wstring() + std::wstring(EXE_NAME));
	return true;
}

int main() {
	if (!isAdmin()) 
		return error(L"Program has not been run as an administrator", true);

	if (keyExists(REGISTRY_PATH) || fileExists(FILE_PATH)) {	
		if (keyExists(REGISTRY_PATH)) 
			if (!deleteRegistryKeys())
				return error(L"Failed to delete keys", true);
		
		message(L"Keys successfully removed");

		if(fileExists(EXE_PATH))
			if (!deleteFile(EXE_PATH)) 
				return error(L"Failed to delete executable", true);

		if (fileExists(FILE_PATH)) {
			if (!deleteDirectory(FILE_PATH))
				return error(L"Failed to remove file", true);
		}

		message(L"File successfully removed");
		message(L"Uninstallation Completed", true);
	} else {
		if (!createRegistryKeys() || 
			!createDirectory(FILE_PATH) || 
			!copyFile(std::wstring(EXE_NAME), EXE_PATH))
			return error(L"Failed to install", true);

		message(L"Installation Completed", true);
	}

	return EXIT_SUCCESS;
}