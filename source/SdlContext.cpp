#include "SdlContext.hpp"
#include "CartesianGrid2d.hpp"
#include "HslaColor.hpp"

namespace Project::SdlContext {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    static Uint64 deltaTime{0u};
    static int windowWidth{430};
    static int windowHeight{430};

    static CartesianGrid2d<HslaColor/* value type */, int/* size type */> colorGrid(
        windowHeight/* row count */,
        windowWidth/* column count*/
    );
}

Uint64 Project::SdlContext::getDeltaTime() { return deltaTime; }
int Project::SdlContext::getWindowHeight() { return windowHeight; }
int Project::SdlContext::getWindowWidth() { return windowWidth; }

void Project::SdlContext::exitHandler() {
    if (window != nullptr) SDL_DestroyWindow(window);
    if (renderer != nullptr) SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void Project::SdlContext::mainLoop() {
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
                std::exit(EXIT_FAILURE);
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

void Project::SdlContext::refreshWindow() {

    static constexpr auto drawOutwardColorGradient = [](double const centerX, double const centerY) -> void {
        int const minLength{std::min(windowWidth, windowHeight)};
        double const colorUnit = 360.0 / static_cast<double>(minLength);
        
        for (int y{0}; y < windowHeight; ++y) {
            for (int x{0}; x < windowWidth; ++x) {
                double const distance{std::sqrt(
                    std::pow(static_cast<double>(x) - centerX, 2.0) + std::pow(static_cast<double>(y) - centerY, 2.0)
                )};

                double const colorOffset{colorUnit * distance};

                HslaColor hslaPixel = colorGrid.at(y/* row */, x/* column */);

                hslaPixel.setHue(hslaPixel.getHue() + colorOffset);

                SDL_Color const rgbaPixel(hslaPixel.toRgbaColor());

                check(SDL_SetRenderDrawColor(renderer, rgbaPixel.r, rgbaPixel.g, rgbaPixel.b, rgbaPixel.a));
                check(SDL_RenderDrawPoint(renderer, x, y));
            }
        }
    };

    drawOutwardColorGradient(windowWidth / 2, windowHeight / 2);

}
