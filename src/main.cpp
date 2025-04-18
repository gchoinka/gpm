#include "gpm/gpm.hpp"
#include <iostream>
#include <print>

int main() {
    // Call the function from our hello library
    gpm::print_hello("World");
    
    // C++23 std::print feature
    std::print("Using C++23 std::print: {}\n", gpm::get_greeting("C++23"));
    
    return 0;
}
