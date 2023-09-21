#include "engine/texture.h"

namespace Engine
{
Texture::Texture(std::string name)
{
    load(name);
}

uint8_t Texture::operator[](int i) const
{
    return data[i];
}

uint8_t& Texture::operator[](int i)
{
    return data[i];
}

/*
 * This statemachine loads PPM files.
 * It ensures the following image properties:
 * - magic number P6
 * - 32 by 32 pixels
 * - colours expressed in unsigned bytes (255)
 *
 * In case of violation of these properties, the program
 * states the problem and exits.
 */
void Texture::load(std::string name)
{
    std::ifstream file("data/textures/" + name);
    std::string s;
    int line = 0;

    while (std::getline(file, s))  // clang-format off
    {
        switch (line)
        {
            case 0: if (s != "P6")    goto error; break;
            case 1: if (s != "32 32") goto error; break;
            case 2: if (s != "255")   goto error; break;
            case 3: for (const uint8_t c : s) data.push_back(c); break;
            
            error: default:
            {
                std::cout << "Problem loading " + name + ":" << s << std::endl;
                exit(0);
            }
        }

        line++;
    }  // clang-format on
};
}  // namespace Engine