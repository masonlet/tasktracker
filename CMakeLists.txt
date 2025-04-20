cmake_minimum_required(VERSION 3.10)
project(TaskTracker)

set(CMAKE_CXX_STANDARD 17)

add_executable(TaskTracker TaskTracker/TaskTracker.cpp)
add_executable(Installer Installer/Installer.cpp)