#ifndef TEXTURE_H
#define TEXTURE_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

namespace Engine
{
class Texture
{
   public:
    Texture(std::string name);

    uint8_t operator[](int i) const;
    uint8_t& operator[](int i);

   private:
    std::vector<uint8_t> data;

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
    void load(std::string name);
};
}  // namespace Engine

#endif  // TEXTURE_H