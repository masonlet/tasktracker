cmake_minimum_required(VERSION 3.10)
project(TaskTracker)
set(CMAKE_CXX_STANDARD 17)

add_executable(TaskTracker tasktracker/tasktracker.cpp)

add_executable(Installer installer/installer.cpp installer/tasktracker.rc)
SET_TARGET_PROPERTIES(Installer PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE")

add_dependencies(Installer TaskTracker)

add_custom_command(TARGET TaskTracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:TaskTracker>
        ${CMAKE_SOURCE_DIR}/installer/tasktracker.exe
)
