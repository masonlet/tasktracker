#pragma comment(lib, "Shell32.lib")

#include <Windows.h>
#include <Winreg.h>
#include <shlobj_core.h>
#include <filesystem>
#include <array>
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include "resource.h"

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

/* Helper Functions */
static bool error(const std::wstring_view& error, bool pause = false) {
	std::wcout << L"TaskTracker Installer [Error]: " << error << L'\n';

	if (pause) {
		std::wcout << L"Press Enter to continue...";
		std::wcin.get();
	}
	return false;
}
static void message(const std::wstring_view& message, bool pause = false) {
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

/* Registry Functions */
struct RegKey {
	HKEY hKey = nullptr;

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

static bool keyExists(const std::wstring_view& path) {
	RegKey hKey;

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT, path.data(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return false;

	return true;
}

static bool deleteKey(const std::wstring& path) {
	if (RegDeleteTreeW(HKEY_CLASSES_ROOT, path.c_str()) != ERROR_SUCCESS) {
		message(L"Failed to remove key at " + path);
		return false;
	}
	
	return true;
}

static bool createRegistryKey(HKEY hKey, const std::wstring& path, RegKey& hKeyHandle, DWORD disposition) {
	LONG result = RegCreateKeyExW(hKey, path.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKeyHandle, &disposition);
	if (result != ERROR_SUCCESS) 
		return error(L"Failed to create registry key " + std::to_wstring(result) + L" at " + path);
	
	return true;
}
static bool setRegistryValue(RegKey& hKey, const std::wstring& valueName, const std::wstring& value) {
	LONG result = RegSetValueExW(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
	if (result != ERROR_SUCCESS)
		return error(L"Failed to set registry value " + std::to_wstring(result));

	return true;
}

/* File Functions */
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
	if (fileExists(path)) {
		if (!std::filesystem::remove_all(path))
			return error(L"Failed to remove directory at " + path.wstring());

		message(L"Directory Removed at " + path.wstring());
	}

	return !std::filesystem::exists(path);
}

static bool createDirectory(const std::filesystem::path& path) {
	if (!std::filesystem::exists(path) && !std::filesystem::create_directories(path))
		return error(L"Failed to create TaskTracker folder at " + path.wstring());

	message(L"Folder added at " + path.wstring());
	return true;
}
static bool extractTaskTrackerExe(const std::filesystem::path& toPath) {
	HMODULE hModule = GetModuleHandle(NULL);
	if (!hModule) return error(L"Failed to get module handle");

	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(TASKTRACKER_EXE), RT_RCDATA);
	if (!hResource) return error(L"Failed to find TaskTracker.exe resource");

	HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
	if (!hLoadedResource) return error(L"Failed to load TaskTracker.exe resource");

	const void* pResourceData = LockResource(hLoadedResource);
	if (!pResourceData) return error(L"Failed to lock TaskTracker.exe resource");

	DWORD resourceSize = SizeofResource(hModule, hResource);
	if (resourceSize == 0) return error(L"TaskTracker.exe resource has zero size");

	std::ofstream outFile(toPath, std::ios::binary);
	if (!outFile) return error(L"Failed to create file at " + toPath.wstring());

	outFile.write(static_cast<const char*>(pResourceData), resourceSize);
	outFile.close();
	if (!outFile.good())
		return error(L"Failed to write TaskTracker.exe to " + toPath.wstring());

	return true;
}

/* Installer */
static bool createTasktrackerKeys() {
	RegKey hKey;
	DWORD disposition{};

	//Base key
	if (!createRegistryKey(HKEY_CLASSES_ROOT, REGISTRY_PATH, hKey, disposition))
		return error(L"Failed to create Main Registry key");

	//Base Key Values
	if (!setRegistryValue(hKey, L"Icon", L"shell32.dll,-4") || !setRegistryValue(hKey, L"MUIVerb", L"Task Tracker") || !setRegistryValue(hKey, L"ExtendedSubCommandsKey", REGISTRY_PATH))
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
		if (!createRegistryKey(HKEY_CLASSES_ROOT, subCommand.path, subKey, disposition))
			return error(L"Failed to create Registry subkey: " + subCommand.path.wstring());

		//Sub Key Values
		if (!setRegistryValue(subKey, L"MUIVerb", subCommand.name) ||
			!setRegistryValue(subKey, L"Icon", subCommand.iconPath))
			return error(L"Failed to set Registry subkey " + subCommand.path.wstring() + L" values");

		//Sub Folder
		std::filesystem::path folderPath = subCommand.path / "command";
		if (!createRegistryKey(HKEY_CLASSES_ROOT, folderPath, folderKey, disposition)) 
			return error(L"Failed to create Registry subkey " + folderPath.wstring());	

		//Sub Key Command
		const std::wstring cmd = L"\"" + EXE_PATH.wstring() + L"\" \"%V\" \"" + subCommand.iconPath + L"\"";
		if (!setRegistryValue(folderKey, L"", cmd))
			return error(L"Failed to set Registry subkey " + folderPath.wstring() + L" command");
	}	

	message(L"Program Keys Added");
	return true;
}
static bool deleteTasktrackerKeys() {
	bool success = true;
	
	success &= deleteKey(SHELL_PATH);
	success &= deleteKey(REGISTRY_PATH);

	if (success) message(L"Program Keys Removed");
	else message(L"Partial / Entire failure of program key removal");

	return success;
}

struct COMInitializer {
	HRESULT result;
	COMInitializer() { result = CoInitializeEx(NULL, COINIT_MULTITHREADED); }
	~COMInitializer() { CoUninitialize(); }

	operator HRESULT() const {
		return result;
	}
};
int wmain() {
	COMInitializer comInitializer;
	if (FAILED(comInitializer))
		return error(L"Failed to initialize COM library");

	if (!isAdmin()) {
		MessageBoxW(NULL, L"Must be ran as adminstrator", L"Adminstrator Required", MB_OK | MB_ICONERROR);
		return error(L"Program has not been run as an administrator", true);
	}

	if (keyExists(REGISTRY_PATH) || fileExists(FILE_PATH)) {	
		if (keyExists(REGISTRY_PATH) && !deleteTasktrackerKeys()) 
			return error(L"Failed to delete keys", true);
		else 
			message(L"Keys successfully removed");

		if (fileExists(EXE_PATH) && !deleteFile(EXE_PATH))
			return error(L"Failed to delete executable", true);
		else
			message(L"Executable successfully deleted");

		if (fileExists(FILE_PATH) && !deleteDirectory(FILE_PATH)) 
			return error(L"Failed to remove file", true);
		else 
			message(L"File successfully removed");

		message(L"Uninstallation Completed", true);
	} else {
		if (!createTasktrackerKeys() || !createDirectory(FILE_PATH) || !extractTaskTrackerExe(EXE_PATH))
			return error(L"Failed to install", true);

		message(L"Installation Completed", true);
	}

	return EXIT_SUCCESS;
}