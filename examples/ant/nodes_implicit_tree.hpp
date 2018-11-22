#pragma once

#include <string>
#include <string_view>
#include <array>
#include <tuple>
#include <type_traits>
#include <gpm/io.hpp>


namespace impl_tree{
using ImplTreeType = std::string;

enum class EvalMode { DoEval, TraverseOnly };



template <uint8_t kMaxHash, typename BeginIterType, typename EndIterType>
constexpr uint8_t simpleHash(BeginIterType begin, EndIterType end) {
  uint8_t r = 0;
  for (; begin != end; ++begin) {
    r = (r + 7) ^ *begin;
  }
  return r & (kMaxHash - 1);
}

template <uint8_t kMaxHash, typename RangeType>
constexpr uint8_t simpleHash(RangeType range) {
  return simpleHash<kMaxHash>(std::begin(range), std::end(range));
}

template<typename ContexType>
using BehaviorFunctionType = std::add_pointer_t<gpm::RPNTokenCursor&(gpm::RPNTokenCursor &, ContexType&, EvalMode)>;

template<typename ContexType>
struct NodeDef {
  std::size_t childCount;
  std::string_view name;
  BehaviorFunctionType<ContexType> behavior;
};

template<typename ContexType>
std::array<NodeDef<ContexType>, 16> kNodes{};

 
template<uint8_t kMaxHash, typename ContexType> 
auto ifBehavior(gpm::RPNTokenCursor & tokenCursor, ContexType & c, EvalMode em)-> gpm::RPNTokenCursor &{
  if(em == EvalMode::TraverseOnly) {
    for(int i = 0; i < 2; ++i) {
      tokenCursor.next();
      auto token = tokenCursor.token();
      auto behaviorFun = kNodes<ContexType>[simpleHash<kMaxHash>(token)].behavior;
      tokenCursor = (*behaviorFun)(tokenCursor, c, EvalMode::TraverseOnly);
    }
  }
  else {
    auto foodIsInFront = c.is_food_in_front();
    for(int i = 0; i < 2; ++i) {
      tokenCursor.next();
      auto token = tokenCursor.token();
      EvalMode childEMode = EvalMode::TraverseOnly;
      if((foodIsInFront && i == 0) || (!foodIsInFront && i == 1)){
        childEMode = EvalMode::DoEval;
      }
      auto behaviorFun = kNodes<ContexType>[simpleHash<kMaxHash>(token)].behavior;
      tokenCursor = (*behaviorFun)(tokenCursor, c, childEMode);
    }
  }
  return tokenCursor;
}

template<uint8_t kMaxHash, typename ContexType> 
auto moveBehavior(gpm::RPNTokenCursor & tokenCursor, ContexType & c, EvalMode em)-> gpm::RPNTokenCursor &{
  if(em == EvalMode::DoEval) {
    c.move();
  }
  return tokenCursor;
}

template<uint8_t kMaxHash, typename ContexType> 
auto rightBehavior(gpm::RPNTokenCursor & tokenCursor, ContexType & c, EvalMode em)-> gpm::RPNTokenCursor &{
  if(em == EvalMode::DoEval) {
    c.right();
  }
  return tokenCursor;
}

template<uint8_t kMaxHash, typename ContexType> 
auto leftBehavior(gpm::RPNTokenCursor & tokenCursor, ContexType & c, EvalMode em)-> gpm::RPNTokenCursor &{
  if(em == EvalMode::DoEval) {
    c.left();
  }
  return tokenCursor;
}

template<uint8_t kMaxHash, typename ContexType> 
auto p2Behavior (gpm::RPNTokenCursor & tokenCursor, ContexType & c, EvalMode em)-> gpm::RPNTokenCursor &{
  for(int i = 0; i < 2; ++i) {
    tokenCursor.next();
    auto token = tokenCursor.token();
    auto behaviorFun = kNodes<ContexType>[simpleHash<kMaxHash>(token)].behavior;
    tokenCursor = (*behaviorFun)(tokenCursor, c, em);
  }
  return tokenCursor;
}

template<uint8_t kMaxHash, typename ContexType> 
auto p3Behavior(gpm::RPNTokenCursor & tokenCursor, ContexType & c, EvalMode em)-> gpm::RPNTokenCursor &{
  for(int i = 0; i < 3; ++i) {
    tokenCursor.next();
    auto token = tokenCursor.token();
    auto behaviorFun = kNodes<ContexType>[simpleHash<kMaxHash>(token)].behavior;
    tokenCursor = (*behaviorFun)(tokenCursor, c, em);
  }
  return tokenCursor;
}

template<uint8_t kMaxHash, typename ContexType>
bool initKNodes()
{
  using NodeT = NodeDef<ContexType>;
  using namespace std::literals;
  {
    auto name = "if"sv;
    kNodes<ContexType>[simpleHash<kMaxHash>(name)] = NodeT{2, name, &ifBehavior<kMaxHash, ContexType>};
  }
  {
    auto name = "m"sv;
    kNodes<ContexType>[simpleHash<kMaxHash>(name)] = NodeT{0, name, &moveBehavior<kMaxHash, ContexType>};
  }
  {
    auto name = "r"sv;
    kNodes<ContexType>[simpleHash<kMaxHash>(name)] = NodeT{0, name, &rightBehavior<kMaxHash, ContexType>};
  }
  {
    auto name = "l"sv;
    kNodes<ContexType>[simpleHash<kMaxHash>(name)] = NodeT{0, name, &leftBehavior<kMaxHash, ContexType>};
  }
  {
    auto name = "p2"sv;
    kNodes<ContexType>[simpleHash<kMaxHash>(name)] = NodeT{2, name, &p2Behavior<kMaxHash, ContexType>};
  }
  {
    auto name = "p3"sv;
    kNodes<ContexType>[simpleHash<kMaxHash>(name)] = NodeT{3, name, &p3Behavior<kMaxHash, ContexType>};
  }
  return true;
}

template<template <typename>typename HashFunction, typename ContexType>
bool initKNodes2()
{
  constexpr uint8_t kMaxHash = 16;
  using NodeT = NodeDef<ContexType>;
  using namespace std::literals;
  {
    auto name = "if"sv;
    kNodes<ContexType>[HashFunction<std::string_view>::simpleHash(name)] = NodeT{2, name, &ifBehavior<kMaxHash, ContexType>};
  }
  {
    auto name = "m"sv;
    kNodes<ContexType>[HashFunction<std::string_view>::simpleHash(name)] = NodeT{0, name, &moveBehavior<kMaxHash, ContexType>};
  }
  {
    auto name = "r"sv;
    kNodes<ContexType>[HashFunction<std::string_view>::simpleHash(name)] = NodeT{0, name, &rightBehavior<kMaxHash, ContexType>};
  }
  {
    auto name = "l"sv;
    kNodes<ContexType>[HashFunction<std::string_view>::simpleHash(name)] = NodeT{0, name, &leftBehavior<kMaxHash, ContexType>};
  }
  {
    auto name = "p2"sv;
    kNodes<ContexType>[HashFunction<std::string_view>::simpleHash(name)] = NodeT{2, name, &p2Behavior<kMaxHash, ContexType>};
  }
  {
    auto name = "p3"sv;
    kNodes<ContexType>[HashFunction<std::string_view>::simpleHash(name)] = NodeT{3, name, &p3Behavior<kMaxHash, ContexType>};
  }
  return true;
}


template<typename ContexType>
void eval(gpm::RPNTokenCursor tokenCursor, ContexType & c){
  constexpr uint8_t kMaxHash = 16;
  [[gnu::unused]]static bool b = initKNodes<kMaxHash, ContexType>();
  
  auto token = tokenCursor.token();
  auto behaviorFun = kNodes<ContexType>[simpleHash<kMaxHash>(token)].behavior;
  (*behaviorFun)(tokenCursor, c, EvalMode::DoEval);
  
}


}
