/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once
#include <boost/any.hpp>
#include <gpm/gpm.hpp>

namespace ant {

struct move : public gpm::BaseNode<boost::any, 0, gpm::NodeToken<'m'>> {};

struct right : public gpm::BaseNode<boost::any, 0, gpm::NodeToken<'r'>> {};

struct left : public gpm::BaseNode<boost::any, 0, gpm::NodeToken<'l'>> {};

struct if_food_ahead;

struct prog2;

struct prog3;

using ant_nodes =
    boost::variant<move, left, right, boost::recursive_wrapper<if_food_ahead>,
                   boost::recursive_wrapper<prog2>,
                   boost::recursive_wrapper<prog3>>;


struct prog2 : public gpm::BaseNode<ant_nodes, 2, gpm::NodeToken<'p', '2'>> {
  using BaseNode::BaseNode;
};

struct prog3 : public gpm::BaseNode<ant_nodes, 3, gpm::NodeToken<'p', '3'>> {
  using BaseNode::BaseNode;
};

struct if_food_ahead
    : public gpm::BaseNode<ant_nodes, 2, gpm::NodeToken<'i', 'f'>> {
  using if_food_ahead::BaseNode::BaseNode;

  constexpr ant_nodes const& get(bool b) const {
    return b ? children[0] : children[1];
  }
};



}  // namespace ant


namespace gpm{
  template<>
  constexpr size_t ChildrenSize<ant::if_food_ahead> = 2;
  template<>
  constexpr size_t ChildrenSize<ant::prog3> = 3;
  template<>
  constexpr size_t ChildrenSize<ant::prog2> = 2;
  
}
