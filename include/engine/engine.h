#ifndef ENGINE_H
#define ENGINE_H

#include <GL/freeglut.h>
#include <GL/glut.h>

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

#include "game.h"
#include "texture.h"

namespace Engine
{
///////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DECLARATIONS
///////////////////////////////////////////////////////////////////////////////

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 640;
const int FOV = 2 * atan(0.60);

///////////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////////

inline double old_time_since_frame = 0;
inline double time_since_frame = 0;
inline double delta_time = 0;

inline std::vector<Texture> textures;
inline std::vector<int> depth_buffer = std::vector<int>(SCREEN_WIDTH, 0);
const int window_id = 1;

inline Game game{};

///////////////////////////////////////////////////////////////////////////////
// RENDER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void render_crosshair();
void render_enemies();
void render_floor();
void render_scene();

///////////////////////////////////////////////////////////////////////////////
// GLUT HOOKS
///////////////////////////////////////////////////////////////////////////////

void button_down(unsigned char key, int x, int y);
void button_up(unsigned char key, int x, int y);
void look(int x, int y);
void display();
void initialize(int argc, char* argv[]);

}  // namespace Engine

#endif  // ENGINE_H