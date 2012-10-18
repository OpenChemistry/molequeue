include_directories(${CMAKE_SOURCE_DIR}/molequeue
  ${CMAKE_BINARY_DIR}/molequeue/client
  ${CMAKE_BINARY_DIR}/molequeue/testing)

set(qt_components QtCore QtGui QtNetwork QtTest)

if(MoleQueue_USE_EZHPC_UIT)
  list(APPEND qt_components QtXmlPatterns)
endif()

find_package(Qt4 4.8.0 COMPONENTS ${qt_components})
include(${QT_USE_FILE})

include_directories(${CMAKE_BINARY_DIR}/molequeue/transport/localsocket)

add_definitions(-DMoleQueue_TESTDATA_DIR="${CMAKE_SOURCE_DIR}/molequeue/testing/data/")
add_definitions(-DMoleQueue_TESTSCRIPT_DIR="${CMAKE_SOURCE_DIR}/molequeue/testing/scripts/")
add_definitions(-DMoleQueue_PYTHON_EXECUTABLE="${PYTHON_EXECUTABLE}")
add_definitions(-DMoleQueue_SOURCE_DIR="${MoleQueue_SOURCE_DIR}")
add_definitions(-DMoleQueue_BINARY_DIR="${MoleQueue_BINARY_DIR}")

if(USE_ZERO_MQ)
  add_definitions(-DUSE_ZERO_MQ)
endif()

set(testutils_SRCS
  dummyjsonrpc.cpp
  dummyqueuemanager.cpp
  dummyqueueremote.cpp
  dummyserver.cpp
  dummysshcommand.cpp
  referencestring.cpp
  testserver.cpp
  xmlutils.cpp
)

add_library(testutils STATIC ${testutils_SRCS})
set_target_properties(testutils PROPERTIES AUTOMOC TRUE)
target_link_libraries(testutils molequeue_static)

set(MyTests
  abstractrpcinterface
  client
  clientjsonrpc
  filespecification
  jobmanager
  jsonrpc
  pbs
  program
  queue
  queuemanager
  queueremote
  server
  serverjsonrpc
  sge
  sshcommand
  )

# This test is currently only configured to run on unix
if(USE_ZERO_MQ AND PYTHON_EXECUTABLE AND NOT WIN32)
  list(APPEND MyTests clientserver)
endif()

if(MoleQueue_USE_EZHPC_UIT)
  list(APPEND MyTests
    uit
    kerberoscredentials
    authenticatecont
    authenticateresponse)
endif()

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

include_directories("${MoleQueue_SOURCE_DIR}/thirdparty/qt5json")
set(test "localclient")
add_executable(${test}test ${test}test.cpp)
set_target_properties(${test}test PROPERTIES AUTOMOC TRUE)
target_link_libraries(${test}test
  ${QT_LIBRARIES}
  MoleQueueClient
  testutils)
add_test(NAME ${test} COMMAND ${test}test)

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
