#pragma once

#include "filePath.h"

#include <shlobj_core.h>

struct RegKey {
	HKEY hKey = nullptr;

	RegKey() = default;
	~RegKey() {
		if (hKey) RegCloseKey(hKey);
	}

	RegKey(const RegKey&) = delete;
	RegKey& operator=(const RegKey&) = delete;

	operator HKEY() const { return hKey; }

	HKEY* operator&() { return &hKey; }
};

struct SubKey {
	const std::wstring name;
	const Path path;
	const std::wstring iconPath;
};

struct COMInitializer {
	HRESULT result;
	COMInitializer() { result = CoInitializeEx(NULL, COINIT_MULTITHREADED); }
	~COMInitializer() { CoUninitialize(); }

	operator HRESULT() const { return result; }
};

Path getProgramFilesPath();
bool isAdmin();