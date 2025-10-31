#pragma once

#include "filePath.h"
#include <optional>

namespace TaskTracker {
	struct ParsedArgs {
		Path folder;
		Path icon;
	};

	enum class ParseResult {
		Success,
		ShowHelp,
		Error
	};

	struct ArgResult {
		ParseResult result;
		std::optional<ParsedArgs> args;
	};

	ArgResult parseArgs(const int argc, wchar_t* argv[]);
	int setFolderIcon(const Path& folder, const Path& icon);
}
