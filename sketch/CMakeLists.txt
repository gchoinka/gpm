
add_executable(mapfmt mapfmt.cpp) 
set_target_properties(mapfmt PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(mapfmt PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(mapfmt fmt::fmt)

add_subdirectory(datastore)
