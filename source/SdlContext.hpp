#ifndef SdlContext_hpp
#define SdlContext_hpp true

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "SDL.h"
#pragma GCC diagnostic pop

#include <iostream>

namespace SdlContext {
    [[noreturn]]
    inline void errorOut() {
        std::cerr << SDL_GetError() << '\n';
        std::exit(EXIT_FAILURE);
    }

    inline constexpr void check(int const returnCode) {
        if (returnCode != 0) errorOut();
    }

    template <typename PointerT>
    inline constexpr std::enable_if_t<std::is_pointer_v<PointerT>, PointerT> check(PointerT const pointer) {
        if (pointer == nullptr) errorOut();
        else return pointer;
    }

    extern Uint64 deltaTime;
    extern SDL_Window *window;
    extern SDL_Renderer *renderer;
    extern Sint32 windowWidth;
    extern Sint32 windowHeight;

    extern void exitHandler();
    extern void mainLoop();
    extern void refreshWindow();
}


#endif
