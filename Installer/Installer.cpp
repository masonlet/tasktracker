#pragma comment(lib, "Shell32.lib")

/*
Program name: Installer
Purpose: Installs Task Tracker program to a Windows 10 / Windows 11 computer
Author: Mason L'Etoile
Date: January 26, 2025
*/
#include <filesystem>
#include <iostream>
#include <Windows.h>
#include <Winreg.h>
#include <array>

const std::filesystem::path EXE_PATH		= "C:\\Program Files\\Task Tracker";

const std::filesystem::path REG_PATH		= "Directory\\shell\\TaskTracker";
const std::filesystem::path SHELL_PATH      = REG_PATH / "shell";

const std::filesystem::path FINISHED_PATH   = SHELL_PATH / "Finished";
const std::filesystem::path HIDDEN_PATH     = SHELL_PATH / "Hidden";
const std::filesystem::path UNFINISHED_PATH = SHELL_PATH / "Unfinished";
const std::filesystem::path DEFAULT_PATH    = SHELL_PATH / "Default";

const std::filesystem::path FINISHED_CMD   = FINISHED_PATH   / "command";
const std::filesystem::path HIDDEN_CMD     = HIDDEN_PATH     / "command";
const std::filesystem::path UNFINISHED_CMD = UNFINISHED_PATH / "command";
const std::filesystem::path DEFAULT_CMD    = DEFAULT_PATH    / "command";

static bool keyExists(const std::filesystem::path& path) {
	HKEY hKey;

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT, path.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return true;
	}

	return false;
}
static void removeKey() {
	const std::filesystem::path paths[] = { FINISHED_PATH, HIDDEN_PATH, UNFINISHED_PATH, DEFAULT_PATH };
	const std::filesystem::path cmdPaths[] = { FINISHED_CMD, HIDDEN_CMD, UNFINISHED_CMD, DEFAULT_CMD };

	for (size_t i = 0; i < 4; i++) {
		if (keyExists(paths[i])) {
			if (keyExists(cmdPaths[i]))
				RegDeleteKeyW(HKEY_CLASSES_ROOT, cmdPaths[i].c_str());

			RegDeleteKeyW(HKEY_CLASSES_ROOT, paths[i].c_str());
		}
	}

	if (keyExists(SHELL_PATH)) 
		RegDeleteKeyW(HKEY_CLASSES_ROOT, SHELL_PATH.c_str());
	
	if (keyExists(REG_PATH)) 
		RegDeleteKeyW(HKEY_CLASSES_ROOT, REG_PATH.c_str());

	std::cout << "Command Removed\n";
}
static void removeFile() {
	std::filesystem::remove_all(EXE_PATH);

	std::cout << "File Removed\n";
}

static bool error(const std::string& error, bool removeItems = true) {
	std::cout << "\nTaskTracker Installer [Error]: " << error << "\n\n";

	if (removeItems) {
		removeKey();
		removeFile();
	}

	system("pause");
	return false;
}

struct SubKey {
	std::wstring name;
	std::filesystem::path path;
	std::wstring iconPath;
};

static bool createKey(const std::filesystem::path& path, HKEY& hKey, DWORD& disposition) {
	LONG result = RegCreateKeyExW(HKEY_CLASSES_ROOT, path.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, &disposition);
	if (result == ERROR_SUCCESS) 
		return true;

	return error("Creating registry key: " + result);
}
static bool setValue(HKEY& hKey, const std::wstring& valueName, const std::wstring& value) {
	LONG result = RegSetValueExW(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), static_cast<DWORD>(value.size() + 1) * sizeof(wchar_t));
	if (result == ERROR_SUCCESS)
		return true;

	return error("Setting registry value: " + result);
}
static bool createKeys() {
	HKEY hKey;
	DWORD disposition;

	//Base key
	if (!createKey(REG_PATH, hKey, disposition)) 
		return error("Creating Registry key");

	if (!setValue(hKey, L"Icon", L"shell32.dll,-4") ||
		!setValue(hKey, L"MUIVerb", L"Task Tracker") ||
		!setValue(hKey, L"ExtendedSubCommandsKey", REG_PATH))
		return error("Setting Registry key values");

	//Sub keys
	const std::array<SubKey, 4> subCommands = {
		SubKey{L"Finished", FINISHED_PATH, L"C:\\Windows\\System32\\shell32.dll,-16810"},
		SubKey{L"Hidden", HIDDEN_PATH, L"C:\\Windows\\System32\\shell32.dll,-200"},
		SubKey{L"Unfinished", UNFINISHED_PATH, L"C:\\Windows\\System32\\shell32.dll,-240"},
		SubKey{L"Default", DEFAULT_PATH, L"C:\\Windows\\System32\\shell32.dll,-4"}
	};

	for (const SubKey& subCommand : subCommands) {
		HKEY subKey, folderKey;

		//Sub Key
		if (!createKey(subCommand.path, subKey, disposition)) 
			return error("Creating Registry sub key: " + subCommand.path.string());

		setValue(subKey, L"MUIVerb", subCommand.name);
		setValue(subKey, L"Icon", subCommand.iconPath);

		//Sub Folder
		std::filesystem::path folderPath = subCommand.path / "command";
		if (!createKey(folderPath, folderKey, disposition)) 
			return error("Creating Registry key");	

		//Sub Key Command
		const std::wstring cmd = EXE_PATH.wstring() + L"\\TaskTracker.exe \"%V\" \"" + subCommand.iconPath + L"\"";
		setValue(folderKey, L"", cmd);

		//Cleanup
		RegCloseKey(folderKey);
		RegCloseKey(subKey);
	}	

	//Cleanup
	RegCloseKey(hKey);
	std::cout << "Command Added\n";
	return true;
}

static bool fileExists(const std::filesystem::path& path) {
	if (std::filesystem::exists(path))
		return true;

	return false;
}
static bool createFile() {
	if (!std::filesystem::create_directories(EXE_PATH))
		return error("Error creating TaskTracker folder at" + EXE_PATH.string());

	std::cout << "Exe folder added at " + EXE_PATH.string() + '\n';

	try {
		std::filesystem::copy_file("TaskTracker.exe", EXE_PATH.wstring() + L"\\TaskTracker.exe", std::filesystem::copy_options::overwrite_existing);
	} catch (const std::filesystem::filesystem_error& e) {
		return error("addFile: " + std::string(e.what()));
	}

	std::cout << "Exe copied to " + EXE_PATH.string() + "\\TaskTracker.exe" + '\n';

	return true;
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

int main() {
	if (!isAdmin())
		return error("Program has not been run as an adminstrator", false);

	if (keyExists(REG_PATH) || fileExists(EXE_PATH)) {	
		removeKey();
		removeFile();
	}
	else {
		createKeys();
		createFile();
	}
	system("pause");
}