include_directories(${CMAKE_SOURCE_DIR}/molequeue)
include_directories(${CMAKE_BINARY_DIR}/molequeue/testing)

find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtTest)
include(${QT_USE_FILE})

include_directories(${CMAKE_BINARY_DIR}/molequeue/transport/localsocket)

# Where to find test files
add_definitions(-DTESTDATADIR="${CMAKE_SOURCE_DIR}/molequeue/testing/data/")

if(USE_ZERO_MQ)
  add_definitions(-DUSE_ZERO_MQ)
endif()

set(testutils_SRCS
  dummyqueuemanager.cpp
  dummyqueueremote.cpp
  dummyserver.cpp
  dummysshcommand.cpp
  testserver.cpp
)

add_library(testutils STATIC ${testutils_SRCS})
set_target_properties(testutils PROPERTIES AUTOMOC TRUE)
target_link_libraries(testutils molequeue_static)

set(MyTests
  abstractrpcinterface
  client
  filespecification
  jobmanager
  jsonrpc
  pbs
  program
  queue
  queuemanager
  queueremote
  server
  sge
  sshcommand
  )

foreach(test ${MyTests})
  add_executable(${test}test MACOSX_BUNDLE ${test}test.cpp)
  set_target_properties(${test}test PROPERTIES AUTOMOC TRUE)
  target_link_libraries(${test}test
    molequeue_static
    ${QT_LIBRARIES}
    mqconnection
    mqlocalsocketclient
    testutils
    )
  add_test(NAME molequeue-${test} COMMAND ${test}test)
endforeach()

set(mq_connection_tests
  localsocketconnection)

if(USE_ZERO_MQ)
  list(APPEND mq_connection_tests zeromqconnection)
  include_directories(${CMAKE_BINARY_DIR}/molequeue/transport/zeromq)
endif()

add_library(mqconnectiontest STATIC connectiontest.cpp)
set_target_properties(mqconnectiontest PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqconnectiontest
  molequeue_static
  ${QT_LIBRARIES}
  )

foreach(test ${mq_connection_tests})
  set(test_SRC ${test}test.cpp)
  add_executable(${test}test MACOSX_BUNDLE ${test}test.cpp)
  set_target_properties(${test}test PROPERTIES AUTOMOC TRUE)
  set(mq_client_libs mqlocalsocketclient)
  if(USE_ZERO_MQ)
    list(APPEND mq_client_libs mqzeromqclient)
  endif()

  target_link_libraries(${test}test
    mqconnectiontest
    ${mq_client_libs}
    )

  add_test(NAME molequeue-${test} COMMAND ${test}test)
endforeach()
