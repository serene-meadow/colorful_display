#ifndef SdlContext_hpp
#define SdlContext_hpp true

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wpedantic"
#include "SDL.h"
#pragma GCC diagnostic pop

#include <iostream>

namespace Project::SdlContext {
    [[noreturn]]
    inline void errorOut() {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    inline constexpr void check(int const returnCode) {
        if (returnCode != 0) errorOut();
    }

    template <typename PointerT>
    [[nodiscard]]
    inline constexpr std::enable_if_t<std::is_pointer_v<PointerT>, PointerT> check(PointerT const pointer) {
        if (pointer == nullptr) errorOut();
        else return pointer;
    }

    extern SDL_Window *window;
    extern SDL_Renderer *renderer;
    extern SDL_Texture *canvasBuffer;
    extern SDL_PixelFormat *pixelFormat;

    extern void refreshCachedWindowSize();

    extern Uint64 getDeltaTime();
    extern int getWindowWidth();
    extern int getWindowHeight();

    inline constexpr int canvasBufferWidth{270}, canvasBufferHeight{270};

    extern void exitHandler();
    extern void mainLoop();
    extern void refreshWindow();
}


#endif
