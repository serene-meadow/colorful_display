#include "SdlContext.hpp"
#include "CartesianGrid2d.hpp"
#include "HslaColor.hpp"

namespace Project::SdlContext {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *canvasBuffer = nullptr;
    SDL_PixelFormat *pixelFormat = nullptr;

    static Uint64 deltaTime{0u};
    static int windowWidth{430}, windowHeight{430};
    static int mouseX{0}, mouseY{0};
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
            case SDLK_COMMA:
                println("title: ", SDL_GetWindowTitle(window));

                println("cached window size: ", charJoin(windowWidth, windowHeight));

                int w, h; SDL_GetWindowSize(window, &w, &h);
                println("actual window size: ", charJoin(w, h));

                int windowX, windowY; SDL_GetWindowPosition(window, &windowX, &windowY);
                println("window position: ", charJoin(windowX, windowY));

                float windowOpacity; check(SDL_GetWindowOpacity(window, &windowOpacity));
                println("window opacity: ", windowOpacity);

                break;
            case SDLK_BACKQUOTE:
                // "Proper" fullscreen may not be supported in all browsers.
                #ifdef __EMSCRIPTEN__
                check(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP /* "fake" fullscreen */));
                #else
                check(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN));
                #endif
                break;
            case SDLK_ESCAPE:
                check(SDL_SetWindowFullscreen(window, 0u));
                break;
        } break;
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
        // case SDL_STOP
        case SDL_MOUSEBUTTONDOWN: {
            switch (event.button.button) {
                case SDL_BUTTON_MIDDLE:
                    print("Mouse middle: ");
                    break;
                case SDL_BUTTON_RIGHT:
                    print("Mouse  right: ");
                    break;
                case SDL_BUTTON_LEFT:
                    print("Mouse   left: ");
                    break;
            }
            println(mouseX, ',', mouseY);
        }; break;
        case SDL_MOUSEMOTION: {
            SDL_GetMouseState(&mouseX, &mouseY);
            mouseX = linearInterpolation<float>(
                static_cast<float>(mouseX) / static_cast<float>(windowWidth), 0.0f, canvasBufferWidth
            );
            mouseY = linearInterpolation<float>(
                static_cast<float>(mouseY) / static_cast<float>(windowHeight), 0.0f, canvasBufferHeight
            );
        } break;
        case SDL_QUIT:
            std::exit(EXIT_SUCCESS);
            break;
    }

    refreshWindow();

    // As this iteration ends, update the previous time.
    previousTime = currentTime;

    // Give the CPU a break.
    SDL_Delay(1u);
}

#include <array>

void Project::SdlContext::refreshWindow() {
    static HslaColor mainColor;

    static double percentage{0.0};
    double const deltaPercentage{static_cast<double>(deltaTime) * 0.0008};
    percentage = wrapValue(percentage + deltaPercentage, 1.0);
    mainColor.setHue(linearInterpolation(percentage, 0.0, 360.0));

    int const minLength{std::min(canvasBufferWidth, canvasBufferHeight)};
    double const colorUnit = 2.0 * 360.0 / static_cast<double>(minLength);

    std::vector<SDL_FPoint> const sourcePointList = {
        // SDL_FPoint{windowWidth / 2.0f, windowHeight / 2.0f},
        SDL_FPoint{0.0f, 0.0f},
        // SDL_FPoint{windowWidth - 1.0f, 0.0f},
        // SDL_FPoint{0.0f, windowHeight - 1.0f},
        SDL_FPoint{static_cast<float>(mouseX), static_cast<float>(mouseY)},
        SDL_FPoint{canvasBufferWidth - 1.0f, canvasBufferHeight - 1.0f},
    };

    void *pixelPointer;
    int pitch;
    check(SDL_LockTexture(canvasBuffer, nullptr/* lock entire texture */, &pixelPointer, &pitch));

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnarrowing"
    int const pixelRowLength{pitch / SDL_BYTESPERPIXEL(pixelFormat->format)};
    #pragma GCC diagnostic pop

    Uint32 *const pixelArray = static_cast<Uint32 *>(pixelPointer);

    for (int y{0}; y < canvasBufferHeight; ++y) {
        for (int x{0}; x < canvasBufferWidth; ++x) {
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
