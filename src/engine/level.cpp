#include "engine/level.h"

namespace Engine
{
int Level::operator[](int i) const
{
    return data[i];
}

int& Level::operator[](int i)
{
    return data[i];
}

// Initializes a level with borders (1's) and inside a big empty space (0's).
void Level::initialize()
{
    for (int j = 0; j < height; j++)
    {
        if (j == 0 || j == height - 1)
        {
            for (int i = 0; i < width; i++) data.push_back(1);
        }
        else
        {
            for (int i = 0; i < width; i++)
            {
                if (i == 0 || i == width - 1)
                    data.push_back(1);
                else
                    data.push_back(0);
            }
        }
    }
}
}  // namespace Engine