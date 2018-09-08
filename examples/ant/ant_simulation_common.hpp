/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <array>

namespace ant::sim
{
    struct Pos2d
    {
        Pos2d(int xval, int yval)
            :pos{{xval, yval}}
        {
        }
                                      
        std::array<int, 2> pos;
        
        int & x(){ return pos[0]; }
        int & y(){ return pos[1]; }

        int x() const { return pos[0]; }
        int y() const { return pos[1]; }
    };

    Pos2d operator+(Pos2d const & lhs, Pos2d const & rhs)
    {
        return Pos2d{lhs.x() + rhs.x(), lhs.y() + rhs.y()};
    }

    bool operator==(Pos2d const & lhs, Pos2d const & rhs)
    {
        return lhs.pos == rhs.pos;
    }

    inline std::array<Pos2d, 4> const toPos{
        Pos2d{-1,  0},
        Pos2d{ 0,  1},
        Pos2d{ 1,  0},
        Pos2d{ 0, -1}
    };

    constexpr std::array<char, 4> directionToChar{{
        'N',
        'E',
        'S',
        'W',
    }};

    enum class Direction : size_t {
        north = 0,
        east  = 1,
        south = 2,
        west  = 3
    };

    Direction rotateCW(Direction p)
    {
        return static_cast<Direction>((static_cast<size_t>(p) + 1) % 4);
    }

    Direction rotateCCW(Direction p)
    {
        return static_cast<Direction>((static_cast<size_t>(p) + 3) % 4);
    }
}
