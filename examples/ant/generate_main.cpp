/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <iostream>
#include <chrono>
#include <fstream>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <gpm/utils/fmtutils.hpp>


#include "common/santa_fe_board.hpp"
#include "common/ant_simulation.hpp"
#include "common/nodes.hpp"
#include "common/visitor.hpp"


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
    int const indentSize = 4;
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

class CPPVistorNotationPrinter : public boost::static_visitor<std::string>
{
public:
    std::string simulationName_;
    std::string visitorName_;
    int indent_ = 0;
    int const indentSize = 4;
    std::string indentStr_;
    
    
    CPPVistorNotationPrinter(std::string const & simulationName, std::string const & visitorName, int indent)
        :simulationName_{simulationName}, visitorName_{visitorName}, indent_{indent}, indentStr_{fmt::format("{:<{}}", "", indent_*indentSize)}
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
            , "true_branch", boost::apply_visitor(CPPVistorNotationPrinter{simulationName_, visitorName_, indent_+1}, c.get<true>())
            , "false_branch", boost::apply_visitor(CPPVistorNotationPrinter{simulationName_, visitorName_, indent_+1}, c.get<false>())
            , "indent", indentStr_
        );
    }
    
    template<typename T>
    std::string operator()(T t) const
    {
        auto nodeType = boost::typeindex::type_id_runtime(t).pretty_name();
        std::string res;
        if constexpr(t.nodes.size() == 0)
        {
            res += gpm::utils::formatNamed("{indent}{visitorName}({nodeType}{{}});\n" 
                                             , "visitorName", visitorName_
                                             , "nodeType", nodeType
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
    int const indentSize = 4;
    std::string indentStr_;
    
    
    VariantStylePrinter(int indent)
        :indent_{indent}, indentStr_{fmt::format("{:<{}}", "", indent_*indentSize)}
    {
    }

    std::string operator()(if_food_ahead const & c) const
    {
        return gpm::utils::formatNamed(
R"""(
{indent}{nodeName}{{
{indent}  {true_branch}
{indent}, {false_branch}
{indent}}}
)"""
    , "nodeName", boost::typeindex::type_id_runtime(c).pretty_name()
    , "true_branch", boost::apply_visitor(VariantStylePrinter{indent_+1}, c.get<true>())
    , "false_branch", boost::apply_visitor(VariantStylePrinter{indent_+1}, c.get<true>())
    , "indent", indentStr_  
    
        );
    }
    
    template<typename T>
    std::string operator()(T t) const
    {
        std::string res;
        if constexpr(t.nodes.size() != 0)
        {
            auto delimi = "\n";
            for(auto & n: t.nodes)
            {
                (res += delimi) += boost::apply_visitor(VariantStylePrinter{indent_+1}, n);
                delimi = ",";
            }
            res += "\n";
        }
        
        res = gpm::utils::formatNamed("{indent}{nodeName}{{{nodeChildren}}}\n"
                , "indent", indentStr_  
                , "nodeName", boost::typeindex::type_id_runtime(t).pretty_name()
                , "nodeChildren", res
        );
        return res;
    }    
};


}



int main()
{        
    int minHeight = 1;
    int maxHeight = 7;
    std::random_device rd;
    auto bgen = gpm::BasicGenerator<ant::ant_nodes>{minHeight, maxHeight, rd()};
    auto randomAnt = bgen();
    

    auto randomAntVairantNotation = boost::apply_visitor(ant::VariantStylePrinter{1}, randomAnt);
    auto antBoardSimName = "antBoardSim";
    auto randomAntCPPNotation = boost::apply_visitor(ant::CPPStypePrinter{antBoardSimName, 2}, randomAnt);
    auto antBoardSimVisitorName = "antBoardSimVisitor";
    auto randomAntCPPVisitorNotation = boost::apply_visitor(ant::CPPVistorNotationPrinter{antBoardSimName, antBoardSimVisitorName, 2}, randomAnt);
    
    std::cout << gpm::utils::formatNamed( 
R"""(
template<typename AntBoardSimT>
static int dynamicTree(AntBoardSimT {antBoardSimName})
{{    
    auto optAnt = ant::ant_nodes{{{VairantNotation}}};
            
    auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{{antBoardSimName}}};
            
    while(!{antBoardSimName}.is_finish())
    {{
        boost::apply_visitor(antBoardSimVisitor, optAnt);
    }}
    return {antBoardSimName}.score(); 
}}

static void BM_dynamicTree(benchmark::State& state) 
{{
    for (auto _ : state) {{dynamicTree(getAntBoardSim());}}
}}
BENCHMARK(BM_dynamicTree); 
    
template<typename AntBoardSimT>
int staticTree(AntBoardSimT {antBoardSimName})
{{
    while(!{antBoardSimName}.is_finish()){{
    {CPPNotation}
    }}
    return {antBoardSimName}.score();
}}
    
static void BM_staticTree(benchmark::State& state) 
{{
    for (auto _ : state) {{staticTree(getAntBoardSim());}}
}}
BENCHMARK(BM_staticTree);  
 
    
template<typename AntBoardSimT>
int staticWithVistorTree(AntBoardSimT {antBoardSimName})
{{                
    auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{{antBoardSimName}}};
            
    while(!{antBoardSimName}.is_finish())
    {{
        {CPPVisitorNotation}
    }}
    return {antBoardSimName}.score(); 
}}

static void BM_staticWithVistorTree(benchmark::State& state) 
{{
    for (auto _ : state) {{staticWithVistorTree(getAntBoardSim());}}
}}
BENCHMARK(BM_staticWithVistorTree);    
    
)"""
        , "antBoardSimName", antBoardSimName
        , "VairantNotation", randomAntVairantNotation
        , "CPPNotation", randomAntCPPNotation
        , "CPPVisitorNotation", randomAntCPPVisitorNotation
    );
}
