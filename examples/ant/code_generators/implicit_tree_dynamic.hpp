/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <fmt/format.h>
#include <string>

struct ImplicitTreeDynamic {
  std::string name() const { return "implicitTreeDynamic"; }
  std::string functionName() const { return "implicitTreeDynamic"; }
  std::string body(ant::NodesVariant) const {
    return fmt::format(R"""(
template <typename HashType, HashType kMaxHashValue_>
struct NodeNameHash {{
  static constexpr HashType kMaxHashValue = kMaxHashValue_;

  template <typename BeginIterType, typename EndIterType>
  static constexpr HashType get(BeginIterType begin, EndIterType end) {{
    HashType r = 0;
    for (; begin != end; ++begin) {{
      r = (r + 7) ^ *begin;
    }}
    return r & (kMaxHashValue - 1);
  }}

  template <typename RangeType>
  static constexpr HashType get(RangeType range) {{
    return get(std::begin(range), std::end(range));
  }}
}};  

template<typename AntBoardSimT, typename CursorType>
static int implicitTreeDynamic(AntBoardSimT antBoardSim, CursorType cursor, BenchmarkPart toMessure)
{{  

  using HashFunction = NodeNameHash<uint8_t, 16>;

  if(toMessure == BenchmarkPart::Create) {{
    benchmark::DoNotOptimize(cursor);
    return 0;
  }}

  while (!antBoardSim.is_finish()) {{
    implicit_tree::eval<HashFunction>(cursor, antBoardSim);
  }}

  benchmark::DoNotOptimize(antBoardSim.score());
  return antBoardSim.score(); 
}}
)""");
  }
};
