#include "SdlContext.hpp"

namespace SdlContext {
    Uint64 deltaTime{0u};
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    Sint32 windowWidth{430};
    Sint32 windowHeight{430};
}

void SdlContext::exitHandler() {
    if (window != nullptr) SDL_DestroyWindow(window);
    if (renderer != nullptr) SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void SdlContext::mainLoop() {
    // Time of the previous iteration.
    static Uint64 previousTime{0u};

    // Get the time of this iteration.
    Uint64 const currentTime{SDL_GetTicks64()};

    // Get the change in time.
    deltaTime = currentTime - previousTime;

    static SDL_Event event{};
    while (SDL_PollEvent(&event)) switch (event.type) {
        case SDL_KEYDOWN: switch (event.key.keysym.sym) {
            case SDLK_BACKQUOTE:
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                break;
            case SDLK_ESCAPE:
                SDL_SetWindowFullscreen(window, 0u);
                break;
        } break;
        case SDL_WINDOWEVENT: switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
                break;
        } break;
        case SDL_QUIT:
            std::exit(EXIT_SUCCESS);
            break;
    }

    // Refresh the window.
    refreshWindow();

    // As this iteration ends, update the previous time.
    previousTime = currentTime;

    // Give the CPU a break.
    SDL_Delay(1u);
}

void SdlContext::refreshWindow() {
    ;;;
}
