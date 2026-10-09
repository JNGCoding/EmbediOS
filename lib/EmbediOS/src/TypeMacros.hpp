#ifndef __EMBEDI_TYPE_MACROS_HPP__
#define __EMBEDI_TYPE_MACROS_HPP__

#include <stdint.h>
#include <string.h>

#ifndef __AVR__
#include <assert.h>
#else
#ifndef assert
#define assert(expr) ((expr) ? (void)0 : abort())
#endif
#endif

#ifndef __AVR__
#include <stddef.h>
constexpr uint32_t MAX_ALIGNMENT = alignof(max_align_t);
#else
constexpr uint32_t MAX_ALIGNMENT = (alignof(long long) > alignof(double)) ? alignof(long long) : alignof(double);
#endif

namespace TypeMacros
{
    using sint = int;                   // signed integer
    using uint = unsigned int;          // unsigned integer

    // Signed integer versions
    using i8  = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    // Unsigned integer versions
    using u8    = uint8_t;
    using u16   = uint16_t;
    using u32   = uint32_t;
    using u64   = uint64_t;
    using usize = unsigned long long;

    // float versions
    using f32 = float;
    using f64 = double;

    // pointer types
    using memptr = void*;
    using uintptr = uintptr_t;
    using cstr = char*;
    template<typename Type> using buffer = Type*;

    // helper types
    using emtime = usize;
};

// Creating my own same type checker at compile time
template<typename U, typename V>
struct __embedi_is_same_type { static constexpr bool value = false; };
template<typename U>
struct __embedi_is_same_type<U, U> { static constexpr bool value = true; };
template<typename U, typename V>
inline constexpr bool is_same_type_v = __embedi_is_same_type<U, V>::value;

constexpr TypeMacros::u32 Kb(double x) { return 1024 * x; }
constexpr TypeMacros::u32 Mb(double x) { return 1024 * Kb(x); }
constexpr TypeMacros::u64 Gb(double x) { return 1024 * Mb(x); }

constexpr TypeMacros::u32 Kbps(double x) { return 1000 * x; }
constexpr TypeMacros::u32 Mbps(double x) { return 1000 * Kbps(x); }
constexpr TypeMacros::u32 Gbps(double x) { return 1000 * Mbps(x); }

inline bool string_equals(const char* s1, const char* s2) { return strcmp(s1, s2) == 0; }

constexpr bool is_space(const char ch) { return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\r' || ch == '\f' || ch == '\n' || ch == '\0'; }
constexpr bool is_digit(const char ch) { return (ch >= '0' && ch <= '9'); }

#ifndef embedi_max
#define embedi_max(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef embedi_min
#define embedi_min(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef embedi_abs
#define embedi_abs(x) (((x) < 0) ? -(x) : (x))
#endif

#ifndef embedi_constrain
#define embedi_constrain(x, low, high) constrain(x, low, high)  // ((x) > (high) ? (high) : ((x) < (low) ? (low) : (x)))
#endif

#endif