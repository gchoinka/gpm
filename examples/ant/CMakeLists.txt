SET(targetName ant_simulation_generate)
add_executable(${targetName} ant_simulation_generate.cpp) 
set_target_properties(ant_simulation_generate PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(${targetName} PRIVATE -Wall -Wextra -pedantic -Werror)
target_link_libraries(${targetName} fmt::fmt)



SET(targetName static_vs_dynamic_ant)
add_executable(${targetName} static_vs_dynamic_ant.cpp) 
set_target_properties(${targetName} PROPERTIES
           CXX_STANDARD 17
           CXX_EXTENSIONS OFF
)
target_compile_options(${targetName} PRIVATE -Wall -Wextra -pedantic -Werror)
target_link_libraries(${targetName} fmt::fmt)



SET(targetName ant_simulation_benchmark)
add_executable(${targetName} ant_simulation_benchmark.cpp) 
set_target_properties(${targetName} PROPERTIES
           CXX_STANDARD 17
           CXX_EXTENSIONS OFF
)
target_compile_options(${targetName} PRIVATE -Wall -Wextra -pedantic -Werror)
target_link_libraries(${targetName} fmt::fmt)



add_custom_target(make_static_vs_dynamic 
    COMMAND ant_simulation_generate > "${GeneratedIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate "${CMAKE_CURRENT_SOURCE_DIR}/ant_simulation_benchmark.cpp"
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ant_simulation_benchmark
    DEPENDS ant_simulation_generate
)


if(${GPM_BUILD_TESTS})
    add_subdirectory(tests)
endif()


