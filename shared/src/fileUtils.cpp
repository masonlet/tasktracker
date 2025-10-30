#include "fileUtils.hpp"
#include "log.hpp"

#include <fstream>

namespace TaskTracker::FileUtils {
	bool fileExists(const Path& path, const std::wstring_view& caller, const bool verbose) {
		return std::filesystem::exists(path)
			? verbose
			? Log::info(caller, L"File found at " + path.wstring())
			: true
			: verbose
			? Log::info(caller, L"File not found at " + path.wstring(), false)
			: false;
	}
	bool deleteFile(const Path& path, const std::wstring_view& caller) {
		if (!fileExists(path, caller, false))
			return Log::info(caller, L"No file to remove at " + path.wstring());

		if (!std::filesystem::remove(path))
			return Log::error(caller, L"Failed to remove file at " + path.wstring());

		return fileExists(path, caller, false)
			? Log::error(caller, L"Failed to remove file at " + path.wstring())
			: Log::info(caller, L"File Removed at " + path.wstring());
	}

	bool isDirectory(const Path& path, const std::wstring_view& caller, const bool verbose) {
		return std::filesystem::is_directory(path)
			? verbose
			? Log::info(caller, L"Directory found at " + path.wstring())
			: true
			: verbose
			? Log::info(caller, L"Directory not found at " + path.wstring(), false)
			: false;
	}
	bool deleteDirectory(const Path& path, const std::wstring_view& caller) {
		if (!fileExists(path, caller, false))
			return Log::info(caller, L"No directory to remove at " + path.wstring());

		if (!std::filesystem::remove_all(path))
			return Log::error(caller, L"Failed to remove directory at " + path.wstring());

		return isDirectory(path, caller, false)
			? Log::error(caller, L"Failed to remove directory at " + path.wstring())
			: Log::info(caller, L"Directory Removed at " + path.wstring());
	}
	bool createDirectory(const Path& path, const std::wstring_view& caller) {
		if (!fileExists(path, caller, true) && !std::filesystem::create_directories(path))
			return Log::error(caller, L"Failed to create TaskTracker folder at " + path.wstring());

		return fileExists(path, caller, false)
			? Log::info(caller, L"Folder added at " + path.wstring())
			: Log::error(caller, L"Failed to create folder at " + path.wstring());
	}
}