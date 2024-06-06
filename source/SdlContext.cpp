#include "SdlContext.hpp"
#include "CartesianGrid2d.hpp"
#include "HslaColor.hpp"

namespace Project::SdlContext {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *canvasBuffer = nullptr;

    static Uint64 deltaTime{0u};
    #ifdef __EMSCRIPTEN__
    static int windowWidth{100}, windowHeight{100};
    #else
    static int windowWidth{430}, windowHeight{430};
    #endif
    static int mouseX{0}, mouseY{0};
}

Uint64 Project::SdlContext::getDeltaTime() { return deltaTime; }
int Project::SdlContext::getWindowHeight() { return windowHeight; }
int Project::SdlContext::getWindowWidth() { return windowWidth; }

void Project::SdlContext::exitHandler() {
    if (window != nullptr) SDL_DestroyWindow(window);
    if (renderer != nullptr) SDL_DestroyRenderer(renderer);
    if (canvasBuffer != nullptr) SDL_DestroyTexture(canvasBuffer);
    SDL_Quit();
}

void Project::SdlContext::mainLoop() {
    // Time of the previous iteration.
    static Uint64 previousTime{0u};

    // Get the time of this iteration.
    Uint64 const currentTime{SDL_GetTicks64()};

    // Get the change in time.
    deltaTime = currentTime - previousTime;

    // Handle events.
    static SDL_Event event;
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
        case SDL_MOUSEMOTION: {
            SDL_GetMouseState(&mouseX, &mouseY);
        } break;
        case SDL_QUIT:
            std::exit(EXIT_SUCCESS);
            break;
    }

    // Refresh the window.
    refreshWindow();

    // As this iteration ends, update the previous time.
    previousTime = currentTime;

    // Copy data from the canvas buffer to the window.
    SDL_RenderCopy(renderer, canvasBuffer, NULL, NULL);

    // Give the CPU a break.
    SDL_Delay(1u);
}

#include <array>

void Project::SdlContext::refreshWindow() {
    static HslaColor mainColor;

    static double percentage{0.0};
    double const deltaPercentage{static_cast<double>(deltaTime) * 0.0008};
    percentage = Utility::wrapValue(percentage + deltaPercentage, 1.0);
    mainColor.setHue(Utility::linearInterpolation(percentage, 0.0, 360.0));

    static constexpr auto drawColorGradient = []() -> void {
        int const minLength{std::min(windowWidth, windowHeight)};
        double const colorUnit = 2.0 * 360.0 / static_cast<double>(minLength);

        std::vector<SDL_FPoint> const sourcePointList = {
            // SDL_FPoint{windowWidth / 2.0f, windowHeight / 2.0f},
            SDL_FPoint{0.0f, 0.0f},
            // SDL_FPoint{windowWidth - 1.0f, 0.0f},
            // SDL_FPoint{0.0f, windowHeight - 1.0f},
            SDL_FPoint{static_cast<float>(mouseX), static_cast<float>(mouseY)},
            SDL_FPoint{windowWidth - 1.0f, windowHeight - 1.0f},
        };

        for (int y{0}; y < windowHeight; ++y) {
            for (int x{0}; x < windowWidth; ++x) {
                HslaColor hslaPixel(mainColor);

                std::uint_fast8_t count{0};
                for (auto const &point : sourcePointList) {
                    double const distance{std::sqrt(
                        std::pow(static_cast<double>(x) - point.x, 2.0) + std::pow(static_cast<double>(y) - point.y, 2.0)
                    )};

                    double const colorOffset{colorUnit * distance};

                    if (false or count++ % 2u == 0u) hslaPixel.setHue(hslaPixel.getHue() - colorOffset);
                    else hslaPixel.setHue(hslaPixel.getHue() + colorOffset);
                }

                SDL_Color const rgbaPixel(hslaPixel.toRgbaColor());

                check(SDL_SetRenderDrawColor(renderer, rgbaPixel.r, rgbaPixel.g, rgbaPixel.b, rgbaPixel.a));
                check(SDL_RenderDrawPoint(renderer, x, y));
            }
        }
    };

    drawColorGradient();
    SDL_RenderPresent(renderer);
}
