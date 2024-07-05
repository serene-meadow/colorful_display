#ifndef HslaColor_hpp
#define HslaColor_hpp true

#include <string>
#include "SdlContext.hpp"
#include "project_utility.hpp"

namespace Project {
  inline constexpr SDL_Color black{0x00, 0x00, 0x00, 0xFF};

  extern std::string toString(SDL_Color const &color);

  extern SDL_Color makeRgbaColor(
    double const hue,
    double const saturation=1.0,
    double const luminance=0.5,
    double const alpha=1.0
  );

  class HslaColor;
}

class Project::HslaColor {
  private:
    float hue, saturation, luminance, alpha;

  public:
    constexpr HslaColor(
      float const hueValue=0.0f,
      float const saturationValue=1.0f,
      float const luminanceValue=0.5f,
      float const alphaValue=1.0f
    ):
      hue{hueValue},
      saturation{saturationValue},
      luminance{luminanceValue},
      alpha{alphaValue}
    {}

    constexpr float getHue() const { return hue; }
    constexpr float getSaturation() const { return saturation; }
    constexpr float getLuminance() const { return luminance; }
    constexpr float getAlpha() const { return alpha; }

    float setHue(float const hueValue) { return hue = wrapValue(hueValue, 360.0f); }
    float setSaturation(float const saturationValue) { return saturation = wrapValue(saturationValue, 1.0f); }
    float setLuminance(float const luminanceValue) { return luminance = wrapValue(luminanceValue, 1.0f); }
    float setAlpha(float const alphaValue) { return alpha = wrapValue(alphaValue, 1.0f); }

    static double getCyclicHue(
      double const hue,
      double const percentage,
      double const depth
    );

    static inline double hueWrap(double const value) {
      static constexpr double fullCycle{360.0};
      return wrapValue(value, fullCycle);
    }

    SDL_Color toRgbaColor() const;
    std::string toString() const;
};

#endif
