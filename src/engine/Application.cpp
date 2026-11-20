
#include "Application.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <iostream>
namespace club {
Application::Application() : window_("ClubEngine 0.3 | Vulkan",1280,720), renderer_(window_) { std::cout<<"ClubEngine 0.3 initialized. WASD + mouse; ESC to quit.\\n"<<std::flush; }
int Application::run(){bool captured=false;uint64_t last=SDL_GetTicks();while(running_){uint64_t now=SDL_GetTicks();float dt=std::min((now-last)/1000.0f,0.05f);last=now;SDL_Event e{};while(SDL_PollEvent(&e)){if(e.type==SDL_EVENT_QUIT)running_=false;if(e.type==SDL_EVENT_KEY_DOWN&&e.key.key==SDLK_ESCAPE){if(captured){SDL_SetWindowRelativeMouseMode(window_.native(),false);captured=false;}else running_=false;}if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN&&!captured){SDL_SetWindowRelativeMouseMode(window_.native(),true);captured=true;}if(e.type==SDL_EVENT_MOUSE_MOTION&&captured)camera_.rotate((float)e.motion.xrel,(float)e.motion.yrel);}const bool*keys=SDL_GetKeyboardState(nullptr);camera_.move(keys,dt);scene_.update(dt);int w=0,h=0;SDL_GetWindowSizeInPixels(window_.native(),&w,&h);renderer_.beginFrame();if(h>0)renderer_.drawRoom(camera_.projection((float)w/h)*camera_.view());renderer_.endFrame();}renderer_.waitIdle();return 0;}
}
