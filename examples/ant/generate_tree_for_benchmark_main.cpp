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
#include <type_traits>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <cxxopts.hpp>

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

  std::string operator()(ant::if_food_ahead const& node) const {
    return gpm::utils::format(
        R"""(
            std::make_unique<antoop::IfFoodAhead<decltype({simulationName})>>(
                {true_branch}
                , {false_branch}
            )
            )""",
        gpm::utils::argsnamed, 
        // clang-format off
        "simulationName", simulationName_, 
        "true_branch", boost::apply_visitor(*this, node.get(true)),
        "false_branch", boost::apply_visitor(*this, node.get(false)));
        // clang-format om
  }

  struct AntNodeToClassName {
    static char const* name(ant::move) { return "Move"; }
    static char const* name(ant::left) { return "Left"; }
    static char const* name(ant::right) { return "Right"; }
    static char const* name(ant::prog2) { return "Prog2"; }
    static char const* name(ant::prog3) { return "Prog3"; }
  };

  template <typename NodeT>
  std::string operator()(NodeT node) const {
    std::string res;
    if constexpr (node.children.size() != 0) {
      auto delimi = "\n";
      for (auto& n : node.children) {
        (res += delimi) += boost::apply_visitor(*this, n);
        delimi = ",";
      }
      res += "\n";
    }

    res = gpm::utils::format(
        "std::make_unique<antoop::{nodeName}<decltype({simulationName})>>({"
        "nodeChildren})\n",
        gpm::utils::argsnamed, "nodeName", AntNodeToClassName::name(node),
        "nodeChildren", res, "simulationName", simulationName_);
    return res;
  }
};

class AsCPPFixedNotation : public boost::static_visitor<std::string> {
  std::string simulationName_;

 public:
  AsCPPFixedNotation(std::string const& simulationName)
      : simulationName_{simulationName} {}

//   std::string operator()(ant::if_food_ahead const& node) const {
//     return gpm::utils::format(
//         R"""(
//                 if({simulationName}.is_food_in_front()){{
//                     {true_branch}
//                 }}else{{
//                     {false_branch}
//                 }}
//             )""",
//         gpm::utils::argsnamed, 
//         // clang-format off
//         "simulationName", simulationName_
//         , "true_branch", boost::apply_visitor(*this, node.get(true))
//         , "false_branch", boost::apply_visitor(*this, node.get(false)));
//         // clang-format on
//   }

  struct AntNodeToSimulationMethodName {
    static char const* name(ant::move) { return "move"; }
    static char const* name(ant::left) { return "left"; }
    static char const* name(ant::right) { return "right"; }
    template<typename NodeT>
    static char const* name(NodeT) { return ""; }
  };

