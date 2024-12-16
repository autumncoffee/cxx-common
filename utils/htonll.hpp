#pragma once

#include <arpa/inet.h>
#include <cstdint>

#ifndef htonll
#define htonll(x) ((1 == htonl(1)) ? (x) : ((uint64_t)htonl(\
    (x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#endif

#ifndef ntohll
#define ntohll(x) ((1 == ntohl(1)) ? (x) : ((uint64_t)ntohl(\
    (x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

namespace NAC {
    template<typename T>
    T hton(T val) {
        static_assert(
            (sizeof(T) == sizeof(uint64_t))
            || (sizeof(T) == sizeof(uint32_t))
            || (sizeof(T) == sizeof(uint16_t))
        );

        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return htonll(val);

        } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
            return htonl(val);

        } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
            return htons(val);
        }
    }

    template<typename T>
    T ntoh(T val) {
        static_assert(
            (sizeof(T) == sizeof(uint64_t))
            || (sizeof(T) == sizeof(uint32_t))
            || (sizeof(T) == sizeof(uint16_t))
        );

        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return ntohll(val);

        } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
            return ntohl(val);

        } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
            return ntohs(val);
        }
    }
}
