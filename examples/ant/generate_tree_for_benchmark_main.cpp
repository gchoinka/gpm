/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <fstream>
#include <iostream>
#include <memory>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <gpm/io.hpp>
#include <gpm/utils/fmtutils.hpp>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"

#include "nodes_opp.hpp"

class AsOOPNotation : public boost::static_visitor<std::string> {
  std::string simulationName_;

 public:
  AsOOPNotation(std::string const& simulationName)
      : simulationName_{simulationName} {}

  std::string operator()(ant::if_food_ahead const& c) const {
    return gpm::utils::format(
        R"""(
            std::make_unique<antoop::IfFoodAhead<decltype({simulationName})>>(
                {true_branch}
                , {false_branch}
            )
            )""",
        gpm::utils::argsnamed, "simulationName", simulationName_, "true_branch",
        boost::apply_visitor(AsOOPNotation{simulationName_}, c.get<true>()),
        "false_branch",
        boost::apply_visitor(AsOOPNotation{simulationName_}, c.get<false>()));
  }

  struct AntNodeToClassName {
    static char const* name(ant::move&) { return "Move"; }
    static char const* name(ant::left&) { return "Left"; }
    static char const* name(ant::right&) { return "Right"; }
    static char const* name(ant::prog2&) { return "Prog2"; }
    static char const* name(ant::prog3&) { return "Prog3"; }
  };

  template <typename T>
  std::string operator()(T t) const {
    std::string res;
    if constexpr (t.nodes.size() != 0) {
      auto delimi = "\n";
      for (auto& n : t.nodes) {
        (res += delimi) += boost::apply_visitor(*this, n);
        delimi = ",";
      }
      res += "\n";
    }

    res = gpm::utils::format(
        "std::make_unique<antoop::{nodeName}<decltype({simulationName})>>({"
        "nodeChildren})\n",
        gpm::utils::argsnamed, "nodeName", AntNodeToClassName::name(t),
        "nodeChildren", res, "simulationName", simulationName_);
    return res;
  }
};

class AsCPPFixedNotation : public boost::static_visitor<std::string> {
  std::string simulationName_;

 public:
  AsCPPFixedNotation(std::string const& simulationName)
      : simulationName_{simulationName} {}

  std::string operator()(ant::if_food_ahead const& c) const {
    return gpm::utils::format(
        R"""(
                if({simulationName}.is_food_in_front()){{
                    {true_branch}
                }}else{{
                    {false_branch}
                }}
            )""",
        gpm::utils::argsnamed, "simulationName", simulationName_, "true_branch",
        boost::apply_visitor(*this, c.get<true>()), "false_branch",
        boost::apply_visitor(*this, c.get<false>()));
  }

  struct AntNodeToSimulationMethodName {
    static char const* name(ant::move&) { return "move"; }
    static char const* name(ant::left&) { return "left"; }
    static char const* name(ant::right&) { return "right"; }
  };

  template <typename T>
  std::string operator()(T t) const {
    std::string res;
    if constexpr (t.nodes.size() == 0) {
      res += gpm::utils::format("{simulationName}.{methodName}();\n",
                                gpm::utils::argsnamed, "simulationName",
                                simulationName_, "methodName",
                                AntNodeToSimulationMethodName::name(t));
    }
    for (auto& n : t.nodes) res += boost::apply_visitor(*this, n);
    return res;
  }
};

class AsCPPFixedWithVisitorNotation
    : public boost::static_visitor<std::string> {
  std::string simulationName_;
  std::string visitorName_;

 public:
  AsCPPFixedWithVisitorNotation(std::string const& simulationName,
                                std::string const& visitorName)
      : simulationName_{simulationName}, visitorName_{visitorName} {}

  std::string operator()(ant::if_food_ahead const& c) const {
    return gpm::utils::format(
        R"""(
                if({simulationName}.is_food_in_front()){{
                    {true_branch}
                }}else{{
                    {false_branch}
                }}
            )""",
        gpm::utils::argsnamed, "simulationName", simulationName_, "true_branch",
        boost::apply_visitor(*this, c.get<true>()), "false_branch",
        boost::apply_visitor(*this, c.get<false>()));
  }

  template <typename T>
  std::string operator()(T t) const {
    auto nodeType = boost::typeindex::type_id_runtime(t).pretty_name();
    std::string res;
    if constexpr (t.nodes.size() == 0) {
      res += gpm::utils::format("{visitorName}({nodeType}{{}});\n",
                                gpm::utils::argsnamed, "visitorName",
                                visitorName_, "nodeType", nodeType);
    }
    for (auto& n : t.nodes) res += boost::apply_visitor(*this, n);
    return res;
  }
};

