add_executable(artificial_ant artificial_ant_main.cpp) 
set_target_properties(artificial_ant PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(artificial_ant PRIVATE -Wall -Wextra -pedantic -Werror)
