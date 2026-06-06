#ifndef __EMBEDI_TYPE_MACROS_HPP__
#define __EMBEDI_TYPE_MACROS_HPP__

#include <stdint.h>

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8    = uint8_t;
using u16   = uint16_t;
using u32   = uint32_t;
using u64   = uint64_t;
using usize = unsigned long long;

using memptr = void*;

#define getter(ret_type, name, variable) ret_type name() { return this->variable; }
#define setter(inp_type, name, variable) void name(inp_type __other) { this->variable = __other; }

#define max(x, y) ((x > y) ? x : y)
#define min(x, y) ((x < y) ? x : y)

#endif