/*
 * This file is part of my ray caster.
 * (https://github.com/ericwoude/wolfenstein3d)
 *
 * The MIT License (MIT)
 *
 * Copyright © 2022 Eric van der Woude
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the “Software”), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <limits>

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double epsilon = 0.0000000001;
const double pi = 3.1415926535897932385;

// // Functions
inline double degrees_to_radians(double degrees) { return (degrees * pi) / 180.0; }

inline double clamp_to_unit_circle(double degrees)
{
    if (degrees > 359) degrees -= 360;
    if (degrees < 0) degrees += 360;

    return degrees;
}
#endif  // UTILITY_H