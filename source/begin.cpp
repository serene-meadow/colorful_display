
#include <cstdlib>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "SdlContext.hpp"

static void generateColorfulDisplay(Sint32 const width, Sint32 const height) {
    (void)width;
    (void)height;
}

static void drawOutwardColorGradient(double const centerX, double const centerY) {
    namespace Sdl = SdlContext;

    Sint32 const minLength{std::min(Sdl::windowWidth, Sdl::windowHeight)};
    double const colorUnit = 360.0 / static_cast<double>(minLength);
    
    for (Sint32 y{0}; y < Sdl::windowHeight; ++y) {
        for (Sint32 x{0}; x < Sdl::windowWidth; ++x) {
            double const distance{std::sqrt(
                std::pow(static_cast<double>(x) - centerX, 2.0) + std::pow(static_cast<double>(y) - centerY, 2.0)
            )};


        }
    }
}

int main() {
    namespace Sdl = SdlContext;

    Sdl::check(SDL_Init(SDL_INIT_VIDEO));

    std::atexit/* 1/32 */(&Sdl::exitHandler);

    Sdl::window = Sdl::check(SDL_CreateWindow(
        "Colorful Display",
        SDL_WINDOWPOS_CENTERED/* x position */, SDL_WINDOWPOS_CENTERED/* y position */,
        430/* width */, 430/* height */,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    ));

    Sdl::renderer = Sdl::check(SDL_CreateRenderer(Sdl::window, -1, 0u));

    SDL_SetRenderDrawColor(Sdl::renderer, 0u, 0u, 0u, 1u);

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(&Sdl::mainLoop, -1, true);
    #else
    while (true) Sdl::mainLoop();
    #endif

    return EXIT_SUCCESS;
}
