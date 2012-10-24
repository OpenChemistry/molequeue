cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

project(MoleQueue)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(BuildType)
include(BuildLocation)
include(CompilerFlags)
include(InstallLocation)

# Set up our version.
set(MoleQueue_VERSION_MAJOR   "0")
set(MoleQueue_VERSION_MINOR   "1")
set(MoleQueue_VERSION_PATCH   "0")
set(MoleQueue_VERSION
  "${MoleQueue_VERSION_MAJOR}.${MoleQueue_VERSION_MINOR}.${MoleQueue_VERSION_PATCH}")

option(ENABLE_TESTING "Enable testing" OFF)

option(BUILD_SHARED_LIBS "Build with shared libraries" ON)

option(USE_ZERO_MQ "Build molequeue with ZeroMQ support" OFF)

option(MoleQueue_USE_EZHPC_UIT "Build support for ezHPC UIT interface" OFF)

if(ENABLE_TESTING)
  include(CTest)
  enable_testing()
endif()

# This is necessary for tests, includes etc.
include_directories(FIRST "${MoleQueue_SOURCE_DIR}"
  "${MoleQueue_SOURCE_DIR}/thirdparty"
  "${MoleQueue_SOURCE_DIR}/thirdparty/qt5json/"
  "${MoleQueue_BINARY_DIR}/thirdparty/qt5json/"
  )
set(EXPORT_TARGETS_NAME "MoleQueueTargets")
add_subdirectory(thirdparty/qt5json)
add_subdirectory(molequeue)

option(BUILD_DOCUMENTATION "Build project documentation" OFF)

if(BUILD_DOCUMENTATION)
  add_subdirectory(docs)
endif()

include(CPack)

if(ENABLE_TESTING)
  include(BuildPackageTest)
  BuildPackageTest_Add("MoleQueue" "${CMAKE_CURRENT_BINARY_DIR}")
endif()

configure_file(${MoleQueue_SOURCE_DIR}/cmake/CTestCustom.cmake.in
  ${MoleQueue_BINARY_DIR}/CTestCustom.cmake)

configure_file("${MoleQueue_SOURCE_DIR}/cmake/MoleQueueConfig.cmake.in"
  "${MoleQueue_BINARY_DIR}/MoleQueueConfig.cmake" @ONLY)
configure_file("${MoleQueue_SOURCE_DIR}/cmake/MoleQueueConfigVersion.cmake.in"
  "${MoleQueue_BINARY_DIR}/MoleQueueConfigVersion.cmake" @ONLY)

install(FILES
  "${MoleQueue_BINARY_DIR}/MoleQueueConfig.cmake"
  "${MoleQueue_BINARY_DIR}/MoleQueueConfigVersion.cmake"
  DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/molequeue")
install(EXPORT "MoleQueueTargets"
  DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/molequeue")
