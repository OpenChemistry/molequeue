option(MoleQueue_BUILD_APPLICATION "Build the MoleQueue server application" ON)
option(MoleQueue_BUILD_CLIENT "Build the MoleQueue client library" ON)
option(USE_ZERO_MQ "Build molequeue with ZeroMQ support" OFF)

add_subdirectory(servercore)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/servercore)

# Are we using ZeroMQ
if(USE_ZERO_MQ)
  add_subdirectory(zeromq)
endif()

add_subdirectory(plugins)

# Client library
if(MoleQueue_BUILD_CLIENT)
  add_subdirectory(client)
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/client)
endif()

if(MoleQueue_BUILD_APPLICATION)
  add_subdirectory(app)
endif()

# Keep "add_subdirectory(lastinstall)" last: fixup_bundle needs to be
# *after* all other install(TARGETS and install(FILES calls
if(MoleQueue_BUILD_APPLICATION)
  add_subdirectory(lastinstall)
endif()
