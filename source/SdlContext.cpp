#include <algorithm>
#include <optional>
#include <unordered_map>
#include <array>
#include "SdlContext.hpp"
#include "HslaColor.hpp"
#include <limits>

namespace Project::SdlContext {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *canvasBuffer = nullptr;
    SDL_PixelFormat *pixelFormat = nullptr;
    SDL_Cursor *cursorImage = nullptr;

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
    if (cursorImage != nullptr) SDL_FreeCursor(cursorImage);
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

    static bool mouseRightButtonIsPressed{false};

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
            hueSummand += 27.25f * event.mgesture.dDist/* pinch distance */ * static_cast<float>(event.mgesture.numFingers);
        } break;
        case SDL_WINDOWEVENT: switch (event.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
                break;
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

    refreshWindow();

    // As this iteration ends, update the previous time.
    previousTime = currentTime;

    // Give the CPU a break?
    SDL_Delay(1u);
}

namespace Project::SdlContext {
    template <int frequency=1>
    static constexpr float sine(float const t) { return std::sin(static_cast<float>(frequency) * t); }

    template <int frequency=1>
    static constexpr float cosine(float const t) { return std::cos(static_cast<float>(frequency) * t); }

    template <float (*...functions)(float const t)>
    static constexpr float productOfFunctions(float const t) {
        return (... * functions(t));
    }

    template <
        float (*xFunction)(float const t),
        float (*yFunction)(float const t)
    >
    static constexpr SDL_FPoint parametricWithPeriodOfTwoPi(float const percentage) {
        float const t{linearInterpolation<float>(percentage, 0.0f, 2.0f * pi)};
        return {
            /* x */ canvasBufferWidth  * (xFunction(t) / 2.0f + 0.5f),
            /* y */ canvasBufferHeight * (yFunction(t) / 2.0f + 0.5f) 
        };
    }

    /**
     * @brief Refresh the title of the window.
     * 
     * @note This function is not thread-safe.
     * 
     * @note This algorithm could possibly be improved by
     * only copying the last character from the color strings
     * to the buffer, because those are the only part of the strings
     * that differ between the colors; they all start with "\\xF0\\x9F\\x9F".
     * 
     * @param percentage position in animation time
     */
    static inline void refreshTitle(double const percentage) {
        static constexpr std::uint_fast8_t colorStringLength{4u};
        static constexpr std::uint_fast8_t colorSize{colorStringLength + 1u/* null terminator */};

        static constexpr char
            red    [colorSize] = "\xF0\x9F\x9F\xA5",
            orange [colorSize] = "\xF0\x9F\x9F\xA7",
            yellow [colorSize] = "\xF0\x9F\x9F\xA8",
            green  [colorSize] = "\xF0\x9F\x9F\xA9",
            blue   [colorSize] = "\xF0\x9F\x9F\xA6",
            purple [colorSize] = "\xF0\x9F\x9F\xAA"
        ;
        static constexpr std::array colorList{red, orange, yellow, green, blue, purple};

        static_assert(colorList.size() <= std::numeric_limits<std::uint_fast8_t>::max());
        static constexpr std::uint_fast8_t colorListSize{colorList.size()};

        static std::uint_fast8_t cachedColorOffset{std::numeric_limits<std::uint_fast8_t>::max()};

        std::uint_fast8_t const colorOffset{
            static_cast<std::uint_fast8_t>(
                std::round(
                    linearInterpolation(
                        percentage,
                        /* start */ static_cast<double>(colorListSize),
                        /* end */ 0.0
                    )
                )
            )
        };

        if (colorOffset == cachedColorOffset) return /* Don't set the title if it would change nothing. */;

        cachedColorOffset = colorOffset;

        static constexpr std::uint_fast8_t bufferColorCount{colorListSize + 4u};
        static constexpr std::size_t bufferSize{colorStringLength * bufferColorCount/* color count */ + 1uL/* null terminator */};
        static char buffer[bufferSize] = {}/* (null terminated) */;

        for (std::uint_fast8_t colorIndex{0u}; colorIndex < bufferColorCount; ++colorIndex) {
            auto const color = colorList[(colorIndex + colorOffset) % colorListSize];

            static_assert(bufferSize <= std::numeric_limits<std::uint_fast8_t>::max());
            std::copy(
                color,
                color + colorStringLength,
                buffer + (colorIndex * colorStringLength)
            );
        }

        SDL_SetWindowTitle(window, buffer);
    }
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

        parametricWithPeriodOfTwoPi</* x */ sine<3>, /* y */ sine<2>>,

        parametricWithPeriodOfTwoPi<
            /* x */ productOfFunctions<sine<3>, cosine<5>>,
            /* y */ cosine<3>
        >,

        +outlineCanvas,

        +[](float const percentage) constexpr -> SDL_FPoint {
            return outlineCanvas(wrapValue(percentage + .50f, 1.0f));
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
                // (point.number % 2u == 0u) ? PointType::sink : PointType::source
                PointType::sink
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

    refreshTitle(huePercentage);

    SDL_RenderPresent(renderer);
}
