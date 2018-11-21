#pragma once

#include <boost/variant.hpp>
#include <cstddef>
#include <tuple>

namespace gpm {

class CountNodes : public boost::static_visitor<std::size_t> {
 public:
  template <typename T>
  std::size_t operator()(T const& node) const {
    std::size_t count = 0;
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0) {
      for (auto const& n : node.children) {
        count += boost::apply_visitor(*this, n);
      }
    }
    return 1 + count;
  }
};

template <typename SinkType>
class CallSinkOnNodes : public boost::static_visitor<void> {
 public:
  SinkType const& sink_;

  CallSinkOnNodes(SinkType const& sink) : sink_{sink} {}

  template <typename T>
  void operator()(T& node) const {
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0) {
      for (auto& n : node.children) {
        if (!sink_(n)) return;
        boost::apply_visitor(*this, n);
      }
    }
  }
};

}  // namespace gpm
