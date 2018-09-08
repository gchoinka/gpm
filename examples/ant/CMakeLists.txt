#
#  Copyright 2018 Gerard Choinka
#  
#  Distributed under the Boost Software License, Version 1.0.
#  (See accompanying file LICENSE_1_0.txt or
#  copy at http://www.boost.org/LICENSE_1_0.txt)
#

SET(targetName ant_simulation_generate)
add_executable(${targetName} generate_main.cpp) 
set_target_properties(ant_simulation_generate PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(${targetName} PRIVATE -Wall -Wextra -pedantic -Werror)
target_link_libraries(${targetName} fmt::fmt)



SET(targetName ant_simulation_benchmark)
add_executable(${targetName} benchmark_main.cpp) 
set_target_properties(${targetName} PROPERTIES
           CXX_STANDARD 17
           CXX_EXTENSIONS OFF
)
target_compile_options(${targetName} PRIVATE -Wall -Wextra -pedantic -Werror)
target_link_libraries(${targetName} fmt::fmt)



option(GPM_FORMAT_GENERATED_FILES "Should the files which were generated in ${GeneratedIncludesDir} be formated?(only usefull if someone wants to read them)." OFF)

if(CLANG_FORMAT AND ${GPM_FORMAT_GENERATED_FILES})
    set(FORMAT_STEP "${CLANG_FORMAT}" -i -style=Google "${GeneratedIncludesDir}/ant_simulation_benchmark_generated_functions.cpp")
else()
    set(FORMAT_STEP "${CMAKE_COMMAND}" -E echo "skiped formating step")
endif()


add_custom_target(make_benchmark 
    COMMAND ant_simulation_generate > "${GeneratedIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
    COMMAND ${FORMAT_STEP}
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate "${CMAKE_CURRENT_SOURCE_DIR}/benchmark_main.cpp"
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ant_simulation_benchmark
    DEPENDS ant_simulation_generate
)


if(${GPM_BUILD_TESTS})
    add_subdirectory(tests)
endif()


