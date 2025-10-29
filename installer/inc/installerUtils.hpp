#pragma once

#include "filePath.h"

namespace TaskTracker::Installer {
	bool isTaskTrackerInstalled();

	bool extractTaskTrackerExe(const Path& toPath);
	int installTaskTracker();

	int deleteTaskTracker();
}
