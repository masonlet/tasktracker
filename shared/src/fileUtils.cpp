#include "fileUtils.hpp"
#include "log.hpp"

#include <fstream>

namespace TaskTracker::FileUtils {
	bool fileExists(const Path& path, const bool verbose) {
		return std::filesystem::exists(path)
			? verbose
			? Log::info(L"File found at " + path.wstring())
			: true
			: verbose
			? Log::info(L"File not found at " + path.wstring(), false)
			: false;
	}
	bool deleteFile(const Path& path) {
		if (!fileExists(path, false))
			return Log::info(L"No file to remove at " + path.wstring());

		if (!std::filesystem::remove(path))
			return Log::error(L"Failed to remove file at " + path.wstring());

		return fileExists(path, false)
			? Log::error(L"Failed to remove file at " + path.wstring())
			: Log::info(L"File Removed at " + path.wstring());
	}

	bool isDirectory(const Path& path, const bool verbose) {
		return std::filesystem::is_directory(path)
			? verbose
			? Log::info(L"Directory found at " + path.wstring())
			: true
			: verbose
			? Log::info(L"Directory not found at " + path.wstring(), false)
			: false;
	}
	bool deleteDirectory(const Path& path) {
		if (!fileExists(path, false))
			return Log::info(L"No directory to remove at " + path.wstring());

		if (!std::filesystem::remove_all(path))
			return Log::error(L"Failed to remove directory at " + path.wstring());

		return isDirectory(path, false)
			? Log::error(L"Failed to remove directory at " + path.wstring())
			: Log::info(L"Directory Removed at " + path.wstring());
	}
	bool createDirectory(const Path& path) {
		if (!fileExists(path, true) && !std::filesystem::create_directories(path))
			return Log::error(L"Failed to create TaskTracker folder at " + path.wstring());

		return fileExists(path, false)
			? Log::info(L"Folder added at " + path.wstring())
			: Log::error(L"Failed to create folder at " + path.wstring());
	}
}