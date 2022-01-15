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

using std::vector;

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
        if (keys.w)
        {
            const double dx = cos(degrees_to_radians(p.angle));
            const double dy = -sin(degrees_to_radians(p.angle));

            p.x += 0.2 * dt * dx;
            p.y += 0.2 * dt * dy;
        }

        if (keys.s)
        {
            const double dx = cos(degrees_to_radians(p.angle));
            const double dy = -sin(degrees_to_radians(p.angle));

            p.x -= 0.2 * dt * dx;
            p.y -= 0.2 * dt * dy;
        }
    }

    void add_enemy(double x, double y, double z) { enemies.push_back(enemy(x, y, z)); }

    key_states keys;
    player p;
    level world;
    vector<enemy> enemies;
};

#endif  // GAME_H