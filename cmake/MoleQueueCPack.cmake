set(CPACK_PACKAGE_NAME "MoleQueue")
set(CPACK_PACKAGE_VERSION_MAJOR ${MoleQueue_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MoleQueue_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MoleQueue_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${MoleQueue_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MoleQueue")

if(APPLE)
  configure_file("${MoleQueue_SOURCE_DIR}/COPYING"
    "${MoleQueue_BINARY_DIR}/COPYING.txt" @ONLY)
  set(CPACK_RESOURCE_FILE_LICENSE "${MoleQueue_BINARY_DIR}/COPYING.txt")
else()
  set(CPACK_RESOURCE_FILE_LICENSE "${MoleQueue_SOURCE_DIR}/COPYING")
endif()

set(CPACK_PACKAGE_EXECUTABLES "molequeue" "MoleQueue")
set(CPACK_CREATE_DESKTOP_LINKS "molequeue")

configure_file("${CMAKE_CURRENT_LIST_DIR}/MoleQueueCPackOptions.cmake.in"
  "${MoleQueue_BINARY_DIR}/MoleQueueCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE
  "${MoleQueue_BINARY_DIR}/MoleQueueCPackOptions.cmake")

include(CPack)
