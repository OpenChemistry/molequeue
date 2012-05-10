include_directories(${CMAKE_SOURCE_DIR}/molequeue)
include_directories(${CMAKE_BINARY_DIR}/molequeue/testing)

find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtTest)
include(${QT_USE_FILE})

# Where to find test files
add_definitions(-DTESTDATADIR="${CMAKE_SOURCE_DIR}/molequeue/testing/data/")


set(MyTests
  program
  queue
  jsonrpc
  )

foreach(test ${MyTests})
  message(STATUS "Adding test:  ${test}")
  set(test_SRC ${test}test.cpp)
  set(test_MOC_CPP ${test}test.cpp)
  qt4_wrap_cpp(test_MOC_SRC ${test_MOC_CPP})
  add_custom_target(${test}testmoc ALL DEPENDS ${test_MOC_SRC})
  add_executable(${test}test ${test_SRC})
  add_dependencies(${test}test ${test}testmoc)
  target_link_libraries(${test}test molequeue_static ${QT_LIBRARIES})

  add_test(${test}Test ${CMAKE_BINARY_DIR}/bin/${test}test)

endforeach()
