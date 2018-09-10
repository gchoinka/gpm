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
#include <memory>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <gpm/utils/fmtutils.hpp>


#include "common/santa_fe_board.hpp"
#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/visitor.hpp"

#include "nodes_opp.hpp"



  


class AsOOPNotation : public boost::static_visitor<std::string>
{
    std::string simulationName_;
    int indent_ = 0;
    int const indentSize = 4;
    std::string indentStr_;
    
public:    
    AsOOPNotation(std::string const & simulationName, int indent)
        :simulationName_{simulationName}, indent_{indent}, indentStr_{fmt::format("{:<{}}", "", indent_*indentSize)}
    {
    }

    std::string operator()(ant::if_food_ahead const & c) const
    {
        return gpm::utils::formatNamed(
R"""(
{indent}std::make_unique<antoop::IfFoodAhead<decltype({simulationName})>>(
{indent}  {true_branch}
{indent}, {false_branch}
{indent})
)"""
    , "simulationName", simulationName_
    , "true_branch", boost::apply_visitor(AsOOPNotation{simulationName_, indent_+1}, c.get<true>())
    , "false_branch", boost::apply_visitor(AsOOPNotation{simulationName_, indent_+1}, c.get<false>())
    , "indent", indentStr_  
    
        );
    }
    
    struct AntNodeToClassName
    {
        static char const * name(ant::move) { return "Move"; }
        static char const * name(ant::left) { return "Left"; }
        static char const * name(ant::right) { return "Right"; }
        static char const * name(ant::prog2) { return "Prog2"; }
        static char const * name(ant::prog3) { return "Prog3"; }
    };  
    
    template<typename T>
    std::string operator()(T t) const
    {
        std::string res;
        if constexpr(t.nodes.size() != 0)
        {
            auto delimi = "\n";
            for(auto & n: t.nodes)
            {
                (res += delimi) += boost::apply_visitor(AsOOPNotation{simulationName_, indent_+1}, n);
                delimi = ",";
            }
            res += "\n";
        }
        
        res = gpm::utils::formatNamed("{indent}std::make_unique<antoop::{nodeName}<decltype({simulationName})>>({nodeChildren})\n"
                , "indent", indentStr_  
                , "nodeName", AntNodeToClassName::name(t)
                , "nodeChildren", res
                , "simulationName", simulationName_
        );
        return res;
    }    
};

 

class AsCPPFixedNotation : public boost::static_visitor<std::string>
{

    std::string simulationName_;
    int indent_ = 0;
    int const indentSize = 4;
    std::string indentStr_;
    
 public:   
    AsCPPFixedNotation(std::string const & simulationName, int indent)
        :simulationName_{simulationName}, indent_{indent}, indentStr_{fmt::format("{:<{}}", "", indent_*indentSize)}
    {
    }

    std::string operator()(ant::if_food_ahead const & c) const
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
            , "true_branch", boost::apply_visitor(AsCPPFixedNotation{simulationName_, indent_+1}, c.get<true>())
            , "false_branch", boost::apply_visitor(AsCPPFixedNotation{simulationName_, indent_+1}, c.get<false>())
            , "indent", indentStr_
        );
    }
    
    struct AntNodeToSimulationMethodName
    {
        static char const * name(ant::move) { return "move"; }
        static char const * name(ant::left) { return "left"; }
        static char const * name(ant::right) { return "right"; }
    };    

    
    template<typename T>
    std::string operator()(T t) const
    {
        std::string res;
        if constexpr(t.nodes.size() == 0)
        {
            res += gpm::utils::formatNamed("{indent}{simulationName}.{methodName}();\n" 
                                             , "simulationName", simulationName_
                                             , "methodName", AntNodeToSimulationMethodName::name(t)
                                             , "indent", indentStr_
            );
        }
        for(auto & n: t.nodes)
            res += boost::apply_visitor(AsCPPFixedNotation{simulationName_, indent_+1}, n);
        return res;
    }
};



class AsCPPFixedWithVisitorNotation : public boost::static_visitor<std::string>
{

    std::string simulationName_;
    std::string visitorName_;
    int indent_ = 0;
    int const indentSize = 4;
    std::string indentStr_;
public:    
    
    AsCPPFixedWithVisitorNotation(std::string const & simulationName, std::string const & visitorName, int indent)
        :simulationName_{simulationName}, visitorName_{visitorName}, indent_{indent}, indentStr_{fmt::format("{:<{}}", "", indent_*indentSize)}
    {
    }

    std::string operator()(ant::if_food_ahead const & c) const
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
            , "true_branch", boost::apply_visitor(AsCPPFixedWithVisitorNotation{simulationName_, visitorName_, indent_+1}, c.get<true>())
            , "false_branch", boost::apply_visitor(AsCPPFixedWithVisitorNotation{simulationName_, visitorName_, indent_+1}, c.get<false>())
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
            res += boost::apply_visitor(AsCPPFixedWithVisitorNotation{simulationName_, visitorName_, indent_+1}, n);
        return res;
    }
    
};


class AsRecursiveVariantNotation : public boost::static_visitor<std::string>
{
public:
    int indent_ = 0;
    int const indentSize = 4;
    std::string indentStr_;
    
    
    AsRecursiveVariantNotation(int indent)
        :indent_{indent}, indentStr_{fmt::format("{:<{}}", "", indent_*indentSize)}
    {
    }

