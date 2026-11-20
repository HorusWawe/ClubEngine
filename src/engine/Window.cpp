#include "Window.hpp"
#include <stdexcept>
namespace club {
Window::Window(const std::string& title,int width,int height) {
 if (!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)) throw std::runtime_error(std::string("SDL_Init: ")+SDL_GetError());
 window_=SDL_CreateWindow(title.c_str(),width,height,SDL_WINDOW_VULKAN|SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY);
 if (!window_) { std::string e=SDL_GetError(); SDL_Quit(); throw std::runtime_error("SDL_CreateWindow: "+e); }
}
Window::~Window() { if(window_) SDL_DestroyWindow(window_); SDL_Quit(); }
}
