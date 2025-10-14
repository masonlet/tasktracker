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

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT, path.data(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return false;

	return true;
}

bool deleteKey(const std::wstring& path) {
	if (RegDeleteTreeW(HKEY_CLASSES_ROOT, path.c_str()) != ERROR_SUCCESS) {
		message(L"Failed to remove key at " + path);
		return false;
	}

	return true;
}

bool createRegistryKey(const HKEY hKey, const std::wstring& path, RegKey& hKeyHandle, DWORD& disposition) {
	LONG result = RegCreateKeyExW(hKey, path.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKeyHandle, &disposition);
	if (result != ERROR_SUCCESS)
		return error(L"Failed to create registry key " + std::to_wstring(result) + L" at " + path);

	return true;
}
bool setRegistryValue(RegKey& hKey, const std::wstring& valueName, const std::wstring& value) {
	LONG result = RegSetValueExW(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
	if (result != ERROR_SUCCESS)
		return error(L"Failed to set registry value " + std::to_wstring(result));

	return true;
}

bool createTaskTrackerKeys() {
	RegKey hKey{};
	DWORD disposition{};

	//Base key
	if (!createRegistryKey(HKEY_CLASSES_ROOT, REGISTRY_PATH, hKey, disposition))
		return error(L"Failed to create Main Registry key");

	//Base Key Values
	if (!setRegistryValue(hKey, L"Icon", L"shell32.dll,-4") || !setRegistryValue(hKey, L"MUIVerb", L"Task Tracker") || !setRegistryValue(hKey, L"ExtendedSubCommandsKey", REGISTRY_PATH))
		return error(L"Failed to set Registry key values");

	struct SubKey {
		const std::wstring name;
		const std::filesystem::path path;
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
		const std::filesystem::path folderPath{ subCommand.path / "command" };
		if (!createRegistryKey(HKEY_CLASSES_ROOT, folderPath, folderKey, disposition))
			return error(L"Failed to create Registry subkey " + folderPath.wstring());

		//Sub Key Command
		const std::wstring cmd = L"\"" + EXE_PATH.wstring() + L"\" \"%V\" \"" + subCommand.iconPath + L"\"";
		if (!setRegistryValue(folderKey, L"", cmd))
			return error(L"Failed to set Registry subkey " + folderPath.wstring() + L" command");
	}

	message(L"Program Keys Added");
	return true;
}
bool deleteTasktrackerKeys() {
	bool success = true;

	success &= deleteKey(SHELL_PATH);
	success &= deleteKey(REGISTRY_PATH);

	if (success) message(L"Program Keys Removed");
	return success ? true : error(L"Partial / Entire failure of program key removal");
}