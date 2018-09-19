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

  void operator()(move) const { sim_.move(); }

  void operator()(left) const { sim_.left(); }

  void operator()(right) const { sim_.right(); }

  void operator()(if_food_ahead const& c) const {
    boost::apply_visitor(*this, c.get(sim_.is_food_in_front()));
  }

  template<typename T>
  void operator()(T const& node) const {
    if constexpr(std::tuple_size<decltype(node.nodes)>::value != 0)
      for (auto const& n : node.nodes) boost::apply_visitor(*this, n);
  }

 private:
  AntBoardSimType& sim_;
};

}  // namespace ant
