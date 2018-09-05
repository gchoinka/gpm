#include <functional>
#include <vector>

#include "santa_fe_board.hpp"
#include "ant_simulation.hpp"
#include "nodes.hpp"



ant::sim::AntBoardSimulationStaticSize<ant::santa_fe::x_size, ant::santa_fe::y_size> getAntBoardSim()
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

namespace 
{
std::vector<std::function<int(decltype(getAntBoardSim()))>> toBenchmark;

template<typename T>
int addToBenchmark(T t)
{
   toBenchmark.push_back(t); 
   return 0;
}

}
#define ANT_ADD_TO_BENCHMARK_CONCAT_IMP( x, y ) x##y
#define ANT_ADD_TO_BENCHMARK_CONCAT( x, y ) ANT_ADD_TO_BENCHMARK_CONCAT_IMP( x, y)
#define ANT_ADD_TO_BENCHMARK(functionName) namespace { int ANT_ADD_TO_BENCHMARK_CONCAT(ANT_ADD_TO_BENCHMARK_CONCAT(FOOBAR,__LINE__), __COUNTER__) = addToBenchmark(functionName); }

