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
set(testutils_MOC
  dummyqueuemanager.h
  dummyqueueremote.h
  dummyserver.h
  dummysshcommand.h
  testserver.h
)
qt4_wrap_cpp(testutils_MOC_SRC ${testutils_MOC})
add_library(testutils STATIC ${testutils_SRCS} ${testutils_MOC_SRC})
target_link_libraries(testutils molequeue_static)

set(MyTests
  abstractrpcinterface
  client
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
  set(test_SRC ${test}test.cpp)
  set(test_MOC_CPP ${test}test.cpp)
  qt4_wrap_cpp(test_MOC_SRC ${test_MOC_CPP})
  add_custom_target(${test}testmoc ALL DEPENDS ${test_MOC_SRC})
  add_executable(${test}test ${test_SRC})
  add_dependencies(${test}test ${test}testmoc testutilsmoc)
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

qt4_wrap_cpp(connection_test_moc_src connectiontest.h)

add_library(mqconnectiontest STATIC
  ${connection_test_moc_src}
  connectiontest.cpp
  )

target_link_libraries(mqconnectiontest
  molequeue_static
  ${QT_LIBRARIES}
  )

foreach(test ${mq_connection_tests})
  set(test_SRC ${test}test.cpp)
  set(test_MOC_CPP ${test}test.cpp)
  qt4_wrap_cpp(test_MOC_SRC ${test_MOC_CPP})
  add_custom_target(${test}testmoc ALL DEPENDS ${test_MOC_SRC})
  add_executable(${test}test ${test_SRC})
  add_dependencies(${test}test ${test}testmoc testutilsmoc)

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
