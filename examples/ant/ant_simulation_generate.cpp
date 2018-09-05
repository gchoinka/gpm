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
#include "visitor.hpp"

#include <chrono>
#include <fstream>

#include <fmt/format.h>
#include <fmt/ostream.h>


namespace ant { 


template<typename T>
struct MethodName
{
};
template<>
struct MethodName<ant::move>
{
    constexpr static char const * name = "move";
};
template<>
struct MethodName<ant::left>
{
    constexpr static char const * name = "left";
};
template<>
struct MethodName<ant::right>
{
    constexpr static char const * name = "right";
};


class CPPStypePrinter : public boost::static_visitor<void>
{
public:
    std::ostream & out_;
    std::string vistorName_;
    int indent_ = 0;
    int indentSize = 4;
    
    
    CPPStypePrinter(std::ostream & out, std::string const & vistorName, int indent)
        :out_{out}, vistorName_{vistorName}, indent_{indent}
    {
    }

    void operator()(if_food_ahead const & c) const
    {
        
        fmt::print(out_, "{:<{}}if({}.is_food_in_front()){{\n", "", indent_*indentSize, vistorName_);
        boost::apply_visitor(CPPStypePrinter{out_, vistorName_, indent_+1}, c.get<true>());
        fmt::print(out_, "{:<{}}}}else{{\n", "", indent_*indentSize);
        boost::apply_visitor(CPPStypePrinter{out_, vistorName_, indent_+1}, c.get<false>());
        fmt::print(out_, "{:<{}}}}\n", "", indent_*indentSize);
    }
    
    template<typename T>
    void operator()(T t) const
    {
        if constexpr(t.nodes.size() == 0)
        {
            fmt::print(out_, "{:<{}}{}.{}();\n", "", indent_*indentSize, vistorName_, MethodName<T>::name);
        }
        for(auto & n: t.nodes)
            boost::apply_visitor(*this, n);
    }
    
};

class VariantStylePrinter : public boost::static_visitor<void>
{
public:
    std::ostream & out_;
    int indent_ = 0;
    int indentSize = 4;
    
    
    VariantStylePrinter(std::ostream & out, int indent)
        :out_{out}, indent_{indent}
    {
    }

    void operator()(if_food_ahead const & c) const
    {
        
        fmt::print(out_, "{:<{}}{}{{\n", "", indent_*indentSize, boost::typeindex::type_id_runtime(c).pretty_name());
        out_ << " ";
        boost::apply_visitor(VariantStylePrinter{out_, indent_+1}, c.get<true>());
        out_ << ",";
        boost::apply_visitor(VariantStylePrinter{out_, indent_+1}, c.get<false>());
        fmt::print(out_, "{:<{}}}}\n", "", indent_*indentSize);
    }
    
    template<typename T>
    void operator()(T t) const
    {
        if constexpr(t.nodes.size() == 0)
        {
            fmt::print(out_, "{:<{}}{}{{}}\n", "", indent_*indentSize, boost::typeindex::type_id_runtime(t).pretty_name());
        }
        else
        {
            fmt::print(out_, "{:<{}}{}{{\n", "", indent_*indentSize, boost::typeindex::type_id_runtime(t).pretty_name());
            auto delimi = " ";
            for(auto & n: t.nodes)
            {
                out_ << delimi;
                boost::apply_visitor(VariantStylePrinter{out_, indent_+1}, n);
                delimi = ",";
            }
            fmt::print(out_, "{:<{}}}}\n", "", indent_*indentSize);
        }
    }    
};


std::ostream & toVariantStyle(std::ostream & out, ant_nodes const & ant)
{
    out << "ant::ant_nodes{\n";
    boost::apply_visitor(VariantStylePrinter{out, 1}, ant);
    out << "}\n";
    return out;
}

}

