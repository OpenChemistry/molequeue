cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)

project(MoleQueue)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(BuildType)
include(BuildLocation)
include(CompilerFlags)
include(InstallLocation)

option(ENABLE_TESTING "Enable testing" OFF)

option(BUILD_SHARED_LIBS "Build with shared libraries" ON)

if(ENABLE_TESTING)
  include(CTest)
  enable_testing()
endif()

# This is necessary for tests, includes etc.
include_directories("${MoleQueue_SOURCE_DIR}")
add_subdirectory(molequeue)

configure_file(${MoleQueue_SOURCE_DIR}/cmake/CTestCustom.cmake.in
  ${MoleQueue_BINARY_DIR}/CTestCustom.cmake)

option(BUILD_DOCUMENTATION "Build project documentation" OFF)

if(BUILD_DOCUMENTATION)
  add_subdirectory(docs)
endif()

include(CPack)

if(ENABLE_TESTING)
  include(BuildPackageTest)
  BuildPackageTest_Add("MoleQueue" "${CMAKE_CURRENT_BINARY_DIR}")
endif()

option(USE_ZERO_MQ "Build molequeue with ZeroMQ support" OFF)
