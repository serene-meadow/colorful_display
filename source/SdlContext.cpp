#include <algorithm>
#include <optional>
#include <unordered_map>
#include <array>
#include "SdlContext.hpp"
#include "HslaColor.hpp"

namespace Project::SdlContext {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *canvasBuffer = nullptr;
    SDL_PixelFormat *pixelFormat = nullptr;

    static Uint64 deltaTime{0u};
    static int windowWidth{430}, windowHeight{430};

    /*
        This point represents the position on the canvas buffer, not the actual displayed window.
        If this is null, then the user is not pressing the mouse button.
    */
    static std::optional<SDL_FPoint> mouse = std::nullopt;

    static float hueSummand{0.0f};

    static inline constexpr double customExponential(double const percentage) {
        double const x{linearInterpolation(percentage, -43.165, 54.4)};
        return 4.9 * std::exp(0.07 * x);
    }

    static inline void decayHueSummand(double const percentage) {
        /**/ if (hueSummand > 0.0f)
            hueSummand = std::max<float>(0.0f, hueSummand - customExponential(percentage));
        else if (hueSummand < 0.0f)
            hueSummand = std::min<float>(0.0f, hueSummand + customExponential(percentage));
    }

    struct NumberedPoint : SDL_FPoint {
        using NumberType = std::uint_least8_t;
        NumberType number;
        explicit constexpr NumberedPoint(SDL_FPoint const &point, std::uint_least8_t const number):
            SDL_FPoint(point), number{number}
        {}
    };
    static std::unordered_map<SDL_FingerID, NumberedPoint> fingerMap;
}

void Project::SdlContext::refreshCachedWindowSize() {
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
}

Uint64 Project::SdlContext::getDeltaTime() { return deltaTime; }
int Project::SdlContext::getWindowHeight() { return windowHeight; }
int Project::SdlContext::getWindowWidth() { return windowWidth; }

void Project::SdlContext::exitHandler() {
    if (window != nullptr) SDL_DestroyWindow(window);
    if (renderer != nullptr) SDL_DestroyRenderer(renderer);
    if (canvasBuffer != nullptr) SDL_DestroyTexture(canvasBuffer);
    if (pixelFormat != nullptr) SDL_FreeFormat(pixelFormat);
    SDL_Quit();
}

/**
 * @note Not thread-safe.
 */
