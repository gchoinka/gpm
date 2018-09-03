/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include "ant_simulation_common.hpp" 

#include <array>
#include <string>
#include <iterator>

namespace ant::sim {


enum class BoardState { empty, food, hadFood };
constexpr static std::array<char, 3> boardStateToChar{{' ', 'O', '*'}};

template<typename FieldT>
class AntBoardSimulation
{
public:
    using FieldType = FieldT;
    
    template<typename FieldInitFunction>
    AntBoardSimulation(int steps, int max_food, ant::sim::Pos2d antPos, ant::sim::Direction direction, FieldInitFunction fieldInitFunction)
        :steps_{steps}, max_food_{max_food}, antPos_{antPos}, direction_{direction}
    {
        fieldInitFunction(field_);
    } 
    
    void move()
    {
        --steps_;
        auto toadd = ant::sim::toPos[static_cast<size_t>(direction_)];
        antPos_ = (antPos_ + toadd);
        antPos_.x() = (antPos_.x() + xSize()) % xSize();
        antPos_.y() = (antPos_.y() + ySize()) % ySize();
        
        if(field_[antPos_.x()][antPos_.y()] == BoardState::food)
        {  
            ++foodConsumed_;
            field_[antPos_.x()][antPos_.y()] = BoardState::hadFood;
        }
    }
    
    void left()
    {
        --steps_;
        direction_ = rotateCCW(direction_);
    }
    
    void right()
    {
        --steps_;
        direction_ = rotateCW(direction_);
    }
    
    bool is_food_in_front() const
    {
        auto toadd = ant::sim::toPos[static_cast<size_t>(direction_)];
        auto newPos = antPos_ + toadd;
        newPos.x() = (newPos.x() + xSize()) % xSize();
        newPos.y() = (newPos.y() + ySize()) % ySize();
        
        return field_[newPos.x()][newPos.y()] == BoardState::food;
    }
    
    bool is_finish() const
    {
        return steps_ <= 0 || score() == 0;
    }
    
    int score() const
    {
        return max_food_ - foodConsumed_;
    }
    
    std::string get_status_line() const 
    {
        std::string res;
        res.reserve(ySize());
        (((res += "steps:") += std::to_string(steps_) += " score:") += std::to_string(score()) += " fif:") += std::to_string(is_food_in_front());
        res.insert(res.size(), ySize() - res.size(), ' ');
        return res;
    }
    
    template<typename LineSinkF>
    void get_board_as_str(LineSinkF lineSink) const
    {
        lineSink(get_status_line());
        for(size_t x = 0; x < xSize(); ++x)
        {
            std::string res; res.reserve(ySize());
            for(size_t y = 0; y < ySize(); ++y)
            {
                if(antPos_ == ant::sim::Pos2d{int(x),int(y)})
                    res += ant::sim::directionToChar[static_cast<size_t>(direction_)];
                else 
                    res += boardStateToChar[static_cast<size_t>(field_[x][y])];
            }
            lineSink(res);
        }
    }
    
    auto xSize() const
    {
        return std::size(field_[0]);
    }
    
    auto ySize() const
    {
        return std::size(field_);
    }
    friend bool operator==(AntBoardSimulation const & lhs, AntBoardSimulation const & rhs)
    {
        return lhs.field_ == rhs.field_ 
                && lhs.steps_ == rhs.steps_ 
                && lhs.max_food_ == rhs.max_food_ 
                && lhs.foodConsumed_ == rhs.foodConsumed_
                && lhs.antPos_ == rhs.antPos_
                && lhs.direction_ == rhs.direction_;
    }
private:
    FieldT field_;
    int steps_ = 0;
    int max_food_;
    int foodConsumed_ = 0;
    ant::sim::Pos2d antPos_;
    ant::sim::Direction direction_;
};



template<int XSize, int YSize>
using AntBoardSimulationStaticSize = AntBoardSimulation<std::array<std::array<BoardState, YSize>, XSize>>;

} // namespace ant
