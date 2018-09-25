find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format clang-format-9 clang-format-9.0 clang-format-8 clang-format-8.0 clang-format-7 clang-format-7.0 clang-format-6 clang-format-6.0 clang-format-5 clang-format-5.0 clang-format-4 clang-format-4.0 clang-format-3 clang-format-3.0 DOC "Path to clang-format executable")

if(NOT CLANG_FORMAT_EXECUTABLE)
  message(STATUS "clang-format not found.")
else()
  message(STATUS "clang-format found: ${CLANG_FORMAT_EXECUTABLE}")
endif()