  template <typename NodeT>
  std::string operator()(NodeT const & node) const {
    std::string res;
    if constexpr (std::is_same_v<ant::if_food_ahead, NodeT>) {
      return gpm::utils::format(
        R"""(
          if({simulationName}.is_food_in_front()){{
          {true_branch}
          }}else{{
          {false_branch}
          }}
          )""",
      gpm::utils::argsnamed
      , "simulationName", simulationName_
      , "true_branch", boost::apply_visitor(*this, node.get(true))
      , "false_branch", boost::apply_visitor(*this, node.get(false))
      );
      // clang-format off
    }
    else if constexpr (gpm::ChildrenSize<NodeT> == 0) {
      res += gpm::utils::format("{simulationName}.{methodName}();\n",
                                gpm::utils::argsnamed, 
                                // clang-format off
                                "simulationName", simulationName_, 
                                "methodName", AntNodeToSimulationMethodName::name(node));
                                // clang-format on
    }
    else if constexpr (gpm::ChildrenSize<NodeT> != 0){
      for (auto& n : node.children) res += boost::apply_visitor(*this, n);

    }
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

  std::string operator()(ant::if_food_ahead const& node) const {
    return gpm::utils::format(
        R"""(
                if({simulationName}.is_food_in_front()){{
                    {true_branch}
                }}else{{
                    {false_branch}
                }}
            )""",
        gpm::utils::argsnamed, 
        // clang-format off
        "simulationName", simulationName_, 
        "true_branch", boost::apply_visitor(*this, node.get(true)), 
        "false_branch", boost::apply_visitor(*this, node.get(false)));
        // clang-format on
  }

  template <typename NodeT>
  std::string operator()(NodeT const & node) const {
    auto nodeType = boost::typeindex::type_id_runtime(node).pretty_name();
    std::string res;
    if constexpr (gpm::ChildrenSize<NodeT> == 0) {
      res += gpm::utils::format("{visitorName}({nodeType}{{}});\n",
                                gpm::utils::argsnamed, "visitorName",
                                visitorName_, "nodeType", nodeType);
    }
    if constexpr (gpm::ChildrenSize<NodeT> != 0)
      for (auto& n : node.children) res += boost::apply_visitor(*this, n);
    return res;
  }
};

class AsRecursiveVariantNotation : public boost::static_visitor<std::string> {
 public:
  std::string operator()(ant::if_food_ahead const& node) const {
    return gpm::utils::format(
        R"""(
                {nodeName}{{{true_branch}, {false_branch}}}
            )""",
        gpm::utils::argsnamed, 
        // clang-format off
        "nodeName", boost::typeindex::type_id_runtime(node).pretty_name(), 
        "true_branch", boost::apply_visitor(*this, node.get(true)), 
        "false_branch", boost::apply_visitor(*this, node.get(false)));
        // clang-format on
  }

  template <typename NodeT>
  std::string operator()(NodeT const & node) const {
    std::string res;
    if constexpr (gpm::ChildrenSize<NodeT> != 0) {
      auto delimi = "\n";
      for (auto& n : node.children) {
        (res += delimi) += boost::apply_visitor(*this, n);
        delimi = ",";
      }
      res += "\n";
    }

    res = gpm::utils::format("{nodeName}{{{nodeChildren}}}\n",
                             gpm::utils::argsnamed, "nodeName",
                             boost::typeindex::type_id_runtime(node).pretty_name(),
                             "nodeChildren", res);
    return res;
  }
};

decltype(auto) getRandomAnt() {
  int minHeight = 1;
  int maxHeight = 7;
  // std::random_device rd;

  return gpm::BasicGenerator<ant::ant_nodes>{minHeight, maxHeight}();
}

decltype(auto) getOptAnt() {
  char const* optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
  return gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{optimalAntRPNdef});
}

decltype(auto) getOptAntFromFile(char const* filename) {
  std::ifstream f(filename);
  std::string str;
  std::getline(f, str);
  return gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{str});
}

int main(int argc, char** argv) {
  cxxopts::Options options("generate_tree_for_benchmark", "");
  options.allow_unrecognised_options().add_options()(
      "a,antrpndef", "File name", cxxopts::value<std::string>())(
      "o,outfile", "File name", cxxopts::value<std::string>());

  auto cliArgs = options.parse(argc, argv);

  if (!cliArgs.count("antrpndef") || !cliArgs.count("outfile")) {
    std::cout << options.help({"", ""}) << std::endl;
    exit(0);
  }

  auto filename = cliArgs["antrpndef"].as<std::string>();
  auto ant = getOptAntFromFile(filename.c_str());

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

  auto outFileName = cliArgs["outfile"].as<std::string>();
  std::ofstream outf(outFileName.c_str());
  outf << gpm::utils::format(
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
decltype(auto) getAllTreeBenchmarks()
{{
  return std::make_tuple(
      std::make_tuple(&recursiveVariantTreeFromString<AntBoardSimT>, "recursiveVariantTreeFromString")
    , std::make_tuple(&recursiveVariantTree<AntBoardSimT>, "recursiveVariantTree")
    , std::make_tuple(&cppFixedTree<AntBoardSimT>, "cppFixedTree")
    , std::make_tuple(&cppFixedWithVisitor<AntBoardSimT>, "cppFixedWithVisitor")
    , std::make_tuple(&oopTree<AntBoardSimT>, "oopTree")
    , std::make_tuple(&oopTreeFromString<AntBoardSimT>, "oopTreeFromString")
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
