#pragma once

#include <shlobj_core.h>
#include <filesystem>

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

struct COMInitializer {
	HRESULT result;
	COMInitializer() { result = CoInitializeEx(NULL, COINIT_MULTITHREADED); }
	~COMInitializer() { CoUninitialize(); }

	operator HRESULT() const { return result; }
};

std::filesystem::path getProgramFilesPath();
bool isAdmin();