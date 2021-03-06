/*
 * This file is part of my ray caster.
 * (https://github.com/ericwoude/wolfenstein3d)
 *
 * The MIT License (MIT)
 *
 * Copyright © 2022 Eric van der Woude
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COpyRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <GL/glut.h>

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

#include "game.h"
#include "texture.h"

///////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DECLARATIONS
///////////////////////////////////////////////////////////////////////////////

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 640;
const int FOV = 2 * atan(0.60);

///////////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////////

double old_time_since_frame;
double time_since_frame;
double delta_time;

std::vector<texture> textures;
std::vector<int> depth_buffer = std::vector<int>(121, 0);
game g;
const int window_id = 1;

///////////////////////////////////////////////////////////////////////////////
// DRAWING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void draw_crosshair()
{
    constexpr int center_x = SCREEN_WIDTH / 2;
    constexpr int center_y = SCREEN_HEIGHT / 2;

    glColor3f(1, 1, 1);
    glLineWidth(1);

    glBegin(GL_LINES);
    glVertex2i(center_x - 15, center_y);  // Horizontal crosshair lines
    glVertex2i(center_x - 5, center_y);
    glVertex2i(center_x + 5, center_y);
    glVertex2i(center_x + 15, center_y);

    glVertex2i(center_x, center_y - 15);  // Vertical crosshair lines
    glVertex2i(center_x, center_y - 5);
    glVertex2i(center_x, center_y + 5);
    glVertex2i(center_x, center_y + 15);
    glEnd();
}

void draw_enemies()
{
    const double theta = degrees_to_radians(g.p.angle);

    double rx, ry;
    double sx, sy;

    for (const auto& enemy : g.enemies)
    {
        // Enemy position relative to player
        rx = enemy.x - g.p.x;
        ry = enemy.y - g.p.y;

        // Transformation matrix to compute world space coordinates
        double a = cos(theta) * ry + sin(theta) * rx;
        double b = -sin(theta) * ry + cos(theta) * rx;

        // Transform world coordinates to screen space
        sx = (a * 108.0 / b) + (120 / 2);
        sy = (enemy.z * 108.0 / b) + (80 / 2);

        int scale = 32 * 80 / b;
        scale = std::max(0, scale);
        scale = std::min(120, scale);

        glPointSize(8);
        glBegin(GL_POINTS);

        double tx = 0, ty = 31;
        double tx_step = 32.0 / scale, ty_step = 32.0 / scale;

        if (sx > 0 && sx < 120 && b < depth_buffer[(int)sx])
        {
            for (int x = sx - scale / 2; x < sx + scale / 2; x++)
            {
                ty = 31;
                for (int y = 0; y < scale; y++)
                {
                    int pixel = ((int)ty * 32 + (int)tx) * 3;
                    int r = textures[1][pixel];
                    int g = textures[1][pixel + 1];
                    int b = textures[1][pixel + 2];

                    if (r != 255 && g != 0 && b != 255)
                    {
                        glColor3ub(r, g, b);
                        glVertex2i(x * 8, (sy - y) * 8);
                    }

                    ty = std::max(0.0, ty - ty_step);
                }

                tx += tx_step;
            }
        }

        glEnd();
    }
}

void draw_floor()
{
    glColor3ub(65, 60, 40);
    glBegin(GL_QUADS);
    glVertex2i(0, SCREEN_HEIGHT / 2);
    glVertex2i(SCREEN_WIDTH, SCREEN_HEIGHT / 2);
    glVertex2i(SCREEN_WIDTH, SCREEN_HEIGHT);
    glVertex2i(0, SCREEN_HEIGHT);
    glEnd();
}

void draw_scene()
{
    int vmt = 0, hmt = 0;

    int depth;
    int pos;

    double px = g.p.x, py = g.p.y;
    double pa = g.p.angle;

    double d_vertical, d_horizontal;
    double r_angle = clamp_to_unit_circle(pa + 30);

    // Vertical vector and horizontal vector
    double vx, vy;
    double hx, hy;

    // Offset vector
    double ox, oy;

    for (int ray = 0; ray < 121; ray++)
    {
        double theta = degrees_to_radians(r_angle);
        double tangent = tan(theta);

        /*
         * Register vertical hits:
         *  - First we calculate if the ray points to the left or right,
         *    and we set the vertical ray and the offset multiplier accordingly.
         *  - Then we search for the first wall that the vertical ray hits, with
         *    an upper bound of depth.
         */
        depth = 0;
        d_vertical = infinity;

        if (cos(theta) > epsilon)  // Points left
        {
            vx = (((int)px >> 6) << 6) + 64;
            vy = (px - vx) * tangent + py;

            ox = 64;
            oy = -64 * tangent;
        }
        else if (cos(theta) < -epsilon)  // Points right
        {
            vx = (((int)px >> 6) << 6) - epsilon;
            vy = (px - vx) * tangent + py;

            ox = -64;
            oy = 64 * tangent;
        }
        else  // Points straight up or down, no hit
        {
            vx = px;
            vy = py;

            depth = MAX_DEPTH;
        }

        while (depth < MAX_DEPTH)
        {
            // Position relative to the grid
            pos = ((int)(vy) >> 6) * g.world.width + ((int)(vx) >> 6);

            if (pos > 0 && pos < g.world.width * g.world.height && g.world[pos] > 0)  // Hit
            {
                depth = MAX_DEPTH;
                d_vertical = cos(theta) * (vx - px) - sin(theta) * (vy - py);
                vmt = g.world[pos] - 1;
            }
            else
            {
                vx += ox;
                vy += oy;

                depth += 1;
            }
        }

        /*
         * Register horizontal hits:
         *  - First we calculate if the ray points to the up or down,
         *    and we set the horizontal ray and the offset multiplier accordingly.
         *  - Then we search for the first wall that the horizontal ray hits, with
         *    an upper bound of depth.
         */
        tangent = 1.0 / tangent;
        depth = 0;
        d_horizontal = infinity;

        if (sin(theta) > epsilon)  // Points up
        {
            hy = (((int)py >> 6) << 6) - epsilon;
            hx = (py - hy) * tangent + px;

            ox = 64 * tangent;
            oy = -64;
        }
        else if (sin(theta) < -epsilon)  // Points down
        {
            hy = (((int)py >> 6) << 6) + 64;
            hx = (py - hy) * tangent + px;

            ox = -64 * tangent;
            oy = 64;
        }
        else  // Points straight left or right, no hit
        {
            hx = px;
            hy = py;

            depth = MAX_DEPTH;
        }

        while (depth < MAX_DEPTH)
        {
            // Position relative to the grid
            pos = ((int)(hy) >> 6) * g.world.width + ((int)(hx) >> 6);
            if (pos > 0 && pos < g.world.width * g.world.height && g.world[pos] > 0)  // Hit
            {
                depth = MAX_DEPTH;
                d_horizontal = cos(theta) * (hx - px) - sin(theta) * (hy - py);
                hmt = g.world[pos] - 1;
            }
            else
            {
                hx += ox;
                hy += oy;
                depth += 1;
            }
        }

        double shade = 1;

        /*
         * We take the ray with the shortest distance to draw the scene.
         * To create the illusion of shadows, we use a different color
         * for vertical hits and horizontal hits.
         */
        if (d_vertical < d_horizontal)
        {
            hx = vx;
            hy = vy;
            d_horizontal = d_vertical;
            hmt = vmt;
        }
        else
            shade = 0.75;

        d_horizontal *= cos(degrees_to_radians(clamp_to_unit_circle(pa - r_angle)));
        int wall_height = (64 * SCREEN_HEIGHT) / d_horizontal;
        double ty_step = 32.0 / (double)wall_height;
        double ty_offset = 0;

        if (wall_height > SCREEN_HEIGHT)
        {
            ty_offset = (wall_height - SCREEN_HEIGHT) / 2;
            wall_height = SCREEN_HEIGHT;
        }

        int offset = (SCREEN_HEIGHT / 2) - (wall_height >> 1);

        double tx;
        double ty = ty_offset * ty_step;

        if (shade == 1)
        {
            tx = (int)(hy / 2) % 32;
            if (g.p.angle > 90 && g.p.angle < 270) tx = 31 - tx;
        }
        else
        {
            tx = (int)(hx / 2) % 32;
            if (g.p.angle > 180) tx = 31 - tx;
        }

        // Begin drawing
        depth_buffer[ray] = d_horizontal;
        glPointSize(8);
        glBegin(GL_POINTS);

        for (int y = 0; y < wall_height; y++)
        {
            int pixel = ((int)ty * 32 + (int)tx) * 3;
            int r = textures[hmt][pixel] * shade;
            int g = textures[hmt][pixel + 1] * shade;
            int b = textures[hmt][pixel + 2] * shade;

            glColor3ub(r, g, b);

            glVertex2i(ray * 8, y + offset);

            ty += ty_step;
        }

        glEnd();

        r_angle = clamp_to_unit_circle(r_angle - 0.5);
    }

    glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////
