#pragma once

#include <array>
#include <gpm/io.hpp>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace implizit {

enum class EvalMode { DoEval, TraverseOnly };

template <typename CursorType, typename ContexType>
using BehaviorFunctionType =
    std::add_pointer_t<CursorType &(CursorType &, ContexType &, EvalMode)>;

template <typename CursorType, typename ContexType>
struct NodeDef {
  std::size_t childCount = 0;
  std::string_view name{""};
  BehaviorFunctionType<CursorType, ContexType> behavior{nullptr};
};

template <typename HashFunction, typename CursorType, typename ContexType>
inline std::array<NodeDef<CursorType, ContexType>, HashFunction::kMaxHashValue>
    kNodesLUT{};

template <typename HashFunction, typename CursorType, typename ContexType>
auto defaultBehavior(CursorType &tokenCursor, ContexType &c, EvalMode em,
                     std::size_t childrenNodeCount) -> CursorType & {
  for (std::size_t i = 0; i < childrenNodeCount; ++i) {
    tokenCursor.next();
    auto token = tokenCursor.token();
    auto behaviorFun = kNodesLUT<HashFunction, CursorType,
                                 ContexType>[HashFunction::get(token)]
                           .behavior;
    tokenCursor = (*behaviorFun)(tokenCursor, c, em);
  }
  return tokenCursor;
}

template <typename HashFunction, typename CursorType, typename ContexType>
void initNodesBehavior() {
  using NodeT = NodeDef<CursorType, ContexType>;
  using namespace std::literals;
  std::array nodeDef = {
      NodeT{2, "if"sv,
            [](CursorType &tokenCursor, ContexType &c,
               EvalMode em) -> CursorType & {
              if (em == EvalMode::TraverseOnly) {
                tokenCursor =
                    defaultBehavior<HashFunction>(tokenCursor, c, em, 2);
              } else {
                auto foodIsInFront = c.is_food_in_front();
                for (int i = 0; i < 2; ++i) {
                  tokenCursor.next();
                  auto token = tokenCursor.token();
                  EvalMode childEMode = EvalMode::TraverseOnly;
                  if ((foodIsInFront && i == 0) || (!foodIsInFront && i == 1)) {
                    childEMode = EvalMode::DoEval;
                  }
                  auto behaviorFun =
                      kNodesLUT<HashFunction, CursorType,
                                ContexType>[HashFunction::get(token)]
                          .behavior;
                  tokenCursor = (*behaviorFun)(tokenCursor, c, childEMode);
                }
              }
              return tokenCursor;
            }},
      NodeT{0, "m"sv,
            [](CursorType &tokenCursor, ContexType &c,
               EvalMode em) -> CursorType & {
              if (em == EvalMode::DoEval) {
                c.move();
              }
              return tokenCursor;
            }},
      NodeT{0, "r"sv,
            [](CursorType &tokenCursor, ContexType &c,
               EvalMode em) -> CursorType & {
              if (em == EvalMode::DoEval) {
                c.right();
              }
              return tokenCursor;
            }},
      NodeT{0, "l"sv,
            [](CursorType &tokenCursor, ContexType &c,
               EvalMode em) -> CursorType & {
              if (em == EvalMode::DoEval) {
                c.left();
              }
              return tokenCursor;
            }},
      NodeT{2, "p2"sv,
            [](CursorType &tokenCursor, ContexType &c,
               EvalMode em) -> CursorType & {
              return defaultBehavior<HashFunction>(tokenCursor, c, em, 2);
            }},
      NodeT{3, "p3"sv,
            [](CursorType &tokenCursor, ContexType &c,
               EvalMode em) -> CursorType & {
              return defaultBehavior<HashFunction>(tokenCursor, c, em, 3);
            }}};
  // TODO: test for multiple inits
  for (auto &nDef : nodeDef) {
    auto hash = HashFunction::get(nDef.name);
    // TODO: test for colision
    kNodesLUT<HashFunction, CursorType, ContexType>[hash] = nDef;
  }
}

template <typename HashFunction, typename CursorType, typename ContexType>
void eval(CursorType tokenCursor, ContexType &c) {
  auto token = tokenCursor.token();
  auto behaviorFun =
      kNodesLUT<HashFunction, CursorType, ContexType>[HashFunction::get(token)]
          .behavior;
  (*behaviorFun)(tokenCursor, c, EvalMode::DoEval);
}

}  // namespace implizit
