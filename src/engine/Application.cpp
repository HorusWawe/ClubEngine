#include "Application.hpp"
#include <SDL3/SDL.h>
#include <iostream>
namespace club {
Application::Application() : window_("ClubEngine 0.1 | Vulkan",1280,720), renderer_(window_) {
 std::cout << "ClubEngine " << CLUBENGINE_VERSION << " initialized.\\n";
}
int Application::run() {
 while (running_) {
  SDL_Event event{};
  while (SDL_PollEvent(&event)) {
   if (event.type == SDL_EVENT_QUIT) running_=false;
   if (event.type == SDL_EVENT_KEY_DOWN && event.key.key==SDLK_ESCAPE) running_=false;
  }
  renderer_.beginFrame();
  scene_.update(0.0f);
  renderer_.endFrame();
 }
 renderer_.waitIdle();
 return 0;
}
}
