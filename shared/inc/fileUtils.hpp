#pragma once

#include "filePath.h"

bool fileExists(const Path& path);
bool deleteFile(const Path& path);

bool isDirectory(const Path& path);
bool deleteDirectory(const Path& path);
bool createDirectory(const Path& path);