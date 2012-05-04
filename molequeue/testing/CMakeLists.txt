include_directories(..)

find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtTest)
include(${QT_USE_FILE})

# Where to find test files
add_definitions(-DTESTDATADIR="${CMAKE_SOURCE_DIR}/molequeue/testing/data/")


set(MyTests
  program
  queue
  jsonrpc
  )

create_test_sourcelist(Tests MoleQueueTests.cxx ${MyTests})

add_executable(MoleQueueTests ${Tests})
target_link_libraries(MoleQueueTests molequeue_static ${QT_LIBRARIES})

foreach(test ${MyTests})
  add_test(NAME molequeue-${test}
    COMMAND MoleQueueTests ${test})
endforeach()
