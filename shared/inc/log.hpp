#pragma once

#include <string>

namespace TaskTracker::Log {
	bool info(const std::wstring_view& message, const bool ret = true);
	bool error(const std::wstring_view& message, const bool pause = false);

	int exitSuccess(const std::wstring_view& message);
	int exitError(const std::wstring_view& message);
}