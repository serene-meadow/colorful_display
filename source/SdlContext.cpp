#include "SdlContext.hpp"
#include "HslaColor.hpp"
#include <algorithm>
#include <optional>
#include <unordered_map>

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

    static float scrollValue{0.0f};

    static inline void decayScrollValue(double const percentage) {
        double const x{linearInterpolation(percentage, -40.0, 24.4)};
        double const offset{4.9 * std::exp(0.07 * x)};

        /**/ if (scrollValue > 0.0f)
            scrollValue = std::max<float>(0.0f, scrollValue - offset);
        else if (scrollValue < 0.0f)
            scrollValue = std::min<float>(0.0f, scrollValue + offset);
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

    static bool debugFlag1{false};

    static double decayTimerPercentage{0.0};

    static SDL_Event event;
    // Handle events.
    /*
        This is the switch statement of greatness.
    */
    while (SDL_PollEvent(&event)) switch (event.type) {
        case SDL_KEYDOWN: switch (event.key.keysym.sym) {
            case SDLK_COMMA:
                println("title: ", SDL_GetWindowTitle(window));

                println("cached window size: ", charJoin(windowWidth, windowHeight));

                int w, h; SDL_GetWindowSize(window, &w, &h);
                println("actual window size: ", charJoin(w, h));

                int windowX, windowY; SDL_GetWindowPosition(window, &windowX, &windowY);
                println("window position: ", charJoin(windowX, windowY));

                float windowOpacity; check(SDL_GetWindowOpacity(window, &windowOpacity));
                println("window opacity: ", windowOpacity);

                println("Finger count: ", fingerMap.size());

                println();

                break;
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
                scrollValue = 0.0f;
                break;
            case SDL_BUTTON_RIGHT:
                debugFlag1 = true;
                break;
        }; break;
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
            case SDL_BUTTON_MIDDLE: break;
            case SDL_BUTTON_RIGHT:
                debugFlag1 = false;
                break;
        } break;
        case SDL_MOUSEWHEEL:
            scrollValue += event.wheel.preciseX + event.wheel.preciseY;
            println("Mouse scroll: (", event.wheel.preciseX, ", ", event.wheel.preciseY, ") -> ", scrollValue);
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
        case SDL_MULTIGESTURE:
            if (std::fabs(event.mgesture.dDist/* pinch distance */) > 0.002f/* threshold */) {
                scrollValue += event.mgesture.dDist/* pinch distance */ * event.mgesture.numFingers;
            }
            break;
        case SDL_WINDOWEVENT: switch (event.window.event) {
            case SDL_WINDOWEVENT_SHOWN:
                println("Window has been shown");
                break;
            case SDL_WINDOWEVENT_HIDDEN:
                println("Window has been hidden.");
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                println("Window has been exposed and should be redrawn");
                break;
            case SDL_WINDOWEVENT_MOVED:
                println("Window has been moved to (", charJoin(event.window.data1, event.window.data2), ")");
                break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                println("Window has been resized: ", event.window.data1, ',', event.window.data2);
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
                break;
            case SDL_WINDOWEVENT_RESIZED/* Resize Request is External to the Program */:
                println("Something external to the program requested for the window to be resized.");
                break;
            case SDL_WINDOWEVENT_MINIMIZED:       println("Window has been minimized"); break;
            case SDL_WINDOWEVENT_MAXIMIZED:       println("Window has been maximized"); break;
            case SDL_WINDOWEVENT_RESTORED:        println("Window has been restored to normal size and position"); break;
            case SDL_WINDOWEVENT_ENTER:           println("Window has gained mouse focus"); break;
            case SDL_WINDOWEVENT_LEAVE:           println("Window has lost mouse focus"); break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:    println("Window has gained keyboard focus"); break;
            case SDL_WINDOWEVENT_FOCUS_LOST:      println("Window has lost keyboard focus"); break;
            case SDL_WINDOWEVENT_CLOSE:           println("The window manager requests that the window be closed"); break;
            case SDL_WINDOWEVENT_TAKE_FOCUS:      println("Window is being offered a focus (should SetWindowInputFocus() on itself or a subwindow, or ignore)"); break;
            case SDL_WINDOWEVENT_HIT_TEST:        println("Window had a hit test that wasn't SDL_HITTEST_NORMAL."); break;
            case SDL_WINDOWEVENT_ICCPROF_CHANGED: println("The ICC profile of the window's display has changed."); break;
            case SDL_WINDOWEVENT_DISPLAY_CHANGED: println("Window has been moved to display data1."); break;
        } break;
        case SDL_QUIT:
            std::exit(EXIT_SUCCESS);
            break;
    }

    if (mouse.has_value() or not fingerMap.empty()) 
        decayTimerPercentage = 0.0;
    else decayScrollValue(
        decayTimerPercentage = std::clamp(decayTimerPercentage + static_cast<double>(deltaTime) * (0.00005), 0.0, 1.0)
    );

    if (debugFlag1) scrollValue += 60.0;

    println("Decay timer percentage: ", decayTimerPercentage, ", Scroll value: ", scrollValue);

    refreshWindow();

    // As this iteration ends, update the previous time.
    previousTime = currentTime;

    // Give the CPU a break.
    SDL_Delay(1u);
}

#include <array>

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
    int const pixelRowLength{pitch / SDL_BYTESPERPIXEL(pixelFormat->format)};
    #pragma GCC diagnostic pop

    Uint32 *const pixelArray = static_cast<Uint32 *>(pixelPointer);

    [[maybe_unused]]
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
            ) constexpr -> void {
                double const distance{
                    std::sqrt(std::pow(static_cast<double>(x) - point.x, 2.0) + std::pow(static_cast<double>(y) - point.y, 2.0))
                };

                double const hueOffset{(hueUnit + scrollValue) * distance};

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

            if (fingerMap.size() == 0u) for (auto const &point : sourcePointList) processPoint(point, PointType::source);

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
