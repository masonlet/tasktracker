#include "taskTrackerUtils.hpp"

int wmain(int argc, wchar_t* argv[]){
	TaskTracker::ArgResult args = TaskTracker::parseArgs(argc, argv);
	switch (args.result) {
	case TaskTracker::ParseResult::ShowHelp: return EXIT_SUCCESS;
	case TaskTracker::ParseResult::Error:    return EXIT_FAILURE;
	case TaskTracker::ParseResult::Success:  return TaskTracker::setFolderIcon(args.args->folder, args.args->icon);
	}
}