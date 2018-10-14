#
#  Copyright 2018 Gerard Choinka
#  
#  Distributed under the Boost Software License, Version 1.0.
#  (See accompanying file LICENSE_1_0.txt or
#  copy at http://www.boost.org/LICENSE_1_0.txt)
#

#move this to somewhere else
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(warning_flags "")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(warning_flags -Wall -Wextra -pedantic -Werror)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(warning_flags -Wall -Wextra -pedantic -Werror -Wno-gnu-string-literal-operator-template)
endif()

set(GeneratedBechmarksIncludesDir "${CMAKE_CURRENT_BINARY_DIR}/generated_includes")
file(MAKE_DIRECTORY "${GeneratedBechmarksIncludesDir}")

add_library(GeneratedBechmarks INTERFACE)
target_include_directories(GeneratedBechmarks INTERFACE ${GeneratedBechmarksIncludesDir})



add_executable(generate_tree_for_benchmark generate_tree_for_benchmark_main.cpp) 
set_target_properties(generate_tree_for_benchmark PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(generate_tree_for_benchmark PRIVATE ${warning_flags})
target_link_libraries(generate_tree_for_benchmark Gpm fmt::fmt Boost::boost Boost::program_options Outcome)


add_executable(tree_benchmark tree_benchmark_main.cpp) 
set_target_properties(tree_benchmark PROPERTIES
           CXX_STANDARD 17
           CXX_EXTENSIONS OFF
)
target_compile_options(tree_benchmark PRIVATE ${warning_flags})
target_link_libraries(tree_benchmark Gpm GeneratedBechmarks fmt::fmt benchmark::benchmark Boost::boost Boost::program_options Outcome Dyno)


add_executable(ant_board_visualization ant_board_visualization.cpp) 
set_target_properties(ant_board_visualization PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(ant_board_visualization PRIVATE ${warning_flags})
target_link_libraries(ant_board_visualization Gpm fmt::fmt Boost::boost Boost::program_options Outcome) 



option(GPM_FORMAT_GENERATED_FILES "Should the files which were generated in ${GeneratedBechmarksIncludesDir} be formated?(only usefull if someone wants to read them)." OFF)

if(CLANG_FORMAT AND ${GPM_FORMAT_GENERATED_FILES})
    set(FORMAT_STEP "${CLANG_FORMAT_EXECUTABLE}" -i -style=google "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp")
else()
    set(FORMAT_STEP "${CMAKE_COMMAND}" -E echo "skiped formating step")
endif()

add_custom_target(timeing_build_tree_benchmark_all)

set(last_target timeing_build_tree_benchmark_all)
foreach(bmName variantDynamic variantCTStatic oopTreeDynamic oppTreeCTStatic unwrappedDirectCTStatic unwrappedVisitorCallingCTStatic tupleCTStatic dynoTreeDynamic dynoTreeCTStatic None)
    add_custom_target(timeing_build_tree_benchmark_${bmName}
      COMMAND "${CMAKE_COMMAND}" -E remove "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
      COMMAND "$<TARGET_FILE:generate_tree_for_benchmark>" --benchmark ${bmName} --antrpndef "${CMAKE_CURRENT_SOURCE_DIR}/data/randomAnt.txt" --outfile "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
      COMMAND ${FORMAT_STEP}
      COMMAND "${CMAKE_COMMAND}" -E touch_nocreate "${CMAKE_CURRENT_SOURCE_DIR}/tree_benchmark_main.cpp"
      COMMAND /usr/bin/time -v -o "${CMAKE_CURRENT_BINARY_DIR}/buildtime_${bmName}.txt" "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target tree_benchmark
      DEPENDS generate_tree_for_benchmark
    )
    add_dependencies(${last_target} timeing_build_tree_benchmark_${bmName})
    set(last_target timeing_build_tree_benchmark_${bmName})
endforeach()
    


add_custom_target(make_tree_benchmark 
    COMMAND "${CMAKE_COMMAND}" -E remove "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
    COMMAND "$<TARGET_FILE:generate_tree_for_benchmark>" --benchmark all --antrpndef "${CMAKE_CURRENT_SOURCE_DIR}/data/randomAnt.txt" --outfile "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
    COMMAND ${FORMAT_STEP}
    COMMAND "${CMAKE_COMMAND}" -E touch_nocreate "${CMAKE_CURRENT_SOURCE_DIR}/tree_benchmark_main.cpp"
    COMMAND /usr/bin/time -v -o "${CMAKE_CURRENT_BINARY_DIR}/buildtime.txt" "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target tree_benchmark
    DEPENDS generate_tree_for_benchmark
)

# add_custom_target(gdb_run_tree_benchmark 
#     COMMAND gdb -return-child-result -quiet -batch -ex "source ${CMAKE_SOURCE_DIR}/gittools/ci/gdbinit.script" -ex run --args "$<TARGET_FILE:tree_benchmark>" --benchmark_out_format=json --benchmark_out="${CMAKE_CURRENT_BINARY_DIR}/tree_benchmark.json" -b "${CMAKE_CURRENT_SOURCE_DIR}/data/santa_fe_board.txt"
#     DEPENDS make_tree_benchmark
# )


add_custom_target(run_tree_benchmark 
    COMMAND "$<TARGET_FILE:tree_benchmark>" --benchmark_out_format=json --benchmark_out="${CMAKE_CURRENT_BINARY_DIR}/tree_benchmark.json" -b "${CMAKE_CURRENT_SOURCE_DIR}/data/santa_fe_board.txt"
    DEPENDS make_tree_benchmark
)



if(${GPM_BUILD_TESTS})
    add_subdirectory(tests)
endif()


