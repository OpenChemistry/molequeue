cmake_minimum_required(VERSION 2.8)

project(MoleQueue)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(BuildType)
include(BuildLocation)
include(CompilerFlags)

option(ENABLE_TESTING "Enable testing" OFF)

if(ENABLE_TESTING)
  include(CTest)
  enable_testing()
endif()

# This is necessary for tests, includes etc.
include_directories("${MoleQueue_SOURCE_DIR}")
add_subdirectory(molequeue)

option(BUILD_DOCUMENTATION "Build project documentation" OFF)

if(BUILD_DOCUMENTATION)
  add_subdirectory(docs)
endif()
