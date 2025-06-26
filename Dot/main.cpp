#include "dot.h"
#include "scenes/default_scene.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int TARGET_FPS = 60;

int main()
{
    new DefaultScene(); // Registers itself
    new NextScene();
    SceneManager::Get().ChangeSceneByName("Default");

    DotApp app;
    app.Run(1280, 720, "Dot.");
}