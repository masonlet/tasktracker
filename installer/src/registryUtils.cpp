#include "registryUtils.hpp"
#include "systemUtils.hpp"
#include "log.hpp"

#include <array>

const std::wstring SHELL_PATH = REGISTRY_PATH + L"\\shell";
const std::wstring FINISHED_PATH = SHELL_PATH + L"\\Finished";
const std::wstring HIDDEN_PATH = SHELL_PATH + L"\\Hidden";
const std::wstring UNFINISHED_PATH = SHELL_PATH + L"\\Unfinished";
const std::wstring DEFAULT_PATH = SHELL_PATH + L"\\Default";

const std::wstring FINISHED_CMD_PATH = FINISHED_PATH + L"\\command";
const std::wstring HIDDEN_CMD_PATH = HIDDEN_PATH + L"\\command";
const std::wstring UNFINISHED_CMD_PATH = UNFINISHED_PATH + L"\\command";
const std::wstring DEFAULT_CMD_PATH = DEFAULT_PATH + L"\\command";

bool keyExists(const std::wstring_view& path) {
	RegKey hKey;
	return RegOpenKeyExW(HKEY_CLASSES_ROOT, path.data(), 0, KEY_READ, &hKey) == ERROR_SUCCESS;
}

bool deleteKey(const std::wstring& path) {
	return RegDeleteTreeW(HKEY_CLASSES_ROOT, path.c_str()) != ERROR_SUCCESS
		? log(L"Failed to remove key at " + path, true)
		: log(L"Removed key at " + path);
}

bool createRegistryKey(const HKEY hKey, const std::wstring& path, RegKey& hKeyHandle, DWORD& disposition) {
	LONG result = RegCreateKeyExW(hKey, path.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKeyHandle, &disposition);
	return (result != ERROR_SUCCESS)
		? log(L"Failed to create registry key " + std::to_wstring(result) + L" at " + path, true)
		: log(L"Created key at " + path);
}
bool setRegistryValue(RegKey& hKey, const std::wstring_view& valueName, const std::wstring_view& value) {
	LONG result = RegSetValueExW(hKey, valueName.data(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.data()), static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
	return (result != ERROR_SUCCESS)
		? log(L"Failed to set registry value " + std::to_wstring(result), true)
		: true;
}

bool createTaskTrackerKeys() {
	RegKey hKey{};
	DWORD disposition{};

	//Base key
	if (!createRegistryKey(HKEY_CLASSES_ROOT, REGISTRY_PATH, hKey, disposition))
		return error(L"Failed to create Main Registry key");

	//Base Key Values
	if (!setRegistryValue(hKey, L"Icon", L"shell32.dll,-4") 
	 || !setRegistryValue(hKey, L"MUIVerb", L"Task Tracker") 
	 || !setRegistryValue(hKey, L"ExtendedSubCommandsKey", REGISTRY_PATH)
		) return error(L"Failed to set Registry key values");

	struct SubKey {
		const std::wstring name;
		const Path path;
		const std::wstring iconPath;
	};
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
			return error(L"Failed to create Registry subkey: " + subCommand.path.wstring());

		//Sub Key Values
		if (!setRegistryValue(subKey, L"MUIVerb", subCommand.name)
			|| !setRegistryValue(subKey, L"Icon", subCommand.iconPath))
			return error(L"Failed to set Registry subkey " + subCommand.path.wstring() + L" values");
		
		//Sub Folder
		const Path folderPath{ subCommand.path / "command" };
		if (!createRegistryKey(HKEY_CLASSES_ROOT, folderPath, folderKey, disposition))
			return log(L"Failed to create Registry subkey " + folderPath.wstring(), true);
		 
		//Sub Key Command
		const std::wstring cmd = L"\"" + EXE_PATH.wstring() + L"\" \"%V\" \"" + subCommand.iconPath + L"\"";
		if (!setRegistryValue(folderKey, L"", cmd))
			return log(L"Failed to set Registry subkey " + folderPath.wstring() + L" command", true);
	}

	return log(L"Program keys successfully added");
}
bool deleteTaskTrackerKeys() {
	bool success{ true };

	success &= deleteKey(SHELL_PATH);
	success &= deleteKey(REGISTRY_PATH);

	return success 
		? log(L"Program Keys successfully removed") 
		: log(L"Partial / Entire failure of program key removal", true);
}