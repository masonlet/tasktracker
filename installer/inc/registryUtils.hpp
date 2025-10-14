#pragma once

#include <string>
#include <filesystem>
#include "systemUtils.hpp"

constexpr std::wstring_view EXE_NAME = L"TaskTracker.exe";
static const Path FILE_PATH = getProgramFilesPath() / "Task Tracker";

const Path EXE_PATH = FILE_PATH / EXE_NAME;
const std::wstring REGISTRY_PATH = L"Directory\\shell\\Task Tracker";

bool setRegistryValue(RegKey& hKey, const std::wstring& valueName, const std::wstring& value);
bool createRegistryKey(const HKEY hKey, const std::wstring& path, RegKey& hKeyHandle, DWORD& disposition);
bool keyExists(const std::wstring_view& path);
bool deleteKey(const std::wstring& path);

bool createTaskTrackerKeys();
bool deleteTaskTrackerKeys();

