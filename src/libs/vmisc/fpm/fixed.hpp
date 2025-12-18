#ifndef FPM_FIXED_HPP
#define FPM_FIXED_HPP

#include <QtGlobal>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>

namespace fpm
{

//! Fixed-point number type
//! \tparam BaseType         the base integer type used to store the fixed-point number. This can be a signed or
//! unsigned type.
//! \tparam IntermediateType the integer type used to store intermediate results during calculations.
//! \tparam FractionBits     the number of bits of the BaseType used to store the fraction
template <typename BaseType, typename IntermediateType, unsigned int FractionBits> class fixed
{
    static_assert(std::is_integral_v<BaseType>, "BaseType must be an integral type");
    static_assert(FractionBits > 0, "FractionBits must be greater than zero");
    static_assert(FractionBits <= sizeof(BaseType) * 8 - 1, "BaseType must at least be able to contain entire "
                                                            "fraction, with space for at least one integral bit");
    static_assert(sizeof(IntermediateType) > sizeof(BaseType), "IntermediateType must be larger than BaseType");
    static_assert(std::is_signed_v<IntermediateType> == std::is_signed_v<BaseType>,
                  "IntermediateType must have same signedness as BaseType");

    // Although this value fits in the BaseType in terms of bits, if there's only one integral bit, this value
    // is incorrect (flips from positive to negative), so we must extend the size to IntermediateType.
    static constexpr IntermediateType FRACTION_MULT = IntermediateType(1) << FractionBits;

    struct raw_construct_tag
    {
    };
    constexpr fixed(BaseType val, raw_construct_tag /*unused*/) noexcept
      : m_value(val)
    {
    }

public:
    inline fixed() noexcept = default;

    // Converts an integral number to the fixed-point type.
    // Like static_cast, this truncates bits that don't fit.
    template <typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
    constexpr explicit fixed(T val) noexcept
      : m_value(static_cast<BaseType>(val * FRACTION_MULT))
    {
    }

    // Converts an floating-point number to the fixed-point type.
    // Like static_cast, this truncates bits that don't fit.
    template <typename T, std::enable_if_t<std::is_floating_point_v<T>> * = nullptr>
    constexpr explicit fixed(T val) noexcept
      : m_value(static_cast<BaseType>((val >= 0.0) ? (val * FRACTION_MULT + T{0.5}) : (val * FRACTION_MULT - T{0.5})))
    {
    }

    // Constructs from another fixed-point type with possibly different underlying representation.
    // Like static_cast, this truncates bits that don't fit.
    template <typename B, typename I, unsigned int F>
    constexpr explicit fixed(fixed<B, I, F> val) noexcept
      : m_value(from_fixed_point<F>(val.raw_value()).raw_value())
    {
    }

    // Explicit conversion to a floating-point type
    template <typename T, std::enable_if_t<std::is_floating_point_v<T>> * = nullptr>
    constexpr explicit operator T() const noexcept
    {
        return static_cast<T>(m_value) / FRACTION_MULT;
    }

    // Explicit conversion to an integral type
    template <typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
    constexpr explicit operator T() const noexcept
    {
        return static_cast<T>(m_value / FRACTION_MULT);
    }

    // Returns the raw underlying value of this type.
    // Do not use this unless you know what you're doing.
    constexpr auto raw_value() const noexcept -> BaseType { return m_value; }

    //! Constructs a fixed-point number from another fixed-point number.
    //! \tparam NumFractionBits the number of bits used by the fraction in \a value.
    //! \param value the integer fixed-point number
    template <unsigned int NumFractionBits, typename T, std::enable_if_t<(NumFractionBits > FractionBits)> * = nullptr>
    static constexpr auto from_fixed_point(T value) noexcept -> fixed
    {
        // To correctly round the last bit in the result, we need one more bit of information.
        // We do this by multiplying by two before dividing and adding the LSB to the real result.
        return fixed(static_cast<BaseType>(value / (T(1) << (NumFractionBits - FractionBits)) +
                                           (value / (T(1) << (NumFractionBits - FractionBits - 1)) % 2)),
                     raw_construct_tag{});
    }

    template <unsigned int NumFractionBits, typename T, std::enable_if_t<(NumFractionBits <= FractionBits)> * = nullptr>
    static constexpr auto from_fixed_point(T value) noexcept -> fixed
    {
        return fixed(static_cast<BaseType>(value * (T(1) << (FractionBits - NumFractionBits))), raw_construct_tag{});
    }

    // Constructs a fixed-point number from its raw underlying value.
    // Do not use this unless you know what you're doing.
    static constexpr auto from_raw_value(BaseType value) noexcept -> fixed { return fixed(value, raw_construct_tag{}); }

    //
    // Constants
    //
    static constexpr auto e() -> fixed { return from_fixed_point<61>(6267931151224907085LL); }
    static constexpr auto pi() -> fixed { return from_fixed_point<61>(7244019458077122842LL); }
    static constexpr auto half_pi() -> fixed { return from_fixed_point<62>(7244019458077122842LL); }
    static constexpr auto two_pi() -> fixed { return from_fixed_point<60>(7244019458077122842LL); }

    //
    // Arithmetic member operators
    //

    constexpr auto operator-() const noexcept -> fixed { return fixed::from_raw_value(-m_value); }

    inline auto operator+=(const fixed &y) noexcept -> fixed &
    {
        m_value += y.m_value;
        return *this;
    }

    template <typename I, std::enable_if_t<std::is_integral_v<I>> * = nullptr>
    inline auto operator+=(I y) noexcept -> fixed &
    {
        m_value += y * FRACTION_MULT;
        return *this;
    }

    inline auto operator-=(const fixed &y) noexcept -> fixed &
    {
        m_value -= y.m_value;
        return *this;
    }

    template <typename I, std::enable_if_t<std::is_integral_v<I>> * = nullptr>
    inline auto operator-=(I y) noexcept -> fixed &
    {
        m_value -= y * FRACTION_MULT;
        return *this;
    }

    inline auto operator*=(const fixed &y) noexcept -> fixed &
    {
        // Normal fixed-point multiplication is: x * y / 2**FractionBits.
        // To correctly round the last bit in the result, we need one more bit of information.
        // We do this by multiplying by two before dividing and adding the LSB to the real result.
        auto value = static_cast<IntermediateType>(
            static_cast<double>(static_cast<IntermediateType>(m_value) * y.m_value) / (FRACTION_MULT / 2));
        m_value = static_cast<BaseType>((value / 2) + (value % 2));
        return *this;
    }

    template <typename I, std::enable_if_t<std::is_integral_v<I>> * = nullptr>
    inline auto operator*=(I y) noexcept -> fixed &
    {
        m_value *= y;
        return *this;
    }

    inline auto operator/=(const fixed &y) noexcept -> fixed &
    {
        assert(y.m_value != 0);
        // Normal fixed-point division is: x * 2**FractionBits / y.
        // To correctly round the last bit in the result, we need one more bit of information.
        // We do this by multiplying by two before dividing and adding the LSB to the real result.
        auto value = (static_cast<IntermediateType>(m_value) * FRACTION_MULT * 2) / y.m_value;
        m_value = static_cast<BaseType>((value / 2) + (value % 2));
        return *this;
    }

    template <typename I, std::enable_if_t<std::is_integral_v<I>> * = nullptr>
    inline auto operator/=(I y) noexcept -> fixed &
    {
        m_value /= y;
        return *this;
    }

    //
    // Shift operators
    //

    template <typename I, std::enable_if_t<std::is_integral_v<I>> * = nullptr>
    inline auto operator>>=(I y) noexcept -> fixed &
    {
        m_value >>= y;
        return *this;
    }

    template <typename I, std::enable_if_t<std::is_integral_v<I>> * = nullptr>
    inline auto operator<<=(I y) noexcept -> fixed &
    {
        m_value <<= y;
        return *this;
    }

private:
    BaseType m_value;
};

//
// Convenience typedefs
//

using fixed_16_16 = fixed<std::int32_t, std::int64_t, 16>;
using fixed_24_8 = fixed<std::int32_t, std::int64_t, 8>;
using fixed_8_24 = fixed<std::int32_t, std::int64_t, 24>;

//
// Addition
//

template <typename B, typename I, unsigned int F>
constexpr auto operator+(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) += y;
}

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator+(const fixed<B, I, F> &x, T y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) += y;
}

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator+(T x, const fixed<B, I, F> &y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(y) += x;
}

