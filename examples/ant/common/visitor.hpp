/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include "ant_board_simulation.hpp"
#include "nodes.hpp"

namespace ant {

template <typename AntBoardSimType>
class AntBoardSimulationVisitor : public boost::static_visitor<void> {
 public:
  AntBoardSimulationVisitor(AntBoardSimType& sim) : sim_{sim} {}

  void operator()(Move) const { sim_.move(); }

  void operator()(Left) const { sim_.left(); }

  void operator()(Right) const { sim_.right(); }

  void operator()(IfFoodAhead const& c) const {
    boost::apply_visitor(*this, c.get(sim_.is_food_in_front()));
  }

  template <typename T>
  void operator()(T const& node) const {
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
      for (auto const& n : node.children) boost::apply_visitor(*this, n);
  }

 private:
  AntBoardSimType& sim_;
};

}  // namespace ant
