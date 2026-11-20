#include "engine/Application.hpp"
#include <exception>
#include <iostream>
int main() {
 try { club::Application app; return app.run(); }
 catch (const std::exception& e) { std::cerr << "ClubEngine fatal error: " << e.what() << '\\n'; return 1; }
}
