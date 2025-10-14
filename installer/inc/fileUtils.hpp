#pragma once

#include <filesystem>

bool fileExists(const std::filesystem::path& path);
bool deleteFile(const std::filesystem::path& path);
bool deleteDirectory(const std::filesystem::path& path);
bool createDirectory(const std::filesystem::path& path);
bool extractTaskTrackerExe(const std::filesystem::path& toPath);