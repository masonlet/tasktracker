#include "registryUtils.hpp"
#include "systemUtils.hpp"
#include "log.hpp"

#include <array>

namespace TaskTracker::RegistryUtils {
	const std::wstring SHELL_PATH = REGISTRY_PATH + L"\\shell";
	const std::wstring FINISHED_PATH = SHELL_PATH + L"\\Finished";
	const std::wstring HIDDEN_PATH = SHELL_PATH + L"\\Hidden";
	const std::wstring UNFINISHED_PATH = SHELL_PATH + L"\\Unfinished";
	const std::wstring DEFAULT_PATH = SHELL_PATH + L"\\Default";

	bool createTaskTrackerKeys() {
		RegKey hKey{};
		DWORD disposition{};

		//Base key
		if (!createRegistryKey(HKEY_CLASSES_ROOT, REGISTRY_PATH, hKey, disposition))
			return Log::error(L"Installer", L"Failed to create Main Registry key");

		//Base Key Values
		if (!setRegistryValue(hKey, L"Icon", L"shell32.dll,-4")
			|| !setRegistryValue(hKey, L"MUIVerb", L"Task Tracker")
			|| !setRegistryValue(hKey, L"ExtendedSubCommandsKey", REGISTRY_PATH)
			) return Log::error(L"Installer", L"Failed to set Registry key values");

		const std::array<SubKey, 4> subCommands = {
			SubKey{L"Finished", FINISHED_PATH, L"C:\\Windows\\System32\\shell32.dll,-16810"},
			SubKey{L"Hidden", HIDDEN_PATH, L"C:\\Windows\\System32\\shell32.dll,-200"},
			SubKey{L"Unfinished", UNFINISHED_PATH, L"C:\\Windows\\System32\\shell32.dll,-240"},
			SubKey{L"Default", DEFAULT_PATH, L"C:\\Windows\\System32\\shell32.dll,-4"}
		};

		for (const SubKey& subCommand : subCommands) {
			RegKey subKey, folderKey;

			//Sub Key
			if (!createRegistryKey(HKEY_CLASSES_ROOT, subCommand.path, subKey, disposition))
				return Log::error(L"Installer", L"Failed to create Registry subkey: " + subCommand.path.wstring());

			//Sub Key Values
			if (!setRegistryValue(subKey, L"MUIVerb", subCommand.name)
				|| !setRegistryValue(subKey, L"Icon", subCommand.iconPath)
				) return Log::error(L"Installer", L"Failed to set Registry subkey " + subCommand.path.wstring() + L" values");

			//Sub Folder
			const Path folderPath{ subCommand.path / "command" };
			if (!createRegistryKey(HKEY_CLASSES_ROOT, folderPath, folderKey, disposition))
				return Log::error(L"Installer", L"Failed to create Registry subkey " + folderPath.wstring());

			//Sub Key Command
			const std::wstring cmd{ L"\"" + SystemUtils::getExePath().wstring() + L"\" \"%V\" \"" + subCommand.iconPath + L"\"" };
			if (!setRegistryValue(folderKey, L"", cmd))
				return Log::error(L"Installer", L"Failed to set Registry subkey " + folderPath.wstring() + L" command");
		}

		return Log::info(L"Installer", L"Program keys successfully added\n");
	}
	bool deleteTaskTrackerKeys() {
		if (!registryKeyExists(REGISTRY_PATH))
			return Log::info(L"Installer", L"No program keys to delete");

		bool success{ true };
		success &= deleteRegistryKey(SHELL_PATH);
		success &= deleteRegistryKey(REGISTRY_PATH);
		return success
			? Log::info(L"Installer", L"Program Keys successfully removed\n")
			: Log::error(L"Installer", L"Partial / Entire failure of program key removal\n");
	}

	bool registryKeyExists(const std::wstring& path) {
		return RegOpenKeyExW(HKEY_CLASSES_ROOT, path.data(), 0, KEY_READ, &RegKey{}) != ERROR_SUCCESS
			? Log::info(L"Installer", L"Key not found at " + path, false)
			: Log::info(L"Installer", L"Key found at " + path);
	}
	bool createRegistryKey(const HKEY hKey, const std::wstring& path, RegKey& hKeyHandle, DWORD& disposition) {
		return RegCreateKeyExW(hKey, path.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKeyHandle, &disposition) != ERROR_SUCCESS
			? Log::error(L"Installer", L"Failed to create registry key at " + path)
			: Log::info(L"Installer", L"Created key at " + path);
	}
	bool deleteRegistryKey(const std::wstring& path) {
		return RegDeleteTreeW(HKEY_CLASSES_ROOT, path.c_str()) != ERROR_SUCCESS
			? Log::error(L"Installer", L"Failed to remove key at " + path)
			: Log::info(L"Installer", L"Removed key at " + path);
	}
	bool setRegistryValue(RegKey& hKey, const std::wstring_view& valueName, const std::wstring& value) {
		return RegSetValueExW(hKey, valueName.data(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.data()), static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t))) != ERROR_SUCCESS
			? Log::error(L"Installer", L"Failed to set registry to " + value)
			: Log::info(L"Installer", L"Set registry value to " + value);
	}
}