if(NOT ${GPM_BUILD_TESTS})
  RETURN()
endif ()

enable_testing()

add_executable(artificial_ant_tests artificial_ant_tests.cpp) 
set_target_properties(artificial_ant_tests PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(artificial_ant_tests PRIVATE ${warning_flags})
target_link_libraries(artificial_ant_tests PUBLIC Catch2::Catch Boost::boost)

add_test(NAME artificial_ant_tests COMMAND artificial_ant_tests)
