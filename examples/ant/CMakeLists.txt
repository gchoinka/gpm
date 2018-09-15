#
#  Copyright 2018 Gerard Choinka
#  
#  Distributed under the Boost Software License, Version 1.0.
#  (See accompanying file LICENSE_1_0.txt or
#  copy at http://www.boost.org/LICENSE_1_0.txt)
#

#move this to somewhere else
if(MSVC)
  set(warning_flags "-Wall")
else ()
  set(warning_flags -Wall -Wextra -pedantic -Werror)
endif()

add_library(extern_defined_ants SHARED extern_defined_ants.cpp)

set(targetName generate_tree_for_benchmark)
add_executable(${targetName} generate_tree_for_benchmark_main.cpp) 
set_target_properties(${targetName} PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(${targetName} PRIVATE ${warning_flags})
target_link_libraries(${targetName} fmt::fmt Boost::boost)



set(targetName tree_benchmark)
add_executable(${targetName} tree_benchmark_main.cpp) 
set_target_properties(${targetName} PROPERTIES
           CXX_STANDARD 17
           CXX_EXTENSIONS OFF
)
target_compile_options(${targetName} PRIVATE ${warning_flags})
target_link_libraries(${targetName} fmt::fmt benchmark::benchmark Boost::boost extern_defined_ants)



set(targetName ant_board_visualization)
add_executable(${targetName} ant_board_visualization.cpp) 
set_target_properties(${targetName} PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(${targetName} PRIVATE ${warning_flags})
target_link_libraries(${targetName} fmt::fmt Boost::boost)



option(GPM_FORMAT_GENERATED_FILES "Should the files which were generated in ${GeneratedIncludesDir} be formated?(only usefull if someone wants to read them)." OFF)

if(CLANG_FORMAT AND ${GPM_FORMAT_GENERATED_FILES})
    set(FORMAT_STEP "${CLANG_FORMAT}" -i -style=google "${GeneratedIncludesDir}/ant_simulation_benchmark_generated_functions.cpp")
else()
    set(FORMAT_STEP "${CMAKE_COMMAND}" -E echo "skiped formating step")
endif()



add_custom_target(make_tree_benchmark 
    COMMAND $<TARGET_FILE:generate_tree_for_benchmark> > "${GeneratedIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
    COMMAND ${FORMAT_STEP}
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate "${CMAKE_CURRENT_SOURCE_DIR}/tree_benchmark_main.cpp"
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target tree_benchmark
    DEPENDS generate_tree_for_benchmark
)

add_custom_target(run_tree_benchmark 
    COMMAND $<TARGET_FILE:tree_benchmark> --benchmark_out_format=json --benchmark_out=${CMAKE_BINARY_DIR}/tree_benchmark.json
    DEPENDS make_tree_benchmark
)



if(${GPM_BUILD_TESTS})
    add_subdirectory(tests)
endif()


