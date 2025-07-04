#include "core/Application.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        Morpheus::Core::Application app("Morpheus Renderer", 800, 600);
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}