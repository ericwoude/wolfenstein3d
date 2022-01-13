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

#include <GL/glut.h>

#include <cmath>
#include <iostream>

#include "level.h"
#include "player.h"
#include "utility.h"

#define MAP_WIDTH 12
#define MAP_HEIGHT 12

#define MAX_DEPTH std::max(MAP_WIDTH, MAP_HEIGHT)

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 480

const int window = 1;

player p(150, 150);
level world(MAP_WIDTH, MAP_HEIGHT);

///////////////////////////////////////////////////////////////////////////////
// DRAWING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void draw_scene()
{
    const int fov = 80;
    int depth;
    int pos;

    double d_vertical, d_horizontal, d_min;
    double r_angle = clamp_to_unit_circle(p.angle + (fov / 2));

    // Vertical vector and horizontal vector
    double vx, vy;
    double hx, hy;

    // Offset vector
    double ox, oy;

    for (int ray = 0; ray < fov; ray++)
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
            vx = (((int)p.x >> 6) << 6) + 64;
            vy = (p.x - vx) * tangent + p.y;

            ox = 64;
            oy = -64 * tangent;
        }
        else if (cos(theta) < -epsilon)  // Points right
        {
            vx = (((int)p.x >> 6) << 6) - epsilon;
            vy = (p.x - vx) * tangent + p.y;

            ox = -64;
            oy = 64 * tangent;
        }
        else  // Points straight up or down, no hit
        {
            vx = p.x;
            vy = p.y;

            depth = MAX_DEPTH;
        }

        while (depth < MAX_DEPTH)
        {
            // Position relative to the grid
            pos = ((int)(vy) >> 6) * world.width + ((int)(vx) >> 6);

            if (pos > 0 && pos < world.width * world.height && world[pos] == 1)  // Hit
            {
                depth = MAX_DEPTH;
                d_vertical = cos(degrees_to_radians(p.angle)) * (vx - p.x) -
                             sin(degrees_to_radians(p.angle)) * (vy - p.y);
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

        if (sin(degrees_to_radians(r_angle)) > epsilon)  // Points up
        {
            hy = (((int)p.y >> 6) << 6) - epsilon;
            hx = (p.y - hy) * tangent + p.x;

            ox = 64 * tangent;
            oy = -64;
        }
        else if (sin(degrees_to_radians(r_angle)) < -epsilon)  // Points down
        {
            hy = (((int)p.y >> 6) << 6) + 64;
            hx = (p.y - hy) * tangent + p.x;

            ox = -64 * tangent;
            oy = 64;
        }
        else  // Points straight left or right, no hit
        {
            hx = p.x;
            hy = p.y;

            depth = MAX_DEPTH;
        }

        while (depth < MAX_DEPTH)
        {
            // Position relative to the grid
            pos = ((int)(hy) >> 6) * world.width + ((int)(hx) >> 6);

            if (pos > 0 && pos < world.width * world.height && world[pos] == 1)  // Hit
            {
                depth = MAX_DEPTH;
                d_horizontal = cos(degrees_to_radians(p.angle)) * (hx - p.x) -
                               sin(degrees_to_radians(p.angle)) * (hy - p.y);
            }
            else
            {
                hx += ox;
                hy += oy;
                depth += 1;
            }
        }

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

            glColor3f(0.9, 0, 0);
        }
        else
            glColor3f(0.7, 0, 0);

        // Draw walls
        d_horizontal *= cos(degrees_to_radians(clamp_to_unit_circle(p.angle - r_angle)));
        double wall_height = (64 * SCREEN_HEIGHT) / d_horizontal;
        double offset = (SCREEN_HEIGHT / 2) - (wall_height / 2);

        glLineWidth(8);
        glBegin(GL_LINES);
        glVertex2i(ray * 8, offset);
        glVertex2i(ray * 8, wall_height + offset);
        glEnd();

        r_angle = clamp_to_unit_circle(r_angle - 1);
    }
}

///////////////////////////////////////////////////////////////////////////////
// GLUT HOOKS
///////////////////////////////////////////////////////////////////////////////

void buttons(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'w':
        {
            double dx = cos(degrees_to_radians(p.angle));
            double dy = -sin(degrees_to_radians(p.angle));
            p.x += 5 * dx;
            p.y += 5 * dy;
        }
        break;

        case 's':
        {
            double dx = cos(degrees_to_radians(p.angle));
            double dy = -sin(degrees_to_radians(p.angle));
            p.x -= 5 * dx;
            p.y -= 5 * dy;
        }
        break;

        case 27:  // Escape key
        {
            glutDestroyWindow(window);
            exit(0);
        }
        break;
    }

    glutPostRedisplay();
}

void idle(void)
{
    if (glutGetWindow() != window) glutSetWindow(window);

    glutPostRedisplay();
}

void look(int x, int y)
{
    constexpr int center_x = SCREEN_WIDTH / 2;
    constexpr int center_y = SCREEN_HEIGHT / 2;

    // Ensures that the mouse stays centered
    if (x != center_x || y != center_y) glutWarpPointer(center_x, center_y);

    // Alter the player's look angle based on mouse x movement
    int dx = x - center_x;
    int d_angle = 0;
    if (dx > 0) d_angle = std::max(1, dx / 25);
    if (dx < 0) d_angle = std::min(-1, dx / 25);

    p.angle = clamp_to_unit_circle(p.angle - d_angle);

    glutPostRedisplay();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_scene();

    glutSwapBuffers();
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("Ray Caster");

    glClearColor(0.25, 0.25, 0.25, 0);
    gluOrtho2D(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    glutSetCursor(GLUT_CURSOR_NONE);

    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutKeyboardFunc(buttons);
    glutPassiveMotionFunc(look);
    glutMainLoop();
}