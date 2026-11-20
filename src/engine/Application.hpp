#pragma once
#include "Window.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
namespace club {
class Application {
public:
 Application();
 int run();
private:
 bool running_{true};
 Window window_;
 Renderer renderer_;
 Camera camera_;
 Scene scene_;
};
}
