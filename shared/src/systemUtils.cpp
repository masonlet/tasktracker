#include "systemUtils.hpp"
#include "fileUtils.hpp"
#include "log.hpp"

namespace TaskTracker::SystemUtils {
	Path getProgramFilesPath() {
		wchar_t* path{ nullptr };
		if (FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, nullptr, &path)))
			return L"C:\\Program Files\\";

		Path out(path);
		CoTaskMemFree(path);
		return out;
	}

	bool isAdmin() {
		HANDLE hToken{ nullptr };
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
			return Log::error(L"Installer", L"OpenProcessTokenFailed");

		TOKEN_ELEVATION elevation{};
		DWORD returnSize = sizeof(TOKEN_ELEVATION);
		GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &returnSize);

		if (hToken) CloseHandle(hToken);
		return elevation.TokenIsElevated;
	}

	namespace {
		void refreshExplorer(const Path& path) {
			//a lot of this is probably redundant, windows does not like refreshing and I do not like hard restarting explorer so it takes a long time
			system("ie4uinit.exe -ClearIconCache");

			SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW, path.c_str(), NULL);
			SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

			SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_PATHW | SHCNF_FLUSH, path.c_str(), NULL);
			SHChangeNotify(SHCNE_ATTRIBUTES, SHCNF_PATHW | SHCNF_FLUSH, path.c_str(), NULL);

			SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>(L"Environment"), SMTO_ABORTIFHUNG, 5000, NULL);

			SHFlushSFCache();

			system("ie4uinit.exe -show");
		}
	}

	int exitAndRefresh(const Path& path, const std::wstring_view& caller) {
		refreshExplorer(path);
		return Log::exitSuccess(caller, L"Refreshing Explorer cache and folder view for \"" + path.wstring() + L"\"");
	}

	bool isValidPath(const TaskTracker::Path& path, const std::wstring_view& caller) {
		if (!TaskTracker::FileUtils::fileExists(path, caller, true)
		 || !TaskTracker::FileUtils::isDirectory(path, caller, true)
		) return TaskTracker::Log::error(caller, L"Folder path \"" + path.wstring() + L"\" is invalid");

		return TaskTracker::Log::info(caller, L"Folder path \"" + path.wstring() + L"\" is valid");
	}

	bool deleteDesktopIni(const TaskTracker::Path& path, const std::wstring_view& caller) {
		if (FileUtils::fileExists(path, caller, true)) {
			SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
			if (!FileUtils::deleteFile(path.c_str(), caller))
				return TaskTracker::Log::error(caller, L"Failed to delete desktop.ini file");
		}

		return TaskTracker::Log::info(caller, L"Deleted desktop.ini file successfully");
	}
}