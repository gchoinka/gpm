add_executable(artificial_ant artificial_ant_main.cpp) 
set_target_properties(artificial_ant PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)
target_compile_options(artificial_ant PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(artificial_ant fmt::fmt)



#add_executable(static_vs_dynamic_ant static_vs_dynamic_ant.cpp) 
#set_target_properties(static_vs_dynamic_ant PROPERTIES
#            CXX_STANDARD 17
#            CXX_EXTENSIONS OFF
#)
#target_compile_options(static_vs_dynamic_ant PRIVATE -Wall -Wextra -pedantic -Werror)
#
#target_link_libraries(static_vs_dynamic_ant fmt::fmt)
#

add_subdirectory(tests)


