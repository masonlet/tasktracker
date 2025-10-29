#pragma once

#include "filePath.h"

namespace TaskTracker::FileUtils {
	bool fileExists(const Path& path, const bool verbose);
	bool deleteFile(const Path& path);

	bool isDirectory(const Path& path, const bool verbose);
	bool deleteDirectory(const Path& path);
	bool createDirectory(const Path& path);
}