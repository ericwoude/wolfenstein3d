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

    uint32_t operator[](int i) const;
    uint32_t& operator[](int i);

   private:
    std::vector<uint32_t> data;

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
    void load(std::string name);
};
}  // namespace Engine

#endif  // TEXTURE_H