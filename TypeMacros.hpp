#ifndef __EMBEDI_TYPE_MACROS_HPP__
#define __EMBEDI_TYPE_MACROS_HPP__

#include <stdint.h>

namespace TypeMacros
{
    using i8  = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using u8    = uint8_t;
    using u16   = uint16_t;
    using u32   = uint32_t;
    using u64   = uint64_t;
    using usize = unsigned long long;

    using f32 = float;
    using f64 = double;

    using memptr = void*;
    using uintptr = uintptr_t;
};

constexpr TypeMacros::u32 Kb(double x) { return x * 1024; }
constexpr TypeMacros::u32 Mb(double x) { return 1024 * Kb(x); }

#define __max(x, y) (((x) > (y)) ? (x) : (y))
#define __min(x, y) (((x) < (y)) ? (x) : (y))
#define __abs(x) (((x) < 0) ? -(x) : (x))
#define string_equals(s1, s2) (strcmp(s1, s2) == 0)

#endif