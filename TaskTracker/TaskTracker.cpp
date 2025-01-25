/*
Program name: Task Tracker
Purpose: Adds three new buttons to the windows right click menu: Finished, Unfinished, and Hidden
Author: Mason L'Etoile
Date: January 24, 2025
*/

#include <iostream>
#include <Windows.h>

const char* regPath = R"(Directory\shell\TaskTracker)";
const char* cmdPath = R"(Directory\shell\TaskTracker\command)";

bool cmdExists() {
	HKEY hKey;

	if (RegOpenKeyExA(HKEY_CLASSES_ROOT, regPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return true;
	}

	return false;
}

void removeCmd() {
	RegDeleteKeyA(HKEY_CLASSES_ROOT, cmdPath);
	RegDeleteKeyA(HKEY_CLASSES_ROOT, regPath);
	std::cout << "Command Removed\n";
}

void addCmd() {
	HKEY hKey;
	DWORD disposition;

	if (RegCreateKeyExA(HKEY_CLASSES_ROOT, regPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &disposition) == ERROR_SUCCESS) {

		std::cout << "Command Added\n";
	} else {
		std::cout << "Error creating registry key\n";
	}
}

int main() {
	if (cmdExists()) {	
		removeCmd();
	}
	else {
		addCmd();
	}
}