/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once
#include <gpm/gpm.hpp>

namespace ant {

struct Move
    : public gpm::BaseNode<gpm::AnyTypeNullSink, 0, gpm::NodeToken<'m'>> {};

struct Right
    : public gpm::BaseNode<gpm::AnyTypeNullSink, 0, gpm::NodeToken<'r'>> {};

struct Left
    : public gpm::BaseNode<gpm::AnyTypeNullSink, 0, gpm::NodeToken<'l'>> {};

struct IfFoodAhead;

struct Prog2;

struct Prog3;

using NodesVariant =
boost::variant<Move, Left, Right, boost::recursive_wrapper<IfFoodAhead>,
boost::recursive_wrapper<Prog2>,
boost::recursive_wrapper<Prog3>>;

struct Prog2 : public gpm::BaseNode<NodesVariant, 2, gpm::NodeToken<'p', '2'>> {
  using BaseNode::BaseNode;
};

struct Prog3 : public gpm::BaseNode<NodesVariant, 3, gpm::NodeToken<'p', '3'>> {
  using BaseNode::BaseNode;
};

struct IfFoodAhead
: public gpm::BaseNode<NodesVariant, 2, gpm::NodeToken<'i', 'f'>> {
  using IfFoodAhead::BaseNode::BaseNode;

  constexpr NodesVariant const& get(bool b) const {
    return b ? children[0] : children[1];
  }
};

}  // namespace ant
