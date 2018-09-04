
#include "santa_fe_board.hpp"
#include "ant_simulation.hpp"
#include "nodes.hpp"
#include "visitor.hpp"
#include <chrono>
#include <iostream>


int dynamicTree()
{
    using namespace ant;
    auto max_steps = 400;
    auto max_food = 89;
    
    auto optAnt = ant::ant_nodes{
    ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
         ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
             ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                 ant::if_food_ahead{
                     ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                         ant::left{}
,                        ant::left{}
                    }
,                    ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                         ant::move{}
,                        ant::left{}
                    }
                }
,                ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                         ant::left{}
,                        ant::move{}
                    }
,                    ant::if_food_ahead{
                         ant::right{}
,                        ant::left{}
                    }
                }
            }
,            ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                 ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::if_food_ahead{
                         ant::right{}
,                        ant::left{}
                    }
,                    ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                         ant::move{}
,                        ant::right{}
                    }
                }
,                ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                         ant::move{}
,                        ant::move{}
                    }
,                    ant::if_food_ahead{
                         ant::right{}
,                        ant::left{}
                    }
                }
            }
        }
,        ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
             ant::left{}
,            ant::left{}
,            ant::move{}
        }
,        ant::if_food_ahead{
             ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
                 ant::move{}
,                ant::move{}
,                ant::move{}
            }
,            ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                 ant::move{}
,                ant::move{}
            }
        }
    }
}
;
    auto antBoardSimulation = sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
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
            
    auto antBoardSimVisitor = AntBoardSimulationVisitor{antBoardSimulation};
            
    while(!antBoardSimulation.is_finish())
    {
        boost::apply_visitor(antBoardSimVisitor, optAnt);
    }
    return antBoardSimulation.score(); 
}
        
int staticTree()
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
    while(!antSim.is_finish()){
        if(antSim.is_food_in_front()){
            antSim.left();
            antSim.left();
        }else{
            antSim.move();
            antSim.left();
        }
        antSim.left();
        antSim.move();
        if(antSim.is_food_in_front()){
            antSim.right();
        }else{
            antSim.left();
        }
        if(antSim.is_food_in_front()){
            antSim.right();
        }else{
            antSim.left();
        }
        antSim.move();
        antSim.right();
        antSim.move();
        antSim.move();
        if(antSim.is_food_in_front()){
            antSim.right();
        }else{
            antSim.left();
        }
        antSim.left();
        antSim.left();
        antSim.move();
        if(antSim.is_food_in_front()){
            antSim.move();
            antSim.move();
            antSim.move();
        }else{
            antSim.move();
            antSim.move();
        }
    }
return antSim.score();
}

        
int main()
{
    int result = 0;
    for(auto fun:{dynamicTree, staticTree})
    {
        auto d8 = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < 10; ++i)
        {
            result += fun();
        }
        auto d9 = std::chrono::high_resolution_clock::now();
        std::cout << boost::typeindex::type_id_runtime(fun).pretty_name() <<  ": " << (d9 - d8).count() << "\n";
    }
        
    return result;
}
