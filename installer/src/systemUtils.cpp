#include "systemUtils.hpp"
#include "log.hpp"

std::filesystem::path getProgramFilesPath() {
	wchar_t* path = nullptr;

	if (FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, &path))) {
		return L"C:\\Program Files\\";
	}

	CoTaskMemFree(path);
	return std::filesystem::path(path);
}

bool isAdmin() {
	bool isElevated{ false };
	HANDLE hToken{ nullptr };

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION elevation{};
		DWORD returnSize = sizeof(TOKEN_ELEVATION);

		if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &returnSize))
			isElevated = elevation.TokenIsElevated;
	}
	else return error(L"OpenProcessToken failed");

	if (hToken) CloseHandle(hToken);
	return isElevated;
}