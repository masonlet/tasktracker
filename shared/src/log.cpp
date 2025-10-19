#include "log.hpp"

#include <iostream>

bool logInfo(const std::wstring_view& message, const bool ret) {
	std::wcout << L"TaskTracker Installer [Info]: " << message << L'\n';
	return ret;
}
bool logError(const std::wstring_view& message, const bool pause) {
	std::wcout << L"TaskTracker Installer [Error]: " << message << L'\n';

	if (pause) {
		std::wcout << L"Press Enter to continue...";
		std::wcin.get();
	}

	return false;
}

int success(const std::wstring_view& message) {
	logInfo(message);
	return EXIT_SUCCESS;
}
int error(const std::wstring_view& message) {
  logError(message, true);
  return EXIT_FAILURE;
}

