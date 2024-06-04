#ifndef Utility_hpp
#define Utility_hpp true

#include <cmath>
#include <cassert>
#include <type_traits>

namespace Project::Utility {

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<T>, T> abs(T const& x) noexcept {
        return x < T{} ? -x : x;
    }

    template <typename IntT>
    constexpr std::enable_if_t<std::is_integral_v<IntT>, IntT> wrapValue(IntT value, IntT const upperBound) {
        constexpr IntT zero{0};
        assert(upperBound > zero);
        value %= upperBound;
        if (value < zero) value += upperBound;
        assert(value >= zero);
        return value;
    }

    template <typename FloatT>
    std::enable_if_t<std::is_floating_point_v<FloatT>, FloatT> wrapValue(FloatT value, FloatT const upperBound) {
        static constexpr FloatT zero{0.0};

        assert(upperBound != 0);
        assert(zero < upperBound);

        value = std::fmod(value, upperBound);

        if (value < zero) value += upperBound;
        
        if (value >= upperBound) value = zero;

        return value;
    }

    template <typename FloatT>
    constexpr std::enable_if_t<std::is_floating_point_v<FloatT>, FloatT> linearInterpolation(
        FloatT const percentage,
        FloatT const start,
        FloatT const end
    ) {
        constexpr FloatT zeroPercent{0.0f};
        constexpr FloatT oneHundredPercent{1.0f};

        assert(percentage >= zeroPercent);
        assert(percentage <= oneHundredPercent);

        return start * (oneHundredPercent - percentage) + end * percentage;
    }
}

#endif
