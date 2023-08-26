/*
 * This file is part of my ray caster.
 * (https://github.com/ericwoude/wolfenstein3d)
 *
 * The MIT License (MIT)
 *
 * Copyright © 2022 Eric van der Woude
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the “Software”), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef GAME_H
#define GAME_H

#include <cmath>
#include <memory>
#include <vector>

#include "enemy.h"
#include "level.h"
#include "player.h"
#include "utility.h"

///////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DECLARATIONS
///////////////////////////////////////////////////////////////////////////////

const int MAP_WIDTH = 12;
const int MAP_HEIGHT = 12;
const int MAX_DEPTH = std::max(MAP_WIDTH, MAP_HEIGHT);

typedef struct
{
    bool w = false;
    bool s = false;
} key_states;

///////////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////////

class game
{
   public:
    game() : p(200, 400), world(MAP_WIDTH, MAP_HEIGHT) {}

    void mouse_look(int dx, double dt)
    {
        int d_angle = 0;

        if (dx > 0) d_angle = std::max(1, (int)(dx / 25 * 0.2 * dt));
        if (dx < 0) d_angle = std::min(-1, (int)(dx / 25 * 0.2 * dt));

        p.angle = clamp_to_unit_circle(p.angle - d_angle);
    }

    void keys_handler(double dt)
    {
        const double dx = cos(degrees_to_radians(p.angle));
        const double dy = -sin(degrees_to_radians(p.angle));

        const int ox = (dx < 0) ? -10 : 10;
        const int oy = (dy < 0) ? -10 : 10;

        const int mx = p.x / 64.0;
        const int my = p.y / 64.0;

        if (keys.w)
        {
            int ipx_po = (p.x + ox) / 64.0;
            int ipy_po = (p.y + oy) / 64.0;

            if (world[my * MAP_WIDTH + ipx_po] == 0) p.x += 0.2 * dt * dx;
            if (world[ipy_po * MAP_WIDTH + mx] == 0) p.y += 0.2 * dt * dy;
        }

        if (keys.s)
        {
            int ipx_no = (p.x - ox) / 64.0;
            int ipy_no = (p.y - oy) / 64.0;

            if (world[my * MAP_WIDTH + ipx_no] == 0) p.x -= 0.2 * dt * dx;
            if (world[ipy_no * MAP_WIDTH + mx] == 0) p.y -= 0.2 * dt * dy;
        }
    }

    template <typename enemy_type>
    requires std::derived_from<enemy_type, enemy>
    void add_enemy(double x, double y, double z)
    {
        enemies.push_back(std::make_unique<enemy>(x, y, z));
    }

    key_states keys;
    player p;
    level world;
    std::vector<std::unique_ptr<enemy>> enemies;
};

#endif  // GAME_H