// // int dynamicTree()
// // {
// //     using namespace ant;
// //     auto max_steps = 400;
// //     auto max_food = 89;
// // //     char const * optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
// // //     auto optAnt = gpm::factory<ant_nodes>(gpm::RPNToken_iterator{optimalAntRPNdef});
// // //     toVariantStyle(std::cout, optAnt);
// //     auto optAnt = ant::ant_nodes{
// //         ant::if_food_ahead{
// //             ant::move{}
// //     ,        ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
// //                 ant::right{}
// //     ,            ant::prog<2, gpm::NodeToken<(char)112, (char)50> >{
// //                     ant::if_food_ahead{
// //                         ant::if_food_ahead{
// //                             ant::move{}
// //     ,                        ant::right{}
// //                         }
// //     ,                    ant::prog<3, gpm::NodeToken<(char)112, (char)51> >{
// //                             ant::left{}
// //     ,                        ant::left{}
// //     ,                        ant::if_food_ahead{
// //                                 ant::move{}
// //     ,                            ant::right{}
// //                             }
// //                         }
// //                     }
// //     ,                ant::move{}
// //                 }
// //             }
// //         }
// //     };
// // 
// //     auto antBoardSimulation = sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
// //         max_steps,
// //         max_food,
// //         sim::Pos2d{0,0}, 
// //         sim::Direction::east,
// //         [](sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>::FieldType & board){
// //             for(size_t x = 0; x < board.size(); ++x)
// //             {
// //                 for(size_t y = 0; y < board[x].size(); ++y)
// //                 {
// //                     board[x][y] = santa_fe::board[x][y] == 'X' ? sim::BoardState::food : sim::BoardState::empty;
// //                 }
// //             }
// //         }
// //     };
// //             
// //     auto antBoardSimVisitor = AntBoardSimulationVisitor{antBoardSimulation};
// //             
// //     while(!antBoardSimulation.is_finish())
// //     {
// //         boost::apply_visitor(antBoardSimVisitor, optAnt);
// //     }
// //     return antBoardSimulation.score(); 
// // }
// // 
// // int staticTree()
// // {
// //     using namespace ant;
// //     auto max_steps = 400;
// //     auto max_food = 89;
// //     auto antSim = sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
// //         max_steps,
// //         max_food,
// //         sim::Pos2d{0,0}, 
// //         sim::Direction::east,
// //         [](sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>::FieldType & board){
// //             for(size_t x = 0; x < board.size(); ++x)
// //             {
// //                 for(size_t y = 0; y < board[x].size(); ++y)
// //                 {
// //                     board[x][y] = santa_fe::board[x][y] == 'X' ? sim::BoardState::food : sim::BoardState::empty;
// //                 }
// //             }
// //         }
// //     };
// //     while(!antSim.is_finish()){
// //         if(antSim.is_food_in_front()){
// //             antSim.move();
// //         }else{
// //             antSim.right();
// //             if(antSim.is_food_in_front()){
// //                 if(antSim.is_food_in_front()){
// //                     antSim.move();
// //                 }else{
// //                     antSim.right();
// //                 }
// //             }else{
// //                 antSim.left();
// //                 antSim.left();
// //                 if(antSim.is_food_in_front()){
// //                     antSim.move();
// //                 }else{
// //                     antSim.right();
// //                 }
// //             }
// //             antSim.move();
// //         }
// //     }
// //     return antSim.score();
// // }

int main(__attribute__((unused)) int argc, __attribute__((unused)) char const ** argv)
{
//     int result = 0;
//     for(auto fun:{dynamicTree, staticTree})
//     {
//         auto d8 = std::chrono::high_resolution_clock::now();
//         for(int i = 0; i < 10000; ++i)
//         {
//             result += fun();
//         }
//         auto d9 = std::chrono::high_resolution_clock::now();
//         std::cout << boost::typeindex::type_id_runtime(fun).pretty_name() <<  ": " << (d9 - d8).count() << std::endl;
//     }
    
    int minHeight = 1;
    int maxHeight = 10;
    std::random_device rd;
    auto bgen = gpm::BasicGenerator<ant::ant_nodes>{minHeight, maxHeight, rd()};
    auto randomAnt = bgen();
    
    //boost::apply_visitor(ant::VariantStylePrinter{std::cout, 0}, fooAnt);
                 
    std::ofstream fout("ant_simulation_benchmark_generated_functions.cpp");
    std::ostringstream vairantStyleStream;
    toVariantStyle(vairantStyleStream, randomAnt);
    

    auto antBoardSimName = "antBoardSim";
    std::ostringstream randomAntFlatStyleStream;
    boost::apply_visitor(ant::CPPStypePrinter{randomAntFlatStyleStream, antBoardSimName, 2}, randomAnt);

    
    fmt::print(fout, 
R"""(
template<typename AntBoardSimT>
int dynamicTree(AntBoardSimT {0})
{{    
    auto optAnt = {1};
            
    auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{{0}}};
            
    while(!{0}.is_finish())
    {{
        boost::apply_visitor(antBoardSimVisitor, optAnt);
    }}
    return {0}.score(); 
}}
ANT_ADD_TO_BENCHMARK(dynamicTree<decltype(getAntBoardSim())>)
        
template<typename AntBoardSimT>
int staticTree(AntBoardSimT {0})
{{
    while(!{0}.is_finish()){{
    {2}
    }}
    return {0}.score();
}}

ANT_ADD_TO_BENCHMARK(staticTree<decltype(getAntBoardSim())>) 

)"""
        , antBoardSimName
        , vairantStyleStream.str()
        , randomAntFlatStyleStream.str()
    );

/*
touch ../examples/ant/ant_simulation_generate.cpp
ninja examples/ant/ant_simulation_generate && examples/ant/ant_simulation_generate
cp ant_simulation_benchmark_generated_functions.cpp ../examples/ant/
touch ../examples/ant/ant_simulation_benchmark.cpp
ninja ant_simulation_benchmark && examples/ant/ant_simulation_benchmark
 */
}
