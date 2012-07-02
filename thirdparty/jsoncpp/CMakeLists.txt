message(STATUS "cur sourcedir: ${CMAKE_CURRENT_SOURCE_DIR}")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} jsoncpp_srcs)

# Disable warnings
if(MSVC)
  string(REGEX REPLACE "(^| )[/-]W[0-4]( |$)" " "
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
elseif(BORLAND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w-")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

# Turn on -fPIC
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_library(jsoncpp STATIC ${jsoncpp_srcs})
