add_executable(ant_simulation_generate ant_simulation_generate.cpp) 
set_target_properties(ant_simulation_generate PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(ant_simulation_generate PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(ant_simulation_generate fmt::fmt)



add_executable(static_vs_dynamic_ant static_vs_dynamic_ant.cpp) 
set_target_properties(static_vs_dynamic_ant PROPERTIES
           CXX_STANDARD 17
           CXX_EXTENSIONS OFF
)
target_compile_options(static_vs_dynamic_ant PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(static_vs_dynamic_ant fmt::fmt)

add_executable(ant_simulation_benchmark ant_simulation_benchmark.cpp) 
set_target_properties(ant_simulation_benchmark PROPERTIES
           CXX_STANDARD 17
           CXX_EXTENSIONS OFF
)
target_compile_options(ant_simulation_benchmark PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(ant_simulation_benchmark fmt::fmt)




add_subdirectory(tests)


