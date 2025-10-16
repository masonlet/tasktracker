#pragma once

#include "filePath.h"

bool isTaskTrackerInstalled();
bool extractTaskTrackerExe(const Path& toPath);

int installTaskTracker();
int deleteTaskTracker();