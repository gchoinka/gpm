/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <chrono>
#include <iostream>
#include <functional>
#include <vector>

#include <benchmark/benchmark.h>

#include <gpm/gpm.hpp>
#include <gpm/io.hpp>


#include "common/santa_fe_board.hpp"
#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/visitor.hpp"

#include "nodes_opp.hpp"


decltype(auto) getAntBoardSim()
{
    using namespace ant;
    auto max_steps = 400;
    auto max_food = 89;
    auto antSim = sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
        max_steps,
        max_food,
        sim::Pos2d{0,0}, 
        sim::Direction::east,
        [](sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>::FieldType & board){
            for(size_t x = 0; x < board.size(); ++x)
            {
                for(size_t y = 0; y < board[x].size(); ++y)
                {
                    board[x][y] = santa_fe::board[x][y] == 'X' ? sim::BoardState::food : sim::BoardState::empty;
                }
            }
        }
    };
    return antSim;
}


#if __has_include("ant_simulation_benchmark_generated_functions.cpp")
    #include "ant_simulation_benchmark_generated_functions.cpp"
#else
    #pragma message "run artificial_ant_generate and copy ant_simulation_benchmark_generated_functions.cpp to the same folder, touch this file and then rerun this target"
#endif



BENCHMARK_MAIN();

