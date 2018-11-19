
add_executable(mapfmt mapfmt.cpp) 
set_target_properties(mapfmt PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(mapfmt PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(mapfmt fmt::fmt Boost::boost Gpm)

add_executable(hanatupletree hanatupletree.cpp)
set_target_properties(hanatupletree PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(hanatupletree PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(hanatupletree fmt::fmt Boost::boost)

add_executable(dynoexample dynoexample.cpp)
set_target_properties(dynoexample PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(dynoexample PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(dynoexample fmt::fmt Boost::boost Dyno)


add_executable(fasterfactories fasterfactories.cpp)
set_target_properties(fasterfactories PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(fasterfactories PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(fasterfactories fmt::fmt Boost::boost Gpm)


add_executable(mybenchmark mybenchmark.cpp)
set_target_properties(mybenchmark PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(mybenchmark PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(mybenchmark benchmark::benchmark fmt::fmt Boost::boost Gpm Frozen)

add_executable(iterators_and_ranges iterators_and_ranges.cpp)
set_target_properties(iterators_and_ranges PROPERTIES
            CXX_STANDARD 20
            CXX_EXTENSIONS OFF
)
target_compile_options(iterators_and_ranges PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(iterators_and_ranges fmt::fmt Boost::boost Gpm Threads::Threads)


add_subdirectory(datastore)
