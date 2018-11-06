#include <vector>
#include <functional>
#include <optional>

#include <boost/variant.hpp>
#include <boost/function_output_iterator.hpp>
#include <boost/range/irange.hpp>

#include <fmt/printf.h>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"

#include <vector>



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



// class FlattenTree : public boost::static_visitor<std::vector<std::reference_wrapper<ant::ant_nodes>>>
// {
// public:
//   mutable ant::ant_nodes * rootNode_ = nullptr;
//   FlattenTree(ant::ant_nodes & rootNode):rootNode_{&rootNode}{}
//   
//   template <typename T>
//   std::vector<std::reference_wrapper<ant::ant_nodes>> operator()(T & node) const {
//     std::vector<std::reference_wrapper<ant::ant_nodes>> toReturn_;
//     if(rootNode_ != nullptr)
//     {
//       toReturn_.push_back(*rootNode_);
//       rootNode_ = nullptr;
//     }
//     if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
//     {
//       for (auto & n : node.children) {
//         toReturn_.push_back(n);
//         auto toAppend = boost::apply_visitor(*this, n);
//         toReturn_.insert(std::end(toReturn_), std::begin(toAppend), std::end(toAppend));
//       }
//     }
//     return toReturn_;
//   }
// };

template<typename OutputIterT>
class FlattenTree : public boost::static_visitor<OutputIterT>
{
public:
  OutputIterT sink_;
  FlattenTree(OutputIterT sink):sink_{sink}{}
  
  template <typename T>
  OutputIterT operator()(T & node) {
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
    {
      for (auto & n : node.children) {
        *sink_++ = std::ref(n);
        boost::apply_visitor(*this, n);
      }
    }
    return sink_;
  }
};

decltype(auto) getAntSataFeStaticBoardSim() {
  using namespace ant;
  auto max_steps = 400;
  auto max_food = 89;
  auto antSim =
  sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
    max_steps, max_food, sim::Pos2d{0, 0}, sim::Direction::east,
    [](sim::AntBoardSimulationStaticSize<
    santa_fe::x_size, santa_fe::y_size>::FieldType& board) {
      for (size_t x = 0; x < board.size(); ++x) {
        for (size_t y = 0; y < board[x].size(); ++y) {
          board[x][y] = santa_fe::board[x][y] == 'X'
          ? sim::BoardState::food
          : sim::BoardState::empty;
        }
      }
    }};
    
    return antSim;
}


int main()
{
  char const* optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
  auto optAnt =
  gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{optimalAntRPNdef});
//   fmt::print("{}\n", boost::apply_visitor(CountNodes{}, optAnt)); 
// 
//   
// 
//   std::vector<std::reference_wrapper<ant::ant_nodes>> nodesRef{optAnt};
//   FlattenTree f{boost::make_function_output_iterator([&nodesRef](ant::ant_nodes & n){ nodesRef.push_back(n); })};
//   boost::apply_visitor(f, optAnt);
//   
//   
//   for(auto const & subTree: nodesRef) 
//   {
//     auto s = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, subTree.get());
//     fmt::print("{}\n", s); 
//   }
//   
  
  constexpr auto populationSize = 5000;
  constexpr auto minHeight = 2;
  constexpr auto maxHeight = 11;

  constexpr auto generationMax = 5000;
  constexpr auto numberOfElite = std::min(4, populationSize);
//   double const mutation_rate = 0.8;
//   double const crossover_rate = 0.8;
//   double const crossover_internal_point_rate = 0.9;
//   double const reproduction_rate = 0.1;
//   size_t const min_tree_height = 1;
//   size_t const init_max_tree_height  = 6;
//   size_t const max_tree_height = 17;
//   size_t const tournament_size = 4;
  
  
  std::vector<ant::ant_nodes> population;
  population.reserve(populationSize);
  struct ScoreIdxPair { double score; std::size_t index; };
  std::vector<ScoreIdxPair> populationScore;
  populationScore.reserve(populationSize);
  
  std::random_device rd;
  auto rndNodeGen = gpm::BasicGenerator<ant::ant_nodes>{minHeight, maxHeight, rd()};
  
  for(auto generation = 0; generation < generationMax; ++generation)
  {
    for(auto i = population.size(); i < populationSize; ++i)
      population.push_back(rndNodeGen());
    
    auto idx = 0u;
    for(auto const & anAnt : population){
      auto sim = getAntSataFeStaticBoardSim();
      auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{sim};
      
      while(!sim.is_finish())
      {
        boost::apply_visitor(antBoardSimVisitor, anAnt);
      }
      populationScore.push_back(ScoreIdxPair{double(sim.score()), idx++}); 
    }
    
    std::sort(std::begin(populationScore), std::end(populationScore), [](auto const & lhs, auto const & rhs) {
      return lhs.score < rhs.score;
    });
    
    auto nextPopulation = std::vector<ant::ant_nodes>{};
    nextPopulation.reserve(population.size());
    auto nextPopulationScore = std::vector<ScoreIdxPair>{};
    nextPopulationScore.reserve(population.size());
    for(std::size_t i = 0; i < numberOfElite; ++i){
      nextPopulationScore.emplace_back(ScoreIdxPair{populationScore[i].score, i});
      nextPopulation.emplace_back(population[populationScore[i].index]);
    }

  
    
    auto s = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, population[populationScore.front().index]);
    fmt::print("score:{} ant:{}\n", populationScore.front().score, s); 
  
    population = std::move(nextPopulation);
    populationScore = std::move(nextPopulationScore);
    
  }
  

//   
//   s = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, population[std::get<1>(populationScore.back())]);
//   fmt::print("score:{} ant:{}\n", std::get<0>(populationScore.back()), s); 
  
//   auto g2 = boost::apply_visitor(FlattenTree{optAnt}, optAnt);
//   fmt::print("{}\n", g2.size()); 
//   for(auto const & subTree: g2) 
//   {
//     
//     fmt::print("{}\n", s); 
//   }

}
