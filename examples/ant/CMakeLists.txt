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
  set(default_compiler_flags "")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(warning_flags -Wall -Wextra -pedantic -Werror)
  set(default_compiler_flags -march=native)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(warning_flags -Wall -Wextra -pedantic -Werror -Wno-gnu-string-literal-operator-template)
  set(default_compiler_flags -march=native)
endif()

message(STATUS "${CMAKE_CXX_COMPILER_ID}")

function(setStandard target CXX_STANDARD)
    set_target_properties(${target} PROPERTIES
            CXX_STANDARD ${CXX_STANDARD}
            CXX_EXTENSIONS OFF)
endfunction()

set(GeneratedBechmarksIncludesDir "${CMAKE_CURRENT_BINARY_DIR}/generated_includes")
file(MAKE_DIRECTORY "${GeneratedBechmarksIncludesDir}")

add_library(GeneratedBechmarks INTERFACE)
target_include_directories(GeneratedBechmarks INTERFACE ${GeneratedBechmarksIncludesDir})

add_library(GpmExamples INTERFACE)
target_compile_options(GpmExamples INTERFACE ${warning_flags} ${default_compiler_flags})
target_link_libraries(GpmExamples INTERFACE Gpm fmt::fmt Boost::boost Outcome)
if(${GPM_BUILD_WPROFILER})
  target_link_libraries(GpmExamples INTERFACE ${GPERFTOOLS_PROFILER})
endif()

add_executable(generate_tree_for_benchmark generate_tree_for_benchmark_main.cpp) 
setStandard(generate_tree_for_benchmark 17)
target_link_libraries(generate_tree_for_benchmark GpmExamples Boost::program_options Frozen)

add_executable(tree_benchmark tree_benchmark_main.cpp) 
setStandard(tree_benchmark 17)
target_link_libraries(tree_benchmark GpmExamples GeneratedBechmarks benchmark::benchmark Boost::program_options Frozen)
if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_link_libraries(tree_benchmark Dyno)
endif()

add_executable(ant_board_visualization ant_board_visualization.cpp) 
setStandard(ant_board_visualization 17)
target_link_libraries(ant_board_visualization GpmExamples Boost::program_options Frozen)

add_executable(ant_genetic_programming ant_genetic_programming_main.cpp) 
setStandard(ant_genetic_programming 20)
target_link_libraries(ant_genetic_programming GpmExamples Threads::Threads Boost::program_options Frozen)



#set(antrpndeffile "${CMAKE_CURRENT_SOURCE_DIR}/data/randomAnt.txt")
set(antrpndeffile "${CMAKE_CURRENT_SOURCE_DIR}/data/optAnt.txt")
#set(antrpndeffile "${CMAKE_CURRENT_SOURCE_DIR}/data/optAntPN.txt")

option(GPM_FORMAT_GENERATED_FILES "Should the files which were generated in ${GeneratedBechmarksIncludesDir} be formated?(only usefull if someone wants to read them)." OFF)

if(CLANG_FORMAT AND ${GPM_FORMAT_GENERATED_FILES})
    set(FORMAT_STEP "${CLANG_FORMAT_EXECUTABLE}" -i -style=google "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp")
else()
    set(FORMAT_STEP "${CMAKE_COMMAND}" -E echo "skiped formating step")
endif()

add_custom_target(time_build_tree_benchmark_all)

set(last_target time_build_tree_benchmark_all)

set(bmNameList implicitTreeDynamic funcPtrDynamic variantDynamic variantDynamic2 variantCTStatic oopTreeDynamic oppTreeCTStatic unwrappedDirectCTStatic unwrappedVisitorCallingCTStatic tupleCTStatic None)

if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(bmNameList ${bmNameList} dynoTreeDynamic dynoTreeCTStatic)
endif()

foreach(bmName ${bmNameList})
    add_custom_target(time_build_tree_benchmark_${bmName}
      COMMAND "${CMAKE_COMMAND}" -E remove "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
      COMMAND "$<TARGET_FILE:generate_tree_for_benchmark>" --benchmark ${bmName} --antrpndef "${antrpndeffile}" --outfile "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
      COMMAND ${FORMAT_STEP}
      COMMAND "${CMAKE_COMMAND}" -E touch_nocreate "${CMAKE_CURRENT_SOURCE_DIR}/tree_benchmark_main.cpp"
      COMMAND /usr/bin/time -v -o "${CMAKE_CURRENT_BINARY_DIR}/buildtime_${bmName}.txt" "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target tree_benchmark
      COMMAND stat --printf="%s" "$<TARGET_FILE:tree_benchmark>" > "${CMAKE_CURRENT_BINARY_DIR}/bin_size_${bmName}.txt"
      DEPENDS generate_tree_for_benchmark
    )
    add_dependencies(${last_target} time_build_tree_benchmark_${bmName})
    set(last_target time_build_tree_benchmark_${bmName})
    
    add_custom_target(make_tree_benchmark_${bmName}
      COMMAND "${CMAKE_COMMAND}" -E remove "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
      COMMAND "$<TARGET_FILE:generate_tree_for_benchmark>" --benchmark ${bmName} --antrpndef "${antrpndeffile}" --outfile "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
      COMMAND ${FORMAT_STEP}
      COMMAND "${CMAKE_COMMAND}" -E touch_nocreate "${CMAKE_CURRENT_SOURCE_DIR}/tree_benchmark_main.cpp"
      COMMAND /usr/bin/time -v -o "${CMAKE_CURRENT_BINARY_DIR}/buildtime.txt" "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target tree_benchmark
      DEPENDS generate_tree_for_benchmark
    )
    add_custom_target(run_tree_benchmark_${bmName}
      COMMAND "$<TARGET_FILE:tree_benchmark>" --benchmark_report_aggregates_only=true --benchmark_out_format=json --benchmark_repetitions=10 --benchmark_out="${CMAKE_CURRENT_BINARY_DIR}/tree_benchmark.json" -b "${CMAKE_CURRENT_SOURCE_DIR}/data/santa_fe_board.txt"
      DEPENDS make_tree_benchmark_${bmName}
    )
endforeach()


add_custom_target(make_tree_benchmark 
    COMMAND "${CMAKE_COMMAND}" -E remove "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
    COMMAND "$<TARGET_FILE:generate_tree_for_benchmark>" --benchmark all --antrpndef "${antrpndeffile}" --outfile "${GeneratedBechmarksIncludesDir}/ant_simulation_benchmark_generated_functions.cpp"
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
    COMMAND "$<TARGET_FILE:tree_benchmark>" --benchmark_report_aggregates_only=true --benchmark_out_format=json --benchmark_repetitions=10 --benchmark_out="${CMAKE_CURRENT_BINARY_DIR}/tree_benchmark.json" -b "${CMAKE_CURRENT_SOURCE_DIR}/data/santa_fe_board.txt"
    DEPENDS make_tree_benchmark
)

if(${GPM_BUILD_TESTS})
    add_subdirectory(tests)
endif()
