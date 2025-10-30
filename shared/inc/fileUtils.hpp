#pragma once

#include "filePath.h"

namespace TaskTracker::FileUtils {
	bool fileExists(const Path& path, const std::wstring_view& caller, const bool verbose);
	bool deleteFile(const Path& path, const std::wstring_view& caller);

	bool isDirectory(const Path& path, const std::wstring_view& caller, const bool verbose);
	bool deleteDirectory(const Path& path, const std::wstring_view& caller);
	bool createDirectory(const Path& path, const std::wstring_view& caller);
}