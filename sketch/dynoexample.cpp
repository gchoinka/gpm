
#include <iostream>

#include "../examples/ant/common/ant_board_simulation.hpp"
#include "../examples/ant/common/santa_fe_board.hpp"
#include "../examples/ant/nodes_dyno.hpp"
using namespace dyno::literals;

decltype(auto) getAntSataFeStaticBoardSim() {
  using namespace ant;
  auto max_steps = 400;
  auto max_food = 89;
  auto antSim =
      sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
          max_steps, max_food, sim::Pos2d{0, 0}, sim::Direction::east,
          [](sim::AntBoardSimulationStaticSize<
              santa_fe::x_size, santa_fe::y_size>::FieldType& board) {
            for (size_t x = 0; x < board.size(); ++x) {
              for (size_t y = 0; y < board[x].size(); ++y) {
                board[x][y] = santa_fe::board[x][y] == 'X'
                                  ? sim::BoardState::food
                                  : sim::BoardState::empty;
              }
            }
          }};

  return antSim;
}

int main() {
  auto antBoardSim = getAntSataFeStaticBoardSim();
  using ContexT = decltype(antBoardSim);
  antdyno::IfFood<ContexT> test{antdyno::Move<ContexT>{},
                                antdyno::Move<ContexT>{}};
  test.eval(antBoardSim);
  std::cout << antBoardSim.score() << "\n";
}
