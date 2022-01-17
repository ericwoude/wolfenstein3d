#ifndef TEXTURE_H
#define TEXTURE_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

using std::cout;
using std::ifstream;
using std::string;
using std::vector;

class texture
{
   public:
    texture(string name) { load(name); }

    int operator[](int i) const { return data[i]; }
    int& operator[](int i) { return data[i]; }

   private:
    vector<int> data;

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
    void load(string name)  // clang-format off
    {
        ifstream file("textures/" + name);
        string s;
        int line = 0;

        while (getline(file, s))
        {
            switch (line)
            {
                case 0: if (s != "P6")    goto error; break;
                case 1: if (s != "32 32") goto error; break;
                case 2: if (s != "255")   goto error; break;
                case 3: for (const int c : s) data.push_back(c); break;
                
                error: default:
                {
                    cout << "Problem loading " + name + ":" << s << '\n';
                    exit(0);
                }
            }

            line++;
        }  // clang-format on
    };
};

#endif  // TEXTURE_H