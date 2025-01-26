/*
Program name: Task Tracker
Purpose: Adds three new buttons to the windows right click menu: Finished, Unfinished, and Hidden
Author: Mason L'Etoile
Date: January 24, 2025
*/

#include <iostream>
#include <Windows.h>

const char* REG_PATH = R"(Directory\shell\TaskTracker)";
const char* SHELL_PATH = R"(Directory\shell\TaskTracker\shell)";

const char* FINISHED_PATH = R"(Directory\shell\TaskTracker\shell\Finished)";
const char* HIDDEN_PATH = R"(Directory\shell\TaskTracker\shell\Hidden)";
const char* UNFINISHED_PATH = R"(Directory\shell\TaskTracker\shell\Unfinished)";
const char* DEFAULT_PATH = R"(Directory\shell\TaskTracker\shell\Default)";

const char* FINISHED_CMD = R"(Directory\shell\TaskTracker\shell\Finished\command)";
const char* HIDDEN_CMD = R"(Directory\shell\TaskTracker\shell\Hidden\command)";
const char* UNFINISHED_CMD = R"(Directory\shell\TaskTracker\shell\Unfinished\command)";
const char* DEFAULT_CMD = R"(Directory\shell\TaskTracker\shell\Default\command)";

bool cmdExists(const char* path) {
	HKEY hKey;

	if (RegOpenKeyExA(HKEY_CLASSES_ROOT, path, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return true;
	}

	return false;
}

void removeKey() {
	const char* paths[] = { FINISHED_PATH, HIDDEN_PATH, UNFINISHED_PATH, DEFAULT_PATH };
	const char* cmdPaths[] = { FINISHED_CMD, HIDDEN_CMD, UNFINISHED_CMD, DEFAULT_CMD };

	for (size_t i = 0; i < 4; i++) {
		if (cmdExists(paths[i])) {
			if (cmdExists(cmdPaths[i])) RegDeleteKeyA(HKEY_CLASSES_ROOT, cmdPaths[i]);
			RegDeleteKeyA(HKEY_CLASSES_ROOT, paths[i]);
		}
	}

	if (cmdExists(SHELL_PATH)) RegDeleteKeyA(HKEY_CLASSES_ROOT, SHELL_PATH);
	
	RegDeleteKeyA(HKEY_CLASSES_ROOT, REG_PATH);
	std::cout << "Command Removed\n";
}

bool createKey(const char* path, HKEY& hKey, DWORD& disposition) {
	LONG result = RegCreateKeyExA(HKEY_CLASSES_ROOT, path, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &disposition);
	if (result == ERROR_SUCCESS) return true;

	std::cout << "Error creating registry key, error: " << result << '\n';
	return false;
}

bool setValue(HKEY& hKey, const char* valueName, const char* value) {
	LONG result = RegSetValueExA(hKey, valueName, 0, REG_SZ, (const BYTE*)value, static_cast<DWORD>(strlen(value) + 1));
	if (result == ERROR_SUCCESS) return true;

	std::cout << "Error setting registry value, Error: " << result << '\n';
	return false;
}

void addKey() {
	HKEY hKey;
	DWORD disposition;

	//Base key
	if (!createKey(REG_PATH, hKey, disposition)) return;
	setValue(hKey, "Icon", "shell32.dll,-4");
	setValue(hKey, "MUIVerb", "Task Tracker");
	setValue(hKey, "ExtendedSubCommandsKey", REG_PATH);

	//Create subkeys
	const char* subPaths[] = { FINISHED_PATH, HIDDEN_PATH, UNFINISHED_PATH, DEFAULT_PATH };
	const char* subCommands[] = { "Finished", "Hidden", "Unfinished", "Default" };
	const std::string subIcons[] = { "C:\\Windows\\System32\\shell32.dll,-16810", "C:\\Windows\\System32\\shell32.dll,-200", "C:\\Windows\\System32\\shell32.dll,-240", "C:\\Windows\\System32\\shell32.dll,-4" };

	for (size_t i = 0; i < 4; i++) {
		HKEY subKey, folderKey;

		//Sub Key
		if (!createKey(subPaths[i], subKey, disposition)) return;
		setValue(subKey, "MUIVerb", subCommands[i]);
		setValue(subKey, "Icon", subIcons[i].c_str());

		//Sub Folder
		std::string folderPath = std::string(subPaths[i]) + "\\command";
		if (!createKey(folderPath.c_str(), folderKey, disposition)) return;	

		//Sub Key Command
		std::string cmd = R"(cmd.exe /c (echo [.ShellClassInfo] & echo IconResource=")" + subIcons[i] + R"(",0) > "%1\desktop.ini" && attrib +h +s "%1\desktop.ini" && ie4uinit.exe -show)";

		setValue(folderKey, NULL, cmd.c_str());

		//Cleanup
		RegCloseKey(folderKey);
		RegCloseKey(subKey);
	}	

	//Cleanup
	RegCloseKey(hKey);
	std::cout << "Command Added\n";
}

int main() {
	if (cmdExists(REG_PATH)) {	
		removeKey();
	}
	else {
		addKey();
	}
	system("pause");
}