#pragma once
#include <SDL3/SDL.h>
#include <string>
namespace club {
class Window {
public:
 Window(const std::string& title,int width,int height);
 ~Window();
 Window(const Window&)=delete;
 Window& operator=(const Window&)=delete;
 SDL_Window* native() const { return window_; }
private:
 SDL_Window* window_{nullptr};
};
}
