find_package(Qt4 4.8 REQUIRED)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(sources
  qjson.cpp
  qjsondocument.cpp
  qjsonparser.cpp
  qjsonwriter.cpp
  qjsonarray.cpp
  qjsonobject.cpp
  qjsonvalue.cpp
  )

add_library(Qt5Json ${sources})
target_link_libraries(Qt5Json ${QT_QTCORE_LIBRARY})

add_executable(json main.cpp)
target_link_libraries(json Qt5Json)
