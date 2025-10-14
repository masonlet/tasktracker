#include "systemUtils.hpp"
#include "log.hpp"

Path getProgramFilesPath() {
	wchar_t* path{ nullptr };

	if (FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, nullptr, &path))) 
		return L"C:\\Program Files\\";

	CoTaskMemFree(path);
	return Path(path);
}

bool isAdmin() {
	bool isElevated{ false };
	HANDLE hToken{ nullptr };

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		error(L"OpenProcessToken failed");
		return false;
	}

	TOKEN_ELEVATION elevation{};
	DWORD returnSize = sizeof(TOKEN_ELEVATION);

	if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &returnSize))
		isElevated = elevation.TokenIsElevated;

	if (hToken) CloseHandle(hToken);
	return isElevated;
}