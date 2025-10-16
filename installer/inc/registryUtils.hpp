#pragma once

#include "filePath.h"
#include "systemUtils.hpp"

#include <string>
#include <filesystem>

constexpr std::wstring_view EXE_NAME = L"TaskTracker.exe";
static const Path FILE_PATH = getProgramFilesPath() / "Task Tracker";

const Path EXE_PATH = FILE_PATH / EXE_NAME;
const std::wstring REGISTRY_PATH = L"Directory\\shell\\Task Tracker";

bool registryKeyExists(const std::wstring& path);
bool createRegistryKey(const HKEY hKey, const std::wstring& path, RegKey& hKeyHandle, DWORD& disposition);
bool deleteRegistryKey(const std::wstring& path);
bool setRegistryValue(RegKey& hKey, const std::wstring_view& valueName, const std::wstring& value);

bool createTaskTrackerKeys();
bool deleteTaskTrackerKeys();

