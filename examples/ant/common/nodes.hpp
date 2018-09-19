/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once
#include <gpm/gpm.hpp>
#include <boost/any.hpp>


namespace ant {
  


struct move : public gpm::BaseNode<boost::any, 0, gpm::NodeToken<'m'>> {};

struct right : public gpm::BaseNode<boost::any, 0, gpm::NodeToken<'r'>> {};

struct left : public gpm::BaseNode<boost::any, 0, gpm::NodeToken<'l'>> {};

struct if_food_ahead;
template <int nodeCount, typename CTString>
struct prog;

using prog2 = prog<2, gpm::NodeToken<'p', '2'>>;
using prog3 = prog<3, gpm::NodeToken<'p', '3'>>;

using ant_nodes =
boost::variant<move, left, right, boost::recursive_wrapper<if_food_ahead>,
                   boost::recursive_wrapper<prog2>,
                   boost::recursive_wrapper<prog3>>;

template <int NodeCount, typename CTString>
struct prog : public gpm::BaseNode<ant_nodes, NodeCount, CTString> {
  using prog::BaseNode::BaseNode;
};

// struct move : public gpm::BaseNode<ant_nodes, 0, gpm::NodeToken<'m'>> {};
// 
// struct right : public gpm::BaseNode<ant_nodes, 0, gpm::NodeToken<'r'>> {};
// 
// struct left : public gpm::BaseNode<ant_nodes, 0, gpm::NodeToken<'l'>> {};

struct if_food_ahead
    : public gpm::BaseNode<ant_nodes, 2, gpm::NodeToken<'i', 'f'>> {
  using if_food_ahead::BaseNode::BaseNode;

  template <bool c>
  constexpr ant_nodes const& get() const {
    return nodes[c ? 0 : 1];
  }
  constexpr ant_nodes const& get(bool b) const {
    return b ? get<true>() : get<false>();
  }
};

}  // namespace ant
