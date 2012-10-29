set(QT5JSON_SOURCE_DIR "${MoleQueue_SOURCE_DIR}/thirdparty/qt5json/")
aux_source_directory(${QT5JSON_SOURCE_DIR} qt5json_srcs)
foreach(source ${qt5json_srcs})
  if (${source} MATCHES ".*/main.cpp")
    list(REMOVE_ITEM qt5json_srcs ${source})
  endif()
endforeach()

message(STATUS ${qt5json_srcs})

include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${MoleQueue_SOURCE_DIR}/molequeue/client/"
  "${MoleQueue_BINARY_DIR}/molequeue/client/"
  "${QT5JSON_SOURCE_DIR}"
)

set(srcs
  sendRpcKill
)

foreach(source ${srcs})
  add_executable(${source} ${source}.cpp ${qt5json_srcs})
  set_target_properties(${source} PROPERTIES AUTOMOC TRUE)
  target_link_libraries(${source} MoleQueueClient ${QT_QTCORE_LIBRARY} )
endforeach()
