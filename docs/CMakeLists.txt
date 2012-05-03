find_package(Doxygen REQUIRED)

set(doxygen_source_dirs "${MoleQueue_SOURCE_DIR}/molequeue")
set(doxygen_output_dir "${MoleQueue_BINARY_DIR}/docs")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in"
  "${CMAKE_CURRENT_BINARY_DIR}/doxyfile")

add_custom_target(documentation
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/html
  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)
