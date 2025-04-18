#pragma once
#include <string>
#include <string_view>

namespace gpm {
    // Using C++23 features like std::string_view
    void print_hello(std::string_view name);
    
    // C++23 feature: auto return type deduction
    auto get_greeting(std::string_view name) -> std::string;
}
