cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(MoleQueue)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Request C++11 standard, using new CMake variables.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)
# Set symbol visibility defaults for all targets.
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN True)

include(BuildType)
include(BuildLocation)
include(CompilerFlags)
include(InstallLocation)
include(SilenceWarnings)
include(DetermineVersion)

# Set up our version.
set(MoleQueue_VERSION_MAJOR   "0")
set(MoleQueue_VERSION_MINOR   "8")
set(MoleQueue_VERSION_PATCH   "0")
set(MoleQueue_VERSION
  "${MoleQueue_VERSION_MAJOR}.${MoleQueue_VERSION_MINOR}.${MoleQueue_VERSION_PATCH}")
find_package(Git)
determine_version(${MoleQueue_SOURCE_DIR} ${GIT_EXECUTABLE} "MoleQueue")

if(APPLE)
  set(MACOSX_BUNDLE_NAME "MoleQueue")
  # Handle installation prefix for Mac bundles...
  option(USE_BUNDLE_LAYOUT "Use the Mac OS X bundle layout" OFF)
endif()
if(APPLE AND USE_BUNDLE_LAYOUT)
  set(prefix "${MACOSX_BUNDLE_NAME}.app/Contents")
  set(INSTALL_INCLUDE_DIR "${prefix}/${INSTALL_INCLUDE_DIR}")
  set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
  set(INSTALL_LIBRARY_DIR "${prefix}/${INSTALL_LIBRARY_DIR}")
  set(INSTALL_ARCHIVE_DIR "${prefix}/${INSTALL_ARCHIVE_DIR}")
  set(INSTALL_DATA_DIR    "${prefix}/${INSTALL_DATA_DIR}")
  set(INSTALL_DOC_DIR     "${prefix}/${INSTALL_DOC_DIR}")
  set(INSTALL_CMAKE_DIR   "${prefix}/Resources")
else()
  set(INSTALL_CMAKE_DIR   "${INSTALL_LIBRARY_DIR}/cmake/molequeue")
endif()

option(ENABLE_TESTING "Enable testing" OFF)

option(BUILD_SHARED_LIBS "Build with shared libraries" ON)

option(MoleQueue_USE_EZHPC_UIT "Build support for ezHPC UIT interface" OFF)

if(ENABLE_TESTING)
  include(CTest)
  enable_testing()
endif()

# This is necessary for tests, includes etc.
include_directories(BEFORE "${MoleQueue_SOURCE_DIR}")
set(EXPORT_TARGETS_NAME "MoleQueueTargets")
# We might want qt5json back if we build a Qt 4 client library.
#add_subdirectory(thirdparty/qt5json)
add_subdirectory(molequeue)

option(BUILD_DOCUMENTATION "Build project documentation" OFF)

if(BUILD_DOCUMENTATION)
  add_subdirectory(docs)
endif()

if(USE_ZERO_MQ)
  add_subdirectory(python)
endif()

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
  DESTINATION "${INSTALL_CMAKE_DIR}")
install(EXPORT "MoleQueueTargets"
  DESTINATION "${INSTALL_CMAKE_DIR}")

install(
  FILES
    README.md
    CONTRIBUTING.md
    COPYING
  DESTINATION "${INSTALL_DOC_DIR}/molequeue")

include(MoleQueueCPack)
