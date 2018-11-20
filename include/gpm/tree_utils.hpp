#pragma once

#include <tuple>
#include <boost/variant.hpp>


namespace gpm{

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
  

}// namespace gpm
