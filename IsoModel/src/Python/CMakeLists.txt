
add_test(NAME test_python
  COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Python/test.py
  WORKING_DIRECTORY $<TARGET_FILE_DIR:python_isomodel>
)

set_tests_properties(test_python PROPERTIES
    ENVIRONMENT PYTHONPATH=.)

install(TARGETS python_isomodel
  DESTINATION python
)
install(FILES $<TARGET_FILE_DIR:python_isomodel>/isomodel.py
  DESTINATION python
)