    std::string operator()(ant::if_food_ahead const & c) const
    {
        return gpm::utils::formatNamed(
R"""(
{indent}{nodeName}{{
{indent}  {true_branch}
{indent}, {false_branch}
{indent}}}
)"""
    , "nodeName", boost::typeindex::type_id_runtime(c).pretty_name()
    , "true_branch", boost::apply_visitor(AsRecursiveVariantNotation{indent_+1}, c.get<true>())
    , "false_branch", boost::apply_visitor(AsRecursiveVariantNotation{indent_+1}, c.get<false>())
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
                (res += delimi) += boost::apply_visitor(AsRecursiveVariantNotation{indent_+1}, n);
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






int main()
{        
    int minHeight = 1;
    int maxHeight = 7;
    std::random_device rd;

    auto ant = gpm::BasicGenerator<ant::ant_nodes>{minHeight, maxHeight, rd()}();
    
    auto antBoardSimName = "antBoardSim";
    auto antBoardSimVisitorName = "antBoardSimVisitor";
    
    
    auto recursiveVariantNotation = boost::apply_visitor(AsRecursiveVariantNotation{1}, ant);
    auto cppFixedNotation = boost::apply_visitor(AsCPPFixedNotation{antBoardSimName, 2}, ant);
    auto cppFixedWithVisitorNotation = boost::apply_visitor(AsCPPFixedWithVisitorNotation{antBoardSimName, antBoardSimVisitorName, 2}, ant);
    auto oopNotation = boost::apply_visitor(AsOOPNotation{antBoardSimName, 2}, ant);
    
    auto antRPN = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, ant);
    
    std::cout << gpm::utils::formatNamed( 
R"""(

static char const antRPNString[] = {{"{antRPN}"}};    

template<typename AntBoardSimT>
static int recursiveVariantTreeFromString(AntBoardSimT {antBoardSimName})
{{    
    auto optAnt = gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{{antRPNString}});
            
    auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{{antBoardSimName}}};
            
    while(!{antBoardSimName}.is_finish())
    {{
        boost::apply_visitor(antBoardSimVisitor, optAnt);
    }}
    return {antBoardSimName}.score(); 
}}

static void BM_recursiveVariantTreeFromString(benchmark::State& state) 
{{
    for (auto _ : state) {{state.counters["score"] = recursiveVariantTreeFromString(getAntBoardSim());}}
}}
BENCHMARK(BM_recursiveVariantTreeFromString); 
    

template<typename AntBoardSimT>
static int recursiveVariantTree(AntBoardSimT {antBoardSimName})
{{    
    auto optAnt = ant::ant_nodes{{{recursiveVariantNotation}}};
            
    auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{{antBoardSimName}}};
            
    while(!{antBoardSimName}.is_finish())
    {{
        boost::apply_visitor(antBoardSimVisitor, optAnt);
    }}
    return {antBoardSimName}.score(); 
}}

static void BM_recursiveVariantTree(benchmark::State& state) 
{{
    for (auto _ : state) {{state.counters["score"] = recursiveVariantTree(getAntBoardSim());}}
}}
BENCHMARK(BM_recursiveVariantTree); 
    
template<typename AntBoardSimT>
int cppFixedTree(AntBoardSimT {antBoardSimName})
{{
    while(!{antBoardSimName}.is_finish()){{
    {cppFixedNotation}
    }}
    return {antBoardSimName}.score();
}}
    
static void BM_cppFixedTree(benchmark::State& state) 
{{
    for (auto _ : state) {{state.counters["score"] = cppFixedTree(getAntBoardSim());}}
}}
BENCHMARK(BM_cppFixedTree);  
 
    
template<typename AntBoardSimT>
int cppFixedWithVisitor(AntBoardSimT {antBoardSimName})
{{                
    auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{{antBoardSimName}}};
            
    while(!{antBoardSimName}.is_finish())
    {{
        {cppFixedWithVisitorNotation}
    }}
    return {antBoardSimName}.score(); 
}}

static void BM_cppFixedWithVisitor(benchmark::State& state) 
{{
    for (auto _ : state) {{state.counters["score"] = cppFixedWithVisitor(getAntBoardSim());}}
}}
BENCHMARK(BM_cppFixedWithVisitor);  

template<typename AntBoardSimT>
int oopTree(AntBoardSimT {antBoardSimName})
{{                
    auto oopTree = {oopNotation};
            
    while(!{antBoardSimName}.is_finish())
    {{
        (*oopTree)({antBoardSimName});
    }}
    return {antBoardSimName}.score(); 
}}

static void BM_oopTree(benchmark::State& state) 
{{
    for (auto _ : state) {{state.counters["score"] = oopTree(getAntBoardSim());}}
}}
BENCHMARK(BM_oopTree);  
    
)"""
        , "antRPN", antRPN
        , "antBoardSimName", antBoardSimName
        , "recursiveVariantNotation", recursiveVariantNotation
        , "cppFixedNotation", cppFixedNotation
        , "cppFixedWithVisitorNotation", cppFixedWithVisitorNotation
        , "oopNotation", oopNotation
    );
}