// GLUT HOOKS
///////////////////////////////////////////////////////////////////////////////

void button_down(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'w': g.keys.w = true; break;
        case 's': g.keys.s = true; break;
        case 27:
        {
            glutDestroyWindow(window_id);
            exit(0);
        }
        break;
    }

    glutPostRedisplay();
}

void button_up(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'w': g.keys.w = false; break;
        case 's': g.keys.s = false; break;
    }

    glutPostRedisplay();
}

void look(int x, int y)
{
    constexpr int center_x = SCREEN_WIDTH / 2;
    constexpr int center_y = SCREEN_HEIGHT / 2;

    // Ensures that the mouse stays centered
    if (x != center_x || y != center_y) glutWarpPointer(center_x, center_y);

    // Alter the player's look angle based on mouse x movement
    int delta_x = x - center_x;
    g.mouse_look(delta_x, delta_time);

    glutPostRedisplay();
}

void display()
{
    // Update delta time to get consistent game speed
    time_since_frame = glutGet(GLUT_ELAPSED_TIME);
    delta_time = time_since_frame - old_time_since_frame;
    old_time_since_frame = time_since_frame;

    g.keys_handler(delta_time);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_floor();
    draw_scene();
    draw_enemies();
    draw_crosshair();

    glutSwapBuffers();
}

void init()
{
    if (glutGetWindow() != window_id) glutSetWindow(window_id);

    glClearColor(0.25, 0.25, 0.25, 0);
    gluOrtho2D(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    glutSetCursor(GLUT_CURSOR_NONE);
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("Ray Caster");

    init();

    textures.push_back(texture("wall.ppm"));
    textures.push_back(texture("skull.ppm"));

    g.add_enemy(250, 400, 15);

    glutDisplayFunc(display);
    glutKeyboardFunc(button_down);
    glutKeyboardUpFunc(button_up);
    glutPassiveMotionFunc(look);

    glutMainLoop();
}