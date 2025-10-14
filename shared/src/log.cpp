#include "log.hpp"

#include <iostream>

bool log(const std::wstring_view& message, const bool pause, const bool error) {
	std::wcout << L"TaskTracker Installer [" << (error ? L"Error" : L"Info") << "]: " << message << L'\n';

	if (pause) {
		std::wcout << L"Press Enter to continue...";
		std::wcin.get();
	}

	return !error;
}
int error(const std::wstring_view& message) {
    log(message, true, true);
    return EXIT_FAILURE;
}

