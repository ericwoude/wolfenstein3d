#include "engine/texture.h"

namespace Engine
{
Texture::Texture(std::string name)
{
    load(name);
}

uint32_t Texture::operator[](int i) const
{
    return data[i];
}

uint32_t& Texture::operator[](int i)
{
    return data[i];
}

/*
 * This statemachine loads PPM files.
 * It ensures the following image properties:
 * - magic number P6
 * - 64 by 64 pixels
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
            case 1: if (s != "64 64") goto error; break;
            case 2: if (s != "255")   goto error; break;
            default: {
                int state = 0;
                uint32_t color = 0;

                for (const uint8_t c : s) {
                    switch (state)
                    {
                        case 0: color |= (c << 16); break;  // red
                        case 1: color |= (c << 8); break;   // green
                        case 2:                             // blue
                        {
                            color |= c;
                            data.push_back(color);
                            
                            color = 0;
                            state = -1;
                            break;
                        }
                    }

                    state++;
                }
                break;
            }
            error:
            {
                std::cout << "Problem loading " + name + ":" << s << std::endl;
                exit(0);
            }
        }

        line++;
    }  // clang-format on
};
}  // namespace Engine