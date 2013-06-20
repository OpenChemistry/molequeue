find_package(PythonInterp REQUIRED)

if(PYTHONINTERP_FOUND)
  execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
    "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
    OUTPUT_VARIABLE PYTHON_PACKAGES_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  install(FILES
    molequeue/__init__.py
    molequeue/client.py
    molequeue/utils.py
    DESTINATION "${PYTHON_PACKAGES_DIR}/molequeue")
endif()