void Project::SdlContext::mainLoop() {
    // Time of the previous iteration.
    static Uint64 previousTime{0u};

    // Get the time of this iteration.
    Uint64 const currentTime{SDL_GetTicks64()};

    // Get the change in time.
    deltaTime = currentTime - previousTime;

    static double decayRateTimerPercentage{0.0};

    [[maybe_unused]] static bool mouseRightButtonIsPressed{false};

    static double mousePowerLevelPercentage{0.0};

    static SDL_Event event;
    // Handle events.
    /*
        This is the switch statement of greatness.
    */
    while (SDL_PollEvent(&event)) switch (event.type) {
        case SDL_KEYDOWN: switch (event.key.keysym.sym) {
            case SDLK_BACKQUOTE:
                // "Real" fullscreen is buggy in the browser.
                #ifdef __EMSCRIPTEN__
                check(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP/* "fake" fullscreen */));
                #else
                check(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN/* "real" fullscreen */));
                #endif
                break;
            case SDLK_ESCAPE:
                check(SDL_SetWindowFullscreen(window, 0u));
                break;
        } break;
        case SDL_MOUSEBUTTONDOWN: switch (event.button.button) {
            case SDL_BUTTON_LEFT:
                mouse = SDL_FPoint{
                    linearInterpolation<float>(
                        static_cast<float>(event.button.x) / static_cast<float>(windowWidth), 0.0f, canvasBufferWidth
                    ),
                    linearInterpolation<float>(
                        static_cast<float>(event.button.y) / static_cast<float>(windowHeight), 0.0f, canvasBufferHeight
                    )
                };
                break;
            case SDL_BUTTON_MIDDLE:
                hueSummand = 0.0f;
                break;
            case SDL_BUTTON_RIGHT:
                mouseRightButtonIsPressed = true;
                break;
        } break;
        case SDL_MOUSEMOTION:
            if (mouse.has_value()) mouse = {
                linearInterpolation<float>(
                    static_cast<float>(event.motion.x) / static_cast<float>(windowWidth), 0.0f, canvasBufferWidth
                ),
                linearInterpolation<float>(
                    static_cast<float>(event.motion.y) / static_cast<float>(windowHeight), 0.0f, canvasBufferHeight
                )
            };
            break;
        case SDL_MOUSEBUTTONUP: switch (event.button.button) {
            case SDL_BUTTON_LEFT:
                mouse = std::nullopt;
                break;
            case SDL_BUTTON_MIDDLE:
                /* no operation; do nothing */;
                break;
            case SDL_BUTTON_RIGHT:
                mouseRightButtonIsPressed = false;
                break;
        } break;
        case SDL_MOUSEWHEEL:
            /* no operation; do nothing */;
            break;
        case SDL_FINGERMOTION: {
            auto const iter(fingerMap.find(event.tfinger.fingerId));
            if (iter != fingerMap.end()) {
                auto &point = iter->second;
                point.x = event.tfinger.x * canvasBufferWidth;
                point.y = event.tfinger.y * canvasBufferHeight;
                break;
            } else {
                // If the finger identifier is not in the map, insert it into the map.
                goto insertFingerIdentifierIntoMap;
            }
        }
        case SDL_FINGERDOWN:
            insertFingerIdentifierIntoMap: fingerMap.emplace(
                /* key */ event.tfinger.fingerId,
                /* value */ NumberedPoint(
                    SDL_FPoint{event.tfinger.x * canvasBufferWidth, event.tfinger.y * canvasBufferHeight},
                    static_cast<NumberedPoint::NumberType>(fingerMap.size())
                )
            );
            break;
        case SDL_FINGERUP:
            fingerMap.erase(event.tfinger.fingerId);
            break;
        case SDL_MULTIGESTURE: if (std::fabs(event.mgesture.dDist/* pinch distance */) > 0.002f/* threshold */) {
            hueSummand += 8.0 + 1.15 * event.mgesture.dDist/* pinch distance */ * event.mgesture.numFingers;
        } break;
        case SDL_QUIT:
            std::exit(EXIT_SUCCESS);
            break;
    }

    if (mouse.has_value() or not fingerMap.empty() or mouseRightButtonIsPressed) {
        decayRateTimerPercentage = 0.0;
    } else decayHueSummand(
        decayRateTimerPercentage = std::clamp(decayRateTimerPercentage + static_cast<double>(deltaTime) * 0.00005, 0.0, 1.0)
    );

    if (mouseRightButtonIsPressed) hueSummand += customExponential(
        mousePowerLevelPercentage = std::clamp(mousePowerLevelPercentage + static_cast<double>(deltaTime) * 0.00005, 0.0, 1.0)
    ); else /* mouse right button is not pressed */ {
        mousePowerLevelPercentage = std::clamp(mousePowerLevelPercentage - static_cast<double>(deltaTime) * 0.00005, 0.0, 1.0);
    }

    #ifndef __EMSCRIPTEN__
    println(
        "Decay Timer: ", decayRateTimerPercentage,
        ", Mouse Power: ", mousePowerLevelPercentage,
        ", Scroll Value: ", hueSummand
    );
    #endif

    refreshWindow();

    // As this iteration ends, update the previous time.
    previousTime = currentTime;

    // Give the CPU a break?
    SDL_Delay(1u);
}

/** 
 * @note Not thread-safe.
 */
