include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${MoleQueue_SOURCE_DIR}/molequeue/client/"
  "${MoleQueue_BINARY_DIR}/molequeue/client/"
)

set(srcs
  sendRpcKill
)

foreach(source ${srcs})
  add_executable(${source} ${source}.cpp)
  set_target_properties(${source} PROPERTIES AUTOMOC TRUE)
  target_link_libraries(${source} MoleQueueClient ${QT_QTCORE_LIBRARY} Qt5Json)
endforeach()