//
// Subtraction
//

template <typename B, typename I, unsigned int F>
constexpr auto operator-(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) -= y;
}

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator-(const fixed<B, I, F> &x, T y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) -= y;
}

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator-(T x, const fixed<B, I, F> &y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) -= y;
}

//
// Multiplication
//

template <typename B, typename I, unsigned int F>
constexpr auto operator*(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) *= y;
}

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator*(const fixed<B, I, F> &x, T y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) *= y;
}

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator*(T x, const fixed<B, I, F> &y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(y) *= x;
}

//
// Division
//

template <typename B, typename I, unsigned int F>
constexpr auto operator/(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) /= y;
}

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator/(const fixed<B, I, F> &x, T y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) /= y;
}

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator/(T x, const fixed<B, I, F> &y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) /= y;
}

//
// Shift operators
//

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator>>(const fixed<B, I, F> &x, T y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) >>= y;
}

template <typename B, typename I, unsigned int F, typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
constexpr auto operator<<(const fixed<B, I, F> &x, T y) noexcept -> fixed<B, I, F>
{
    return fixed<B, I, F>(x) <<= y;
}

//
// Comparison operators
//

template <typename B, typename I, unsigned int F>
constexpr auto operator==(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> bool
{
    return x.raw_value() == y.raw_value();
}

template <typename B, typename I, unsigned int F>
constexpr auto operator!=(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> bool
{
    return x.raw_value() != y.raw_value();
}

template <typename B, typename I, unsigned int F>
constexpr auto operator<(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> bool
{
    return x.raw_value() < y.raw_value();
}

template <typename B, typename I, unsigned int F>
constexpr auto operator>(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> bool
{
    return x.raw_value() > y.raw_value();
}

template <typename B, typename I, unsigned int F>
constexpr auto operator<=(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> bool
{
    return x.raw_value() <= y.raw_value();
}

template <typename B, typename I, unsigned int F>
constexpr auto operator>=(const fixed<B, I, F> &x, const fixed<B, I, F> &y) noexcept -> bool
{
    return x.raw_value() >= y.raw_value();
}

namespace detail
{

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunneeded-internal-declaration")

// Number of base-10 digits required to fully represent a number of bits
static constexpr auto max_digits10(int bits) -> int
{
    // 8.24 fixed-point equivalent of (int)ceil(bits * std::log10(2));
    using T = long long;                                                   // NOLINT(google-runtime-int)
    return static_cast<int>((T{bits} * 5050445 + (T{1} << 24) - 1) >> 24); // NOLINT(hicpp-signed-bitwise)
}

// Number of base-10 digits that can be fully represented by a number of bits
static constexpr auto digits10(int bits) -> int
{
    // 8.24 fixed-point equivalent of (int)(bits * std::log10(2));
    using T = long long;                                // NOLINT(google-runtime-int)
    return static_cast<int>((T{bits} * 5050445) >> 24); // NOLINT(hicpp-signed-bitwise)
}

QT_WARNING_POP

} // namespace detail
} // namespace fpm

// Specializations for customization points
namespace std // NOLINT(cert-dcl58-cpp)
{

template <typename B, typename I, unsigned int F> struct hash<fpm::fixed<B, I, F>>
{
    using argument_type = fpm::fixed<B, I, F>;
    using result_type = std::size_t;

    auto operator()(const argument_type &arg) const noexcept(noexcept(std::declval<std::hash<B>>()(arg.raw_value())))
        -> result_type
    {
        return m_hash(arg.raw_value());
    }

private:
    std::hash<B> m_hash;
};

template <typename B, typename I, unsigned int F> struct numeric_limits<fpm::fixed<B, I, F>>
{
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = std::numeric_limits<B>::is_signed;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr std::float_round_style round_style = std::round_to_nearest;
    static constexpr bool is_iec_559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = std::numeric_limits<B>::is_modulo;
    static constexpr int digits = std::numeric_limits<B>::digits;

    // Any number with `digits10` significant base-10 digits (that fits in
    // the range of the type) is guaranteed to be convertible from text and
    // back without change. Worst case, this is 0.000...001, so we can only
    // guarantee this case. Nothing more.
    static constexpr int digits10 = 1;

    // This is equal to max_digits10 for the integer and fractional part together.
    static constexpr int max_digits10 =
        fpm::detail::max_digits10(std::numeric_limits<B>::digits - F) + fpm::detail::max_digits10(F);

    static constexpr int radix = 2;
    static constexpr int min_exponent = 1 - F;
    static constexpr int min_exponent10 = -fpm::detail::digits10(F);
    static constexpr int max_exponent = std::numeric_limits<B>::digits - F;
    static constexpr int max_exponent10 = fpm::detail::digits10(std::numeric_limits<B>::digits - F);
    static constexpr bool traps = true;
    static constexpr bool tinyness_before = false;

    static constexpr auto lowest() noexcept -> fpm::fixed<B, I, F>
    {
        return fpm::fixed<B, I, F>::from_raw_value(std::numeric_limits<B>::lowest());
    }

    static constexpr auto min() noexcept -> fpm::fixed<B, I, F> { return lowest(); }

    static constexpr auto max() noexcept -> fpm::fixed<B, I, F>
    {
        return fpm::fixed<B, I, F>::from_raw_value(std::numeric_limits<B>::max());
    }

    static constexpr auto epsilon() noexcept -> fpm::fixed<B, I, F> { return fpm::fixed<B, I, F>::from_raw_value(1); }

    static constexpr auto round_error() noexcept -> fpm::fixed<B, I, F> { return fpm::fixed<B, I, F>(1) / 2; }

    static constexpr auto denorm_min() noexcept -> fpm::fixed<B, I, F> { return min(); }
};
} // namespace std

#endif