void Project::SdlContext::refreshWindow() {
    static HslaColor mainColor;

    static double huePercentage{0.0};
    huePercentage = wrapValue(huePercentage + static_cast<double>(deltaTime) * (0.0008), 1.0);

    static double sourceFunctionPercentage{0.0};
    sourceFunctionPercentage = wrapValue(sourceFunctionPercentage + static_cast<double>(deltaTime) * (0.000025), 1.0);

    mainColor.setHue(linearInterpolation(huePercentage, 0.0, 360.0));

    static constexpr int const minLength{std::min(canvasBufferWidth, canvasBufferHeight)};
    static constexpr double const hueUnit{2.0 * 360.0 / static_cast<double>(minLength)};

    void *pixelPointer;
    int pitch;
    check(SDL_LockTexture(canvasBuffer, nullptr/* lock entire texture */, &pixelPointer, &pitch));

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnarrowing"
    int const bytesPerPixel{SDL_BYTESPERPIXEL(pixelFormat->format)};
    #pragma GCC diagnostic pop
    int const pixelRowLength{pitch / bytesPerPixel};

    Uint32 *const pixelArray = static_cast<Uint32 *>(pixelPointer);

    static constexpr auto outlineCanvas = [](float const percentage) constexpr -> SDL_FPoint {
        /****/ if (percentage <= .25) {
            return {linearInterpolation<float>(percentage * 4.0, 0.0f, canvasBufferWidth), 0.0};
        } else if (percentage <= .50) {
            return {canvasBufferWidth, linearInterpolation<float>((percentage - .25) * 4.0, 0.0f, canvasBufferHeight)};
        } else if (percentage <= .75) {
            return {linearInterpolation<float>((percentage - .50) * 4.0, canvasBufferWidth, 0.0f), canvasBufferHeight};
        } else {
            return {0.0, linearInterpolation<float>((percentage - .75) * 4.0, canvasBufferHeight, 0.0f)};
        }
    };

    // Parametric functions.
    static constexpr std::array sourceFunctionList{
        +[](float const percentage) constexpr -> SDL_FPoint {
            float const t{linearInterpolation<float>(percentage, 0.0, 2.0 * pi)};
            return {
                /* x */ canvasBufferWidth * (std::sin(3.0f * t) + 1.0f) / 2.0f,
                /* y */ canvasBufferHeight * (std::sin(2.0f * t) + 1.0f) / 2.0f
            };
        },
        +outlineCanvas,
        +[](float const percentage) constexpr -> SDL_FPoint {
            return outlineCanvas(wrapValue(percentage + .50, 1.0));
        },
    };

    static std::array<SDL_FPoint, sourceFunctionList.size()> sourcePointList{};
    std::transform(
        sourceFunctionList.begin(), sourceFunctionList.end(),
        sourcePointList.begin(),
        [](decltype(sourceFunctionList)::value_type const sourceFunction) constexpr -> SDL_FPoint {
            return sourceFunction(sourceFunctionPercentage);
        }
    );

    for (int y{0}; y < canvasBufferHeight; ++y) {
        for (int x{0}; x < canvasBufferWidth; ++x) {
            HslaColor hslaPixel(mainColor);

            enum struct PointType : std::uint_least8_t { sink, source, };
            auto const processPoint = [x, y, &hslaPixel](
                SDL_FPoint const &point,
                PointType const pointType
            ) -> void {
                double const distance{
                    std::sqrt(std::pow(static_cast<double>(x) - point.x, 2.0) + std::pow(static_cast<double>(y) - point.y, 2.0))
                };

                double const hueOffset{(hueUnit + hueSummand) * distance};

                switch (pointType) {
                    case PointType::source:
                        hslaPixel.setHue(hslaPixel.getHue() - hueOffset);
                        break;
                    case PointType::sink:
                        hslaPixel.setHue(hslaPixel.getHue() + hueOffset);
                        break;
                    default:
                        throw pointType;
                }
            };

            if (mouse.has_value() and fingerMap.size() == 0u) processPoint(*mouse, PointType::sink);

            for (auto const &[identifier, point] : fingerMap) processPoint(
                point,
                (point.number % 2u == 0u) ? PointType::sink : PointType::source
            );

            for (auto const &point : sourcePointList) processPoint(point, PointType::source);

            SDL_Color const rgbaPixel(hslaPixel.toRgbaColor());

            pixelArray[y/* row */ * pixelRowLength + x/* column */] = SDL_MapRGBA(
                pixelFormat, rgbaPixel.r, rgbaPixel.g, rgbaPixel.b, rgbaPixel.a
            );
        }
    }

    SDL_UnlockTexture(canvasBuffer);

    // Copy pixel data from the canvas buffer to the window.
    check(SDL_RenderCopy(renderer, canvasBuffer, nullptr/* use entire texture */, nullptr/* stretch texture to entire window */));

    SDL_RenderPresent(renderer);
}
