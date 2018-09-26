#include <boost/hana.hpp>
#include <string_view>
#include <tuple>

namespace hana = boost::hana;
using namespace hana::literals;  // contains the _c suffix

struct M {};
struct L {};
struct R {};
struct IF {};
struct P2 {};

struct counter {
  int countM = 2;
  int countL = 1;
  int countR = 0;
  int countIF = 1;
  int countP2 = 1;
};

template <typename ContextT, typename TrueTreeT, typename FalseTreeT>
void Sink(ContextT& contex, hana::tuple<IF, TrueTreeT, FalseTreeT> tup) {
  if (contex.cond())
    Sink(contex, tup[1_c]);
  else
    Sink(contex, tup[2_c]);
}

template <typename ContextT, typename Child0T, typename Child1T>
void Sink(ContextT& contex, hana::tuple<P2, Child0T, Child1T> tup) {
  Sink(contex, tup[1_c]);
  Sink(contex, tup[2_c]);
}

template <typename ContextT>
void Sink(ContextT& contex, hana::tuple<M>) {
  contex.move();
}

template <typename ContextT>
void Sink(ContextT& contex, hana::tuple<L>) {
  contex.left();
}

template <typename ContextT>
void Sink(ContextT& contex, hana::tuple<R>) {
  contex.right();
}

template <typename ContextT, typename... T>
void Sink(ContextT& contex, hana::tuple<T...>) {}

template <typename T>
counter foobar(T tup, counter c) {
  boost::hana::for_each(
      tup, hana::overload([&](auto x) { c = foobar(x, c); },
                          [&](P2) { --(c.countP2); },
                          [&](IF) { --(c.countIF); }, [&](M) { --(c.countM); },
                          [&](L) { --(c.countL); }, [&](R) { --(c.countR); }));
  return c;
}

struct AntSim {
  mutable int condC = 0;
  mutable int moveC = 0;
  mutable int leftC = 0;
  mutable int rightC = 0;
  bool cond() const {
    condC++;
    return true;
  }
  void move() { moveC += 10; }
  void left() { leftC += 100; }
  void right() { rightC += 1000; }
  int c() { return condC + moveC + leftC + rightC; }
};

int main() {
  auto b =
      hana::tuple<IF,
                  hana::tuple<P2, hana::tuple<M>,
                              hana::tuple<P2, hana::tuple<M>, hana::tuple<R>>>,
                  hana::tuple<L>>{};
  counter co;
  auto b2 = hana::tuple<
      IF, hana::tuple<hana::tuple<P2, hana::tuple<M, M>>, hana::tuple<L>>>{};

  co = foobar(b2, co);
  // ff::f1(b2[0_c], b2[1_c]);
  //    boost::hana::for_each(b, hana::overload(
  //        [&](auto x) {},
  //       [&](P2, auto n1, auto n2){--countIF;},
  //        [&](IF, auto trueNodes, auto falseNodes){--countIF;},
  //        [&](M){--countM;},
  //        [&](L){--countL;},
  //        [&](R){--countR;}
  //    ));

  AntSim as;
  Sink(as, b);
  return as.c();
}
