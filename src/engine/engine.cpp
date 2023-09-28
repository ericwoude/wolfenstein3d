#include "engine/engine.h"

#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#include <cstring>
#include <iostream>

namespace Engine
{
///////////////////////////////////////////////////////////////////////////////
// RENDER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void render_crosshair()
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

void render_enemies()
{
    const double theta = degrees_to_radians(game.player.angle);

    double rx, ry;
    double sx, sy;

    for (const auto& enemy : game.enemies)
    {
        // Enemy position relative to player
        rx = enemy->x - game.player.x;
        ry = enemy->y - game.player.y;

        // Transformation matrix to compute world space coordinates
        double a = cos(theta) * ry + sin(theta) * rx;
        double b = -sin(theta) * ry + cos(theta) * rx;

        // Transform world coordinates to screen space
        sx = (a * 108.0 / b) + (120 / 2);
        sy = (enemy->z * 108.0 / b) + (80 / 2);

        int scale = 32 * 80 / b;
        scale = std::max(0, scale);
        scale = std::min(120, scale);

        glPointSize(8);
        glBegin(GL_POINTS);

        double tx = 0, ty = 31;
        double tx_step = 32.0 / scale, ty_step = 32.0 / scale;

        if (sx > 0 && sx < 120 && b < depth_buffer[static_cast<int>(sx)])
        {
            for (int x = sx - scale / 2; x < sx + scale / 2; ++x)
            {
                ty = 31;
                for (int y = 0; y < scale; ++y)
                {
                    int pixel = (static_cast<int>(ty) * 32 + static_cast<int>(tx)) * 3;
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

void render_floor()
{
    glColor3ub(65, 60, 40);
    glBegin(GL_QUADS);
    glVertex2i(0, SCREEN_HEIGHT / 2);
    glVertex2i(SCREEN_WIDTH, SCREEN_HEIGHT / 2);
    glVertex2i(SCREEN_WIDTH, SCREEN_HEIGHT);
    glVertex2i(0, SCREEN_HEIGHT);
    glEnd();
}

/*
 * Register vertical hits:
 *  - First we calculate if the ray points to the left or right,
 *    and we set the vertical ray and the offset multiplier accordingly.
 *  - Then we search for the first wall that the vertical ray hits, with
 *    an upper bound of depth.
 */
double calculate_vertical_hits(double theta, double tangent, double& vx, double& vy, int& vmt)
{
    const double py = game.player.y;
    const double px = game.player.x;

    double d_vertical = INFINITY;
    double ox, oy;

    if (cos(theta) > EPSILON)  // Points left
    {
        vx = ((static_cast<int>(px) >> 6) << 6) + 64;
        vy = (px - vx) * tangent + py;

        ox = 64;
        oy = -64 * tangent;
    }
    else if (cos(theta) < -EPSILON)  // Points right
    {
        vx = ((static_cast<int>(px) >> 6) << 6) - EPSILON;
        vy = (px - vx) * tangent + py;

        ox = -64;
        oy = 64 * tangent;
    }
    else  // Points straight up or down, no hit
    {
        vx = px;
        vy = py;

        return d_vertical;
    }

    for (int depth = 0; depth < MAX_DEPTH; ++depth)
    {
        // Calculate Position relative to the grid and see if there is a hit.
        int pos = (static_cast<int>(vy) >> 6) * game.level.width + (static_cast<int>(vx) >> 6);
        bool hit = pos > 0 && pos < game.level.width * game.level.height && game.level[pos] > 0;
        if (hit)
        {
            vmt = game.level[pos] - 1;
            d_vertical = cos(theta) * (vx - px) - sin(theta) * (vy - py);

            break;
        }

        vx += ox;
        vy += oy;
    }

    return d_vertical;
}

/*
 * Register horizontal hits:
 *  - First we calculate if the ray points to the up or down,
 *    and we set the horizontal ray and the offset multiplier accordingly.
 *  - Then we search for the first wall that the horizontal ray hits, with
 *    an upper bound of depth.
 */
double calculate_horizontal_hits(double theta, double tangent, double& hx, double& hy, int& hmt)
{
    tangent = 1.0 / tangent;

    const double py = game.player.y;
    const double px = game.player.x;

    double d_horizontal = INFINITY;
    double ox, oy;

    if (sin(theta) > EPSILON)  // Points up
    {
        hy = ((static_cast<int>(py) >> 6) << 6) - EPSILON;
        hx = (py - hy) * tangent + px;

        ox = 64 * tangent;
        oy = -64;
    }
    else if (sin(theta) < -EPSILON)  // Points down
    {
        hy = ((static_cast<int>(py) >> 6) << 6) + 64;
        hx = (py - hy) * tangent + px;

        ox = -64 * tangent;
        oy = 64;
    }
    else  // Points straight left or right, no hit
    {
        hx = px;
        hy = py;

        return d_horizontal;
    }

    for (int depth = 0; depth < MAX_DEPTH; ++depth)
    {
        // Calculate Position relative to the grid and see if there is a hit.
        int pos = (static_cast<int>(hy) >> 6) * game.level.width + (static_cast<int>(hx) >> 6);
        bool hit = pos > 0 && pos < game.level.width * game.level.height && game.level[pos] > 0;
        if (hit)
        {
            hmt = game.level[pos] - 1;
            d_horizontal = cos(theta) * (hx - px) - sin(theta) * (hy - py);

            break;
        }

        hx += ox;
        hy += oy;
    }

    return d_horizontal;
}

void render_scene()
{
    int vmt = 0, hmt = 0;

    double pa = game.player.angle;
    double r_angle = clamp_to_unit_circle(pa + 30);

    // Vertical vector and horizontal vector representing a ray
    double vx, vy;
    double hx, hy;

    // Reset pixel buffer
    std::memset(pixel_buffer, 0, RENDER_WIDTH * RENDER_HEIGHT * 3 * sizeof(uint8_t));

    for (int ray = 0; ray < RENDER_WIDTH; ++ray)
    {
        double theta = degrees_to_radians(r_angle);
        double tangent = tan(theta);

        // These functions change the values of hx, vx, hmt, vx, vy, vmt.
        double d_horizontal = calculate_horizontal_hits(theta, tangent, hx, vx, hmt);
        double d_vertical = calculate_vertical_hits(theta, tangent, vx, vy, vmt);
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
            shade = 0.5;

        d_horizontal *= cos(degrees_to_radians(clamp_to_unit_circle(pa - r_angle)));
        depth_buffer[ray] = d_horizontal;

        int wall_height = (64 * RENDER_HEIGHT) / d_horizontal;

        double ty_step = 64.0 / static_cast<double>(wall_height);
        double ty_offset = 0;

        if (wall_height > RENDER_HEIGHT)
        {
            ty_offset = (wall_height - RENDER_HEIGHT) / 2;
            wall_height = RENDER_HEIGHT;
        }

        int offset = (RENDER_HEIGHT / 2) - (wall_height >> 1);

        double tx;
        double ty = ty_offset * ty_step;

        if (shade == 1)
        {
            tx = static_cast<int>(hy) % 64;
            if (game.player.angle > 90 && game.player.angle < 270) tx = 63 - tx;
        }
        else
        {
            tx = static_cast<int>(hx) % 64;
            if (game.player.angle > 180) tx = 63 - tx;
        }

        // walls
        for (int y = 0; y < wall_height; ++y)
        {
            const int pixel = (static_cast<int>(ty) * 64 + static_cast<int>(tx));
            const uint32_t color = textures[hmt][pixel];

            const uint8_t r = color >> 16;
            const uint8_t g = (color >> 8) & 0xFF;
            const uint8_t b = color & 0xFF;

            if (r > 0 || g > 0 || b > 0)
            {
                int pixel_pointer = ((y + offset) * (RENDER_WIDTH) + ray) * 3;
                pixel_buffer[pixel_pointer++] = r;
                pixel_buffer[pixel_pointer++] = g;
                pixel_buffer[pixel_pointer++] = b;
            }

            ty += ty_step;
        }

        r_angle = clamp_to_unit_circle(r_angle - 60.0 / RENDER_WIDTH);
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
        case 'w': Engine::game.keys.w = true; break;
        case 'a': Engine::game.keys.a = true; break;
        case 's': Engine::game.keys.s = true; break;
        case 'd': Engine::game.keys.d = true; break;
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
        case 'w': game.keys.w = false; break;
        case 'a': game.keys.a = false; break;
        case 's': game.keys.s = false; break;
        case 'd': game.keys.d = false; break;
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
    game.mouse_look(delta_x, delta_time);

    glutPostRedisplay();
}

void load_texture()
{
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Set texture parameters (you can adjust these as needed)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Load the pixel data into the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 pixel_buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void render_texture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_scene();
    load_texture();
    render_texture();

    // Update delta time to get consistent game speed
    time_since_frame = glutGet(GLUT_ELAPSED_TIME);
    delta_time = time_since_frame - old_time_since_frame;
    old_time_since_frame = time_since_frame;
    game.keys_handler(delta_time);

    int fps = static_cast<int>(1000.0 / delta_time);
    const unsigned char* t = reinterpret_cast<const unsigned char*>(std::to_string(fps).c_str());
    glRasterPos2i(0, 0);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, t);

    glutSwapBuffers();
}

void initialize(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("Ray Caster");

    if (glutGetWindow() != window_id)
    {
        glutSetWindow(window_id);
    }

    glutSetCursor(GLUT_CURSOR_NONE);
    glutDisplayFunc(display);
    glutKeyboardFunc(button_down);
    glutKeyboardUpFunc(button_up);

    glutMainLoop();
}
}  // namespace Engine