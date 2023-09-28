#include "engine/engine.h"

int main(int argc, char* argv[])
{
    Engine::textures.push_back(Engine::Texture("wood.ppm"));
    Engine::textures.push_back(Engine::Texture("eagle.ppm"));

    Engine::textures.push_back(Engine::Texture("skull.ppm"));
    Engine::game.add_enemy<Engine::Skull>(250, 400, 15);

    Engine::initialize(argc, argv);
}