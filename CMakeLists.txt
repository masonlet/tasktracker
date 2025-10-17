cmake_minimum_required(VERSION 3.10)
project(TaskTrackerInstaller)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(Shared)
add_subdirectory(TaskTracker)
add_subdirectory(Installer)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Installer)
