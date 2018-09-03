/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <iostream>


#include "santa_fe_board.hpp"
#include "ant_simulation.hpp"
#include "nodes.hpp"


namespace ant {

template<typename AntBoardSimType>
class AntBoardSimulationVisitor : public boost::static_visitor<void>
{
public:
    AntBoardSimulationVisitor(AntBoardSimType & sim):sim_{sim} {} 

    void operator()(move) const
    {
        sim_.move();
    }
    
    void operator()(left) const
    {
        sim_.left(); 
    }
    
    void operator()(right) const
    {
        sim_.right();
    }
    
    void operator()(if_food_ahead const & c) const
    {
        boost::apply_visitor( *this, c.get(sim_.is_food_in_front()));
    }

    template<int NodeCount, typename CTString>
    void operator()(prog<NodeCount, CTString> const & b) const
    {
        for(auto const & n: b.nodes)
            boost::apply_visitor( *this, n );
    }
    
private:
    AntBoardSimType & sim_;
};

}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char const ** argv)
{
    using namespace ant;
    auto output = false;
    auto max_steps = 400;
    auto max_food = 89;
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
            
    char const * optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
    auto optAnt = gpm::factory<ant_nodes>(gpm::RPNToken_iterator{optimalAntRPNdef});
    
    while(!antBoardSimulation.is_finish())
    {
        antBoardSimulation.get_board_as_str([&](std::string const & s){ output && std::cout << s << "\n"; } );
        boost::apply_visitor(antBoardSimVisitor, optAnt);
        output && std::cout << "######\n";
    }
    antBoardSimulation.get_board_as_str([&](std::string const & s){ output && std::cout << s << "\n"; } );
    
    int minHeight = 1;
    int maxHeight = 7;
    std::random_device rd;
    auto bgen = gpm::BasicGenerator<ant_nodes>{minHeight, maxHeight, rd()};
    auto foo = bgen();
    

    std::cout << boost::apply_visitor([](auto val) { return boost::typeindex::type_id_runtime(val).pretty_name(); }, foo)  << " " <<
                 boost::apply_visitor(gpm::RPNPrinter<std::string>{}, foo) << "\n";
    return 0;
}

