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
#include <gpm/utils/fmtutils.hpp>


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


class CPPStypePrinter : public boost::static_visitor<std::string>
{
public:
    std::string vistorName_;
    int indent_ = 0;
    int indentSize = 4;
    
    
    CPPStypePrinter(std::string const & vistorName, int indent)
        :vistorName_{vistorName}, indent_{indent}
    {
    }

    std::string operator()(if_food_ahead const & c) const
    {
        auto indent = fmt::format("{:<{}}", "", indent_*indentSize);
        return gpm::utils::formatNamed(
R"""(
{indent}if({vistorName}.is_food_in_front()){{
{true_branch}
{indent}}}else{{
{false_branch}
{indent}}}
)"""
            , "vistorName", vistorName_
            , "true_branch", boost::apply_visitor(CPPStypePrinter{vistorName_, indent_+1}, c.get<true>())
            , "false_branch", boost::apply_visitor(CPPStypePrinter{vistorName_, indent_+1}, c.get<false>())
            , "indent", indent
        );
    }
    
    template<typename T>
    std::string operator()(T t) const
    {
        std::string res;
        if constexpr(t.nodes.size() == 0)
        {
            res += fmt::format( "{:<{}}{}.{}();\n", "", indent_*indentSize, vistorName_, MethodName<T>::name);
        }
        for(auto & n: t.nodes)
            res += boost::apply_visitor(*this, n);
        return res;
    }
    
};

class VariantStylePrinter : public boost::static_visitor<std::string>
{
public:
    int indent_ = 0;
    int indentSize = 4;
    
    
    VariantStylePrinter(int indent)
        :indent_{indent}
    {
    }

    std::string operator()(if_food_ahead const & c) const
    {
        auto indent = fmt::format("{:<{}}", "", indent_*indentSize);
        return gpm::utils::formatNamed(
R"""(
{indent}{vistorName}{{
{indent}  {true_branch}
{indent}, {false_branch}
{indent}}}
)"""
    , "vistorName", boost::typeindex::type_id_runtime(c).pretty_name()
    , "true_branch", boost::apply_visitor(VariantStylePrinter{indent_+1}, c.get<true>())
    , "false_branch", boost::apply_visitor(VariantStylePrinter{indent_+1}, c.get<true>())
    , "indent", indent  
    
        );
    }
    
    template<typename T>
    std::string operator()(T t) const
    {
        std::string res;
        if constexpr(t.nodes.size() == 0)
        {
            res += fmt::format("{:<{}}{}{{}}\n", "", indent_*indentSize, boost::typeindex::type_id_runtime(t).pretty_name());
        }
        else
        {
            res += fmt::format("{:<{}}{}{{\n", "", indent_*indentSize, boost::typeindex::type_id_runtime(t).pretty_name());
            auto delimi = " ";
            for(auto & n: t.nodes)
            {
                (res += delimi) += boost::apply_visitor(VariantStylePrinter{indent_+1}, n);
                delimi = ",";
            }
            res += fmt::format("{:<{}}}}\n", "", indent_*indentSize);
        }
        return res;
    }    
};


std::ostream & toVariantStyle(std::ostream & out, ant_nodes const & ant)
{
    out << "ant::ant_nodes{\n";
    out << boost::apply_visitor(VariantStylePrinter{1}, ant);
    out << "}\n";
    return out;
}

}



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
    int maxHeight = 11;
    std::random_device rd;
    auto bgen = gpm::BasicGenerator<ant::ant_nodes>{minHeight, maxHeight, rd()};
    auto randomAnt = bgen();
    
    //boost::apply_visitor(ant::VariantStylePrinter{std::cout, 0}, fooAnt);
                 
    std::ofstream fout("ant_simulation_benchmark_generated_functions.cpp");
    std::ostringstream vairantStyleStream;
    toVariantStyle(vairantStyleStream, randomAnt);
    

    auto antBoardSimName = "antBoardSim";
    auto randomAntFlatStyle =
    boost::apply_visitor(ant::CPPStypePrinter{ antBoardSimName, 2}, randomAnt);

    
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
        , randomAntFlatStyle
    );

/*
touch ../examples/ant/ant_simulation_generate.cpp
ninja examples/ant/ant_simulation_generate && examples/ant/ant_simulation_generate
cp ant_simulation_benchmark_generated_functions.cpp ../examples/ant/
touch ../examples/ant/ant_simulation_benchmark.cpp
ninja ant_simulation_benchmark && examples/ant/ant_simulation_benchmark

 */
}
