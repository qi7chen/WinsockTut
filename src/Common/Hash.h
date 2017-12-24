// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
// https://github.com/aappleby/smhasher

#pragma once

#include <stdint.h>
#include <string>
#include <type_traits>

// murmur hash v3
uint32_t MurmurHash3(const void* key, int len, uint32_t seed);

// hash value of POD
template <typename T>
uint32_t hash_value(const T& val)
{
    static_assert(std::is_pod<T>::value, "POD type only");
    return MurmurHash3(&val, sizeof(val), 0);
}

inline uint32_t hash_value(const std::string& val)
{
    return MurmurHash3(val.c_str(), val.size(), 0);
}
    