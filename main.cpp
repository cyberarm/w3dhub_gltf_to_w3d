#define TINYGLTF_IMPLEMENTATION

#include "window.h"
#include <SDL3/SDL_main.h>

int main(int, char **) {
    Window window = Window();
    window.init();
    window.run();

    return 0;
}
