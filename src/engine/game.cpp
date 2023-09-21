#include "engine/game.h"

namespace Engine
{
void Game::mouse_look(int dx, double dt)
{
    int d_angle = 0;

    if (dx > 0) d_angle = std::max(1, (int)(dx / 25 * 0.2 * dt));
    if (dx < 0) d_angle = std::min(-1, (int)(dx / 25 * 0.2 * dt));

    player.angle = clamp_to_unit_circle(player.angle - d_angle);
}

void Engine::Game::keys_handler(double dt)
{
    const double dx = cos(degrees_to_radians(player.angle));
    const double dy = -sin(degrees_to_radians(player.angle));

    const int ox = (dx < 0) ? -10 : 10;
    const int oy = (dy < 0) ? -10 : 10;

    const int mx = player.x / 64.0;
    const int my = player.y / 64.0;

    if (keys.w)
    {
        int ipx_po = (player.x + ox) / 64.0;
        int ipy_po = (player.y + oy) / 64.0;

        if (level[my * MAP_WIDTH + ipx_po] == 0) player.x += 0.2 * dt * dx;
        if (level[ipy_po * MAP_WIDTH + mx] == 0) player.y += 0.2 * dt * dy;
    }

    if (keys.s)
    {
        int ipx_no = (player.x - ox) / 64.0;
        int ipy_no = (player.y - oy) / 64.0;

        if (level[my * MAP_WIDTH + ipx_no] == 0) player.x -= 0.2 * dt * dx;
        if (level[ipy_no * MAP_WIDTH + mx] == 0) player.y -= 0.2 * dt * dy;
    }
}
}  // namespace Engine