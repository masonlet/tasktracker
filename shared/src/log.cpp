#include "log.hpp"

#include <iostream>

namespace TaskTracker::Log {
	bool info(const std::wstring_view& message, const bool ret) {
		std::wcout << L"TaskTracker Installer [Info]: " << message << L'\n';
		return ret;
	}
	bool error(const std::wstring_view& message, const bool pause) {
		std::wcout << L"TaskTracker Installer [Error]: " << message << L'\n';

		if (pause) {
			std::wcout << L"Press Enter to continue...";
			std::wcin.get();
		}

		return false;
	}

	int exitSuccess(const std::wstring_view& message) {
		info(message);
		return EXIT_SUCCESS;
	}
	int exitError(const std::wstring_view& message) {
		error(message, true);
		return EXIT_FAILURE;
	}
}