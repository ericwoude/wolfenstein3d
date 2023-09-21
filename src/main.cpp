#include "engine/engine.h"

int main(int argc, char* argv[])
{
    Engine::textures.push_back(Engine::Texture("wall.ppm"));
    Engine::textures.push_back(Engine::Texture("skull.ppm"));

    Engine::game.add_enemy<Engine::Skull>(250, 400, 15);

    Engine::initialize(argc, argv);
}