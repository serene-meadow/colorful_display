#include "HslaColor.hpp"

#include <sstream>
#include <cassert>

std::string Project::toString(SDL_Color const &color) {
    std::stringstream buffer;
    buffer << "(R=" << +color.r << ", G=" << +color.g << ", B=" << +color.b << ", A=" << +color.a << ")";
    return buffer.str();
}

SDL_Color Project::makeRgbaColor(
    double const hue,
    double const saturation,
    double const luminance,
    double const alpha
) {
    assert(0 <= hue and hue < 360);
    assert(0 <= saturation and saturation <= 1);
    assert(0 <= luminance and luminance <= 1);
    assert(0 <= alpha and alpha <= 1);

    auto const chroma = (1.0 - std::fabs(2.0 * luminance - 1.0)) * saturation;

    auto const X = chroma * (1.0 - std::fabs(std::fmod(hue / 60.0, 2.0) - 1.0));

    struct Rgb { double r, g, b; } color{};

    /**/ if (  0 <= hue and hue <  60) color = Rgb{chroma,      X,    0.0}; 
    else if ( 60 <= hue and hue < 120) color = Rgb{     X, chroma,    0.0};
    else if (120 <= hue and hue < 180) color = Rgb{   0.0, chroma,      X};
    else if (180 <= hue and hue < 240) color = Rgb{   0.0,      X, chroma};
    else if (240 <= hue and hue < 300) color = Rgb{     X,    0.0, chroma};
    else if (300 <= hue and hue < 360) color = Rgb{chroma,    0.0,      X};

    auto const m = luminance - chroma / 2.0;

    return {
        static_cast<Uint8>((color.r + m) * 0xFF),
        static_cast<Uint8>((color.g + m) * 0xFF),
        static_cast<Uint8>((color.b + m) * 0xFF),
        static_cast<Uint8>((alpha      ) * 0xFF),
    };
}

double Project::HslaColor::getCyclicHue(
    double const hue,
    double const percentage,
    double const depth
) {
    assert(hue >= 0.0); assert(hue < 360.0);
    double const hueOffset{linearInterpolation(percentage, 0.0, 2.0 * depth)};
    if (hueOffset < depth)
        return hueWrap(hue + hueOffset);
    else
        return hueWrap((hue + depth) - (hueOffset - depth));
}

SDL_Color Project::HslaColor::toRgbaColor() const {
    return makeRgbaColor(this->hue, this->saturation, this->luminance, this->alpha);
}

std::string Project::HslaColor::toString() const {
    std::stringstream buffer;
    buffer << "(H=" << hue << ", S=" << saturation << ", L=" << luminance << ", A=" << alpha << ")";
    return buffer.str();
}