class AsRecursiveVariantNotation : public boost::static_visitor<std::string> {
 public:
  std::string operator()(ant::if_food_ahead const& c) const {
    return gpm::utils::format(
        R"""(
                {nodeName}{{{true_branch}, {false_branch}}}
            )""",
        gpm::utils::argsnamed, "nodeName",
        boost::typeindex::type_id_runtime(c).pretty_name(), "true_branch",
        boost::apply_visitor(*this, c.get<true>()), "false_branch",
        boost::apply_visitor(*this, c.get<false>()));
  }

  template <typename T>
  std::string operator()(T t) const {
    std::string res;
    if constexpr (t.nodes.size() != 0) {
      auto delimi = "\n";
      for (auto& n : t.nodes) {
        (res += delimi) += boost::apply_visitor(*this, n);
        delimi = ",";
      }
      res += "\n";
    }

    res = gpm::utils::format("{nodeName}{{{nodeChildren}}}\n",
                             gpm::utils::argsnamed, "nodeName",
                             boost::typeindex::type_id_runtime(t).pretty_name(),
                             "nodeChildren", res);
    return res;
  }
};

decltype(auto) getRandomAnt()
{
  int minHeight = 1;
  int maxHeight = 9;
  // std::random_device rd;
  
  return gpm::BasicGenerator<ant::ant_nodes>{minHeight, maxHeight}();
}

decltype(auto) getOptAnt()
{
  char const* optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
  return gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{optimalAntRPNdef});
}

int main() {

  
  auto ant = getOptAnt();

  auto antBoardSimName = "antBoardSim";
  auto antBoardSimVisitorName = "antBoardSimVisitor";

  auto recursiveVariantNotation =
      boost::apply_visitor(AsRecursiveVariantNotation{}, ant);
  auto cppFixedNotation =
      boost::apply_visitor(AsCPPFixedNotation{antBoardSimName}, ant);
  auto cppFixedWithVisitorNotation = boost::apply_visitor(
      AsCPPFixedWithVisitorNotation{antBoardSimName, antBoardSimVisitorName},
      ant);
  auto oopNotation = boost::apply_visitor(AsOOPNotation{antBoardSimName}, ant);

  auto antRPN = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, ant);

  std::cout << gpm::utils::format(
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

  
template<typename AntBoardSimT>
int cppFixedTree(AntBoardSimT {antBoardSimName})
{{
    while(!{antBoardSimName}.is_finish()){{
    {cppFixedNotation}
    }}
    return {antBoardSimName}.score();
}}
    

    
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

    
template<typename AntBoardSimT>
int oopTreeFromString(AntBoardSimT {antBoardSimName})
{{                
    auto oopTree = antoop::factory<AntBoardSimT>(gpm::RPNToken_iterator{{antRPNString}});
            
    while(!{antBoardSimName}.is_finish())
    {{
        (*oopTree)({antBoardSimName});
    }}
    return {antBoardSimName}.score(); 
}}

    
template<typename AntBoardSimT>
int oopTreeFromExtString(AntBoardSimT {antBoardSimName})
{{                
    auto oopTree = antoop::factory<AntBoardSimT>(gpm::RPNToken_iterator{{ant::optimalAntRPNExt}});
            
    while(!{antBoardSimName}.is_finish())
    {{
        (*oopTree)({antBoardSimName});
    }}
    return {antBoardSimName}.score(); 
}}

template<typename AntBoardSimT>
decltype(auto) getAllTreeBenchmarks()
{{
  return std::make_tuple(
      std::make_tuple(&recursiveVariantTreeFromString<AntBoardSimT>, "recursiveVariantTreeFromString")
    , std::make_tuple(&recursiveVariantTree<AntBoardSimT>, "recursiveVariantTree")
    , std::make_tuple(&cppFixedTree<AntBoardSimT>, "cppFixedTree")
    , std::make_tuple(&cppFixedWithVisitor<AntBoardSimT>, "cppFixedWithVisitor")
    , std::make_tuple(&oopTree<AntBoardSimT>, "oopTree")
    , std::make_tuple(&oopTreeFromString<AntBoardSimT>, "oopTreeFromString")
    , std::make_tuple(&oopTreeFromExtString<AntBoardSimT>, "oopTreeFromExtString")
  );
}}

)""",
// clang-format off
      gpm::utils::argsnamed
      , "antRPN", antRPN
      , "antBoardSimName", antBoardSimName
      , "recursiveVariantNotation", recursiveVariantNotation
      , "cppFixedNotation", cppFixedNotation
      , "cppFixedWithVisitorNotation", cppFixedWithVisitorNotation
      , "oopNotation", oopNotation
      // clang-format on
  );
}
