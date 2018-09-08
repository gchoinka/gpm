/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <chrono>
#include <iostream>
        
#include "santa_fe_board.hpp"
#include "ant_simulation.hpp"
#include "nodes.hpp"
#include "visitor.hpp"
#include "ant_board_simulation.hpp"


#if __has_include("ant_simulation_benchmark_generated_functions.cpp")
    #include "ant_simulation_benchmark_generated_functions.cpp"
#else
    #pragma message "run artificial_ant_generate and copy ant_simulation_benchmark_generated_functions.cpp to the same folder, touch this file and then rerun this target"
#endif


int main()
{
    int result = 0;
    auto antBoardSim = getAntBoardSim();
    for(auto antBoardFunction:toBenchmark)
    {
        auto d8 = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < 1000; ++i)
        {
            result += antBoardFunction(antBoardSim);
        }
        auto d9 = std::chrono::high_resolution_clock::now();
        std::cout << boost::typeindex::type_id_runtime(antBoardFunction).pretty_name() <<  ": " << (d9 - d8).count() << "\n";
    }
        
    return result == 0 ;
}
