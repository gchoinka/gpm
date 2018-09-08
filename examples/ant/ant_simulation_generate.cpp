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
    std::string simulationName_;
    int indent_ = 0;
    int indentSize = 4;
    std::string indentStr_;
    
    
    CPPStypePrinter(std::string const & simulationName, int indent)
        :simulationName_{simulationName}, indent_{indent}, indentStr_{fmt::format("{:<{}}", "", indent_*indentSize)}
    {
    }

    std::string operator()(if_food_ahead const & c) const
    {
        return gpm::utils::formatNamed(
R"""(
{indent}if({simulationName}.is_food_in_front()){{
{true_branch}
{indent}}}else{{
{false_branch}
{indent}}}
)"""
            , "simulationName", simulationName_
            , "true_branch", boost::apply_visitor(CPPStypePrinter{simulationName_, indent_+1}, c.get<true>())
            , "false_branch", boost::apply_visitor(CPPStypePrinter{simulationName_, indent_+1}, c.get<false>())
            , "indent", indentStr_
        );
    }
    
    template<typename T>
    std::string operator()(T t) const
    {
        std::string res;
        if constexpr(t.nodes.size() == 0)
        {
            res += gpm::utils::formatNamed("{indent}{simulationName}.{methodName}();\n" 
                                             , "simulationName", simulationName_
                                             , "methodName", MethodName<T>::name
                                             , "indent", indentStr_
            );
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


std::string toVariantStyle(ant_nodes const & ant)
{
    std::string res;
    res += "ant::ant_nodes{\n";
    res += boost::apply_visitor(VariantStylePrinter{1}, ant);
    res += "}\n";
    return res;
}

}



int main()
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
    int maxHeight = 7;
    std::random_device rd;
    auto bgen = gpm::BasicGenerator<ant::ant_nodes>{minHeight, maxHeight, rd()};
    auto randomAnt = bgen();
    

    auto vairantStyle = toVariantStyle(randomAnt);
    auto antBoardSimName = "antBoardSim";
    auto randomAntFlatStyle = boost::apply_visitor(ant::CPPStypePrinter{ antBoardSimName, 2}, randomAnt);

    
    fmt::print(std::cout, 
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
        , vairantStyle
        , randomAntFlatStyle
    );
}
