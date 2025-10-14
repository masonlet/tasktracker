#include "fileUtils.hpp"
#include "log.hpp"

#include <fstream>

bool fileExists(const Path& path) {
	return std::filesystem::exists(path);
}

bool deleteFile(const Path& path) {
	if (fileExists(path) && !std::filesystem::remove(path)) 
		return error(L"Failed to remove file at " + path.wstring());

	return fileExists(path)
		? log(L"Failed to remove file at " + path.wstring(), true)
		: log(L"File Removed at " + path.wstring());
}

bool deleteDirectory(const Path& path) {
	if (fileExists(path) && !std::filesystem::remove_all(path)) 
		return error(L"Failed to remove directory at " + path.wstring());

	return fileExists(path)
		? log(L"Failed to remove directory at " + path.wstring(), true)
		: log(L"Directory Removed at " + path.wstring());
}

bool createDirectory(const Path& path) {
	if (!fileExists(path) && !std::filesystem::create_directories(path))
		return error(L"Failed to create TaskTracker folder at " + path.wstring());

	return fileExists(path)
		? log(L"Folder added at " + path.wstring())
		: log(L"Failed to create folder at " + path.wstring(), true);
}
