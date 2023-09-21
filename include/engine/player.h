#ifndef PLAYER_H
#define PLAYER_H

namespace Engine
{
class Player
{
   public:
    Player(double x_pos, double y_pos)
        : x(x_pos),
          y(y_pos)
    {
    }

    double x;
    double y;

    double angle = 90;
};
}  // namespace Engine

#endif  // PLAYER_H