
#include <chrono>
#include <iostream>
        
#include "santa_fe_board.hpp"
#include "ant_simulation.hpp"
#include "nodes.hpp"
#include "visitor.hpp"
#include "ant_board_simulation.hpp"


template<typename AntSimT>
int dynamicTree(AntSimT antSim)
{    
    auto optAnt = ant::ant_nodes{
    ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
         ant::if_food_ahead{
             ant::if_food_ahead{
                 ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
                     ant::move{}
,                    ant::move{}
,                    ant::left{}
                }
,                ant::if_food_ahead{
                     ant::right{}
,                    ant::right{}
                }
            }
,            ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                 ant::if_food_ahead{
                     ant::left{}
,                    ant::right{}
                }
,                ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
                     ant::right{}
,                    ant::move{}
,                    ant::right{}
                }
            }
        }
,        ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
             ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                 ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::left{}
,                    ant::right{}
                }
,                ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::right{}
,                    ant::move{}
                }
            }
,            ant::if_food_ahead{
                 ant::if_food_ahead{
                     ant::move{}
,                    ant::left{}
                }
,                ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::left{}
,                    ant::right{}
                }
            }
,            ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                 ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::move{}
,                    ant::left{}
                }
,                ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::right{}
,                    ant::left{}
                }
            }
        }
,        ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
             ant::if_food_ahead{
                 ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::right{}
,                    ant::move{}
                }
,                ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
                     ant::move{}
,                    ant::right{}
,                    ant::right{}
                }
            }
,            ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                 ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
                     ant::move{}
,                    ant::move{}
                }
,                ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
                     ant::right{}
,                    ant::right{}
,                    ant::right{}
                }
            }
        }
    }
}
;
            
    auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antSim};
            
    while(!antSim.is_finish())
    {
        boost::apply_visitor(antBoardSimVisitor, optAnt);
    }
    return antSim.score(); 
}
ANT_ADD_TO_BENCHMARK(dynamicTree<decltype(getAntBoardSim())>)
        
template<typename AntSimT>
int staticTree(AntSimT antSim)
{
    while(!antSim.is_finish()){
        if(antSim.is_food_in_front()){
            if(antSim.is_food_in_front()){
                antSim.move();
                antSim.move();
                antSim.left();
            }else{
                if(antSim.is_food_in_front()){
                    antSim.right();
                }else{
                    antSim.right();
                }
            }
        }else{
            if(antSim.is_food_in_front()){
                antSim.left();
            }else{
                antSim.right();
            }
            antSim.right();
            antSim.move();
            antSim.right();
        }
        antSim.left();
        antSim.right();
        antSim.right();
        antSim.move();
        if(antSim.is_food_in_front()){
            if(antSim.is_food_in_front()){
                antSim.move();
            }else{
                antSim.left();
            }
        }else{
            antSim.left();
            antSim.right();
        }
        antSim.move();
        antSim.left();
        antSim.right();
        antSim.left();
        if(antSim.is_food_in_front()){
            antSim.right();
            antSim.move();
        }else{
            antSim.move();
            antSim.right();
            antSim.right();
        }
        antSim.move();
        antSim.move();
        antSim.right();
        antSim.right();
        antSim.right();
    }
return antSim.score();
}


ANT_ADD_TO_BENCHMARK(staticTree<decltype(getAntBoardSim())>) 

int main()
{
    int result = 0;
    auto antSim = getAntBoardSim();
    for(auto fun:toBenchmark)
    {
        auto d8 = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < 1000; ++i)
        {
            result += fun(antSim);
        }
        auto d9 = std::chrono::high_resolution_clock::now();
        std::cout << boost::typeindex::type_id_runtime(fun).pretty_name() <<  ": " << (d9 - d8).count() << "\n";
    }
        
    return result == 0 ;
}
