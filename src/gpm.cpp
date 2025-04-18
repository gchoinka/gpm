#include "gpm/gpm.hpp"
#include <iostream>
#include <format>

namespace gpm {
    void print_hello(std::string_view name) {
        // Using C++23 std::format
        std::cout << std::format("Hello, {}!\n", name);
    }
    
    auto get_greeting(std::string_view name) -> std::string {
        return std::format("Hello, {}!", name);
    }
}
