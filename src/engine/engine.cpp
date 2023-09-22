#include "engine/engine.h"

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

    for (int ray = 0; ray < 121; ++ray)
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
            shade = 0.75;

        d_horizontal *= cos(degrees_to_radians(clamp_to_unit_circle(pa - r_angle)));
        int wall_height = (64 * SCREEN_HEIGHT) / d_horizontal;
        double ty_step = 32.0 / static_cast<double>(wall_height);
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
            tx = static_cast<int>(hy / 2) % 32;
            if (game.player.angle > 90 && game.player.angle < 270) tx = 31 - tx;
        }
        else
        {
            tx = static_cast<int>(hx / 2) % 32;
            if (game.player.angle > 180) tx = 31 - tx;
        }

        // Begin drawing
        depth_buffer[ray] = d_horizontal;
        glPointSize(8);
        glBegin(GL_POINTS);

        for (int y = 0; y < wall_height; ++y)
        {
            int pixel = (static_cast<int>(ty) * 32 + static_cast<int>(tx)) * 3;
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

void display()
{
    // Update delta time to get consistent game speed
    time_since_frame = glutGet(GLUT_ELAPSED_TIME);
    delta_time = time_since_frame - old_time_since_frame;
    old_time_since_frame = time_since_frame;

    game.keys_handler(delta_time);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_floor();
    render_scene();
    render_enemies();
    render_crosshair();

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

    glClearColor(0.25, 0.25, 0.25, 0);
    gluOrtho2D(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    glutSetCursor(GLUT_CURSOR_NONE);

    glutDisplayFunc(display);
    glutKeyboardFunc(button_down);
    glutKeyboardUpFunc(button_up);
    glutPassiveMotionFunc(look);

    glutMainLoop();
}
}  // namespace Engine