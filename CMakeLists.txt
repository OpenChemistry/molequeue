cmake_minimum_required(VERSION 2.8)

project(MoleQueue)

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(BuildType)
include(BuildLocation)
include(CompilerFlags)

# This is necessary for tests, includes etc.
include_directories("${MoleQueue_SOURCE_DIR}")
add_subdirectory(molequeue)
