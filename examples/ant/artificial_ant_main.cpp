/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <iostream>

#include <gpm/gpm.hpp>
#include "santa_fe_board.hpp"
#include "ant_simulation.hpp"


namespace ant
{
    
struct move;
struct right;
struct left;
struct if_food_ahead;
template<int nodeCount, typename CTString> struct prog;

using prog2 = prog<2, gpm::NodeToken<'p','2'>>;
using prog3 = prog<3, gpm::NodeToken<'p','3'>>;

using ant_nodes = boost::variant<
    boost::recursive_wrapper<move>, 
    boost::recursive_wrapper<left>, 
    boost::recursive_wrapper<right>, 
    boost::recursive_wrapper<if_food_ahead>, 
    boost::recursive_wrapper<prog2>, 
    boost::recursive_wrapper<prog3>
>;




template<int NodeCount, typename CTString>
struct prog : public gpm::BaseNode<ant_nodes, NodeCount, CTString>
{
    using prog::BaseNode::BaseNode;
};

struct move : public gpm::BaseNode<ant_nodes, 0, gpm::NodeToken<'m'>> {};

struct right : public gpm::BaseNode<ant_nodes, 0, gpm::NodeToken<'r'>>{};

struct left : public gpm::BaseNode<ant_nodes, 0, gpm::NodeToken<'l'>>{};

struct if_food_ahead : public gpm::BaseNode<ant_nodes, 2, gpm::NodeToken<'i', 'f'>>
{
    using if_food_ahead::BaseNode::BaseNode;

    template<bool c>
    constexpr ant_nodes const & get() const
    {
        return nodes[c ? 0 : 1];
    }
    constexpr ant_nodes const & get(bool b) const
    {
        return b ? get<true>() : get<false>();
    }
    

};



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
                    board[x][y] = santa_fe::board1[x][y] == 'X' ? sim::BoardState::food : sim::BoardState::empty;
                }
            }
        }
    };
            
    auto antBoardSimVisitor = AntBoardSimulationVisitor{antBoardSimulation};
            
    char const * optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
    auto optAnt = gpm::factory<ant_nodes>(gpm::RPNToken_iterator{optimalAntRPNdef});
    
    while(!antBoardSimulation.is_finish())
    {
        antBoardSimulation.get_board_as_str([&](std::string const & s){ std::cout << s << "\n"; } );
        boost::apply_visitor(antBoardSimVisitor, optAnt);
        std::cout << "######\n";
    }
    antBoardSimulation.get_board_as_str([&](std::string const & s){ std::cout << s << "\n"; } );
    
    return 0;
}

