#include "engine/Application.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <SDL3/SDL.h>

int main() {
    try {
        std::cerr << "ClubEngine: starting..." << std::endl;
        club::Application app;
        std::cerr << "ClubEngine: entering main loop" << std::endl;
        return app.run();
    } catch (const std::exception& e) {
        const std::string message = std::string("ClubEngine startup failed:\n") + e.what() +
            "\n\nSee ClubEngine.log next to the executable for the last initialization stage.";
        {
            std::ofstream log("ClubEngine.log", std::ios::app);
            log << "MAIN FATAL: " << e.what() << std::endl;
        }
        std::cerr << message << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ClubEngine startup error", message.c_str(), nullptr);
        return 1;
    }
}