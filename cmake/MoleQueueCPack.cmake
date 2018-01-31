set(CPACK_PACKAGE_NAME "MoleQueue")
set(CPACK_PACKAGE_VERSION_MAJOR ${MoleQueue_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MoleQueue_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MoleQueue_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${MoleQueue_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MoleQueue")
set(CPACK_PACKAGE_VENDOR "http://openchemistry.org/")
set(CPACK_PACKAGE_DESCRIPTION
  "Desktop integration of high performance computing resources.")

if(APPLE)
  configure_file("${MoleQueue_SOURCE_DIR}/LICENSE"
    "${MoleQueue_BINARY_DIR}/LICENSE.txt" @ONLY)
  set(CPACK_RESOURCE_FILE_LICENSE "${MoleQueue_BINARY_DIR}/LICENSE.txt")
  set(CPACK_PACKAGE_ICON
    "${MoleQueue_SOURCE_DIR}/molequeue/app/icons/molequeue.icns")
  set(CPACK_BUNDLE_ICON "${CPACK_PACKAGE_ICON}")
else()
  set(CPACK_RESOURCE_FILE_LICENSE "${MoleQueue_SOURCE_DIR}/LICENSE")
endif()

set(CPACK_PACKAGE_EXECUTABLES "molequeue" "MoleQueue")
set(CPACK_CREATE_DESKTOP_LINKS "molequeue")

configure_file("${CMAKE_CURRENT_LIST_DIR}/MoleQueueCPackOptions.cmake.in"
  "${MoleQueue_BINARY_DIR}/MoleQueueCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE
  "${MoleQueue_BINARY_DIR}/MoleQueueCPackOptions.cmake")

include(CPack)
