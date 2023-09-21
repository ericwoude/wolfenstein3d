#ifndef ENEMY_H
#define ENEMY_H

namespace Engine
{
class Enemy
{
   public:
    Enemy(double _x, double _y, double _z)
        : x(_x),
          y(_y),
          z(_z)
    {
    }

    ~Enemy() = default;

    double x;
    double y;
    double z;
};

class Skull : public Enemy
{
    Skull(double x, double y, double z)
        : Enemy{x, y, z}
    {
    }
};
}  // namespace Engine

#endif  // ENEMY_H