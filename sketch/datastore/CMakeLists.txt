add_executable(staticdatastore staticdatastore_main.cpp) 
set_target_properties(staticdatastore PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(staticdatastore PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(staticdatastore fmt::fmt)
