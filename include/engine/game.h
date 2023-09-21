#ifndef GAME_H
#define GAME_H

#include <cmath>
#include <memory>
#include <vector>

#include "enemy.h"
#include "level.h"
#include "player.h"
#include "utility.h"

namespace Engine
{
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

class Game
{
   public:
    Game()
        : player{200, 400},
          level{MAP_WIDTH, MAP_HEIGHT}
    {
    }

    void mouse_look(int dx, double dt);

    void keys_handler(double dt);

    template <typename enemy_type>
    requires std::derived_from<enemy_type, Enemy>
    void add_enemy(double x, double y, double z)
    {
        enemies.push_back(std::make_unique<Enemy>(x, y, z));
    }

    key_states keys;
    Player player;
    Level level;
    std::vector<std::unique_ptr<Enemy>> enemies;
};
}  // namespace Engine

#endif  // GAME_H