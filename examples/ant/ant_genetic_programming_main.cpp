#include <vector>
#include <functional>
#include <optional>

#include <boost/variant.hpp>

#include <fmt/printf.h>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"



class CountNodes : public boost::static_visitor<int>
{
public:
  
    template <typename T>
    int operator()(T const& node) const {
      int count = 0;
      if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
      {
        for (auto const& n : node.children) {
          count += boost::apply_visitor(*this, n);
        }
      }
      return 1 + count;
    }
};



class FlattenTree : public boost::static_visitor<std::vector<std::reference_wrapper<ant::ant_nodes>>>
{
public:
  mutable ant::ant_nodes * rootNode_ = nullptr;
  FlattenTree(ant::ant_nodes & rootNode):rootNode_{&rootNode}{}
  
  template <typename T>
  std::vector<std::reference_wrapper<ant::ant_nodes>> operator()(T & node) const {
    std::vector<std::reference_wrapper<ant::ant_nodes>> toReturn_;
    if(rootNode_ != nullptr)
    {
      toReturn_.push_back(*rootNode_);
      rootNode_ = nullptr;
    }
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
    {
      for (auto & n : node.children) {
        toReturn_.push_back(n);
        auto toAppend = boost::apply_visitor(*this, n);
        toReturn_.insert(std::end(toReturn_), std::begin(toAppend), std::end(toAppend));
      }
    }
    return toReturn_;
  }
};

template<typename SinkF>
class FlattenTree2 : public boost::static_visitor<SinkF>
{
public:
  SinkF sink_;
  FlattenTree2(SinkF sink):sink_{sink}{}
  
  template <typename T>
  SinkF operator()(T & node) const {
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
    {
      for (auto & n : node.children) {
        sink_(n);
        boost::apply_visitor(*this, n);
      }
    }
    return sink_;
  }
};



int main()
{
  char const* optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
  auto optAnt =
  gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{optimalAntRPNdef});
  fmt::print("{}\n", boost::apply_visitor(CountNodes{}, optAnt)); 
  
  auto flaterVisitor = FlattenTree{optAnt};
  auto g = boost::apply_visitor(flaterVisitor, optAnt);
  fmt::print("{}\n", g.size()); 

//   g[15].get() = g[0].get();
  

  std::vector<std::reference_wrapper<ant::ant_nodes>> nn{optAnt};
  boost::apply_visitor(FlattenTree2{[&nn](ant::ant_nodes & n){ nn.push_back(n); }}, optAnt);
  
  for(auto const & subTree: nn) 
  {
    auto s = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, subTree.get());
    fmt::print("{}\n", s); 
  }
  
  
//   auto g2 = boost::apply_visitor(FlattenTree{optAnt}, optAnt);
//   fmt::print("{}\n", g2.size()); 
//   for(auto const & subTree: g2) 
//   {
//     auto s = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, subTree.get());
//     fmt::print("{}\n", s); 
//   }

}
