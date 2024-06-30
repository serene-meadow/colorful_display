#ifndef project_utility_hpp
#define project_utility_hpp true

#include <cmath>
#include <cassert>
#include <type_traits>
#include <iostream>
#include <sstream>

namespace Project /* String */ {

    template <typename... Params>
    inline void print(Params &&... params) {
        (std::cout << ... << params) << std::flush;
    }

    template <typename... Params>
    inline void println(Params &&... params) {
        (std::cout << ... << params) << std::endl/* print new line and flush */;
    }

    template<typename T>
    [[nodiscard]] inline constexpr std::enable_if_t<std::is_arithmetic_v<T>, T> absoluteValue(T const x) noexcept {
        return x < T{0} ? -x : x;
    }

    template <typename DelimeterT>
    inline std::ostringstream &joinToStream(std::ostringstream &stream, DelimeterT const &) { return stream; }

    template<typename DelimeterT, typename T, typename... Params>
    inline std::ostringstream &joinToStream(std::ostringstream &stream, DelimeterT const &delimeter, T const &arg, Params const &... params) {
        stream << arg;
        if constexpr (sizeof...(params) > 0u) stream << delimeter;
        return joinToStream(stream, delimeter, params...);
    }

    template<typename DelimeterT, typename... Params>
    inline std::string stringJoin(DelimeterT const &delimeter, Params const &... params) {
        std::ostringstream stream;
        stream << std::boolalpha;
        joinToStream(stream, delimeter, params...);
        return stream.str();
    }

    template<char delimeter=',', typename... Params>
    inline std::string charJoin(Params const &... params) { return stringJoin(delimeter, params...); }

}

namespace Project /* Math */ {

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
    inline std::enable_if_t<std::is_floating_point_v<FloatT>, FloatT> wrapValue(FloatT value, FloatT const upperBound) {
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

    inline constexpr double pi{3.141592653589793115997963468544185161590576171875};
}

#endif
