
add_test(NAME test_ruby
  COMMAND ruby -I . ${PROJECT_SOURCE_DIR}/Ruby/test.rb
  WORKING_DIRECTORY $<TARGET_FILE_DIR:ruby_isomodel>
)

