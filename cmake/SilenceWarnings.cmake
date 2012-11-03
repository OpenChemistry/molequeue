# This file provides a macro to disable compiler warnings on source files.
#
# usage: silence_warnings(${source_files})

macro(silence_warnings _sources)
  if(BORLAND)
    set_property(SOURCE ${_sources}
      PROPERTY
      COMPILE_FLAGS "-w-")
  else()
    set_property(SOURCE ${_sources}
      PROPERTY
      COMPILE_FLAGS "-w")
  endif()
  # Some specific MSVC warnings:
  if(MSVC)
    set_property(SOURCE ${_sources} PROPERTY COMPILE_FLAGS
        "-wd4068" # Unknown pragma
    )
  endif()
endmacro()
