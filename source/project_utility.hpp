#ifndef project_utility_hpp
#define project_utility_hpp true

#include <cmath>
#include <cassert>
#include <type_traits>
#include <iostream>

namespace Project {
    // Base case.
    inline void print() { std::cout << std::flush; }

    // Recursive case.
    template <typename ArgT, typename... ArgListT>
    inline void print(ArgT const &arg, ArgListT const &... argList) { std::cout << arg; print(argList...); }

    // Base Case.
    inline void println() { std::cout << std::endl; /* print new line and flush */ }

    // Recursive Case.
    template <typename ArgT, typename... ArgListT>
    inline void println(ArgT const &arg, ArgListT const &... argList) { std::cout << arg; println(argList...); }

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<T>, T> absoluteValue(T const x) noexcept {
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
        return start * (FloatT{1.0} - percentage) + end * percentage;
    }
}

#endif
