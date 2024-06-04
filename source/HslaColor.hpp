#ifndef HslaColor_hpp
#define HslaColor_hpp true

#include <string>
#include "SdlContext.hpp"

namespace Project::Media {
    inline constexpr SDL_Color black{0x00, 0x00, 0x00, 0xFF};

    std::string toString(SDL_Color const &color);

    SDL_Color makeRgbaColor(
        double const hue,
        double const saturation=1.0,
        double const luminance=0.5,
        double const alpha=1.0
    );

    struct HslaColor;
}

struct Project::Media::HslaColor {
    double hue, saturation, luminance, alpha;

    constexpr HslaColor(
        double const hueValue,
        double const saturationValue=1.0,
        double const luminanceValue=0.5,
        double const alphaValue=1.0
    ):
        hue{hueValue},
        saturation{saturationValue},
        luminance{luminanceValue},
        alpha{alphaValue}
    {}

    static double getCyclicHue(
        double const hue,
        double const percentage,
        double const depth
    );

    static double hueWrap(double const value);

    SDL_Color toRgbaColor() const;
    void addHue(double const hueSupplement);
    std::string toString() const;
};

#endif
