#pragma once

#include <sys/types.h>
#include <stdlib.h>
#include <string>
#include <string.h>

namespace NAC {
    namespace NStringUtils {
        static inline void Strip(
            const size_t size,
            const char* data,
            size_t& offset,
            char*& out,
            bool alloc = true,
            const size_t alphabetSize = 2,
            const char* alphabet = " \r"
        ) {
            char mask[256];
            memset(mask, 0, 256);
            size_t i = 0;

            for(; i < alphabetSize; ++i) {
                mask[(unsigned char)alphabet[i]] = 1;
            }

            offset = 0;
            bool strip = true;

            if(alloc)
                out = (char*)malloc(size);

            for(i = 0; i < size; ++i) {
                if(strip && mask[(unsigned char)data[i]])
                    continue;

                out[offset++] = data[i];
                strip = false;
            }

            ssize_t lastIdx = -1;

            for(ssize_t i = (size - 1); i >= 0; --i) {
                lastIdx = i;

                if(mask[(unsigned char)data[i]])
                    continue;

                break;
            }

            offset -= (size - (lastIdx + 1));
        }

        static inline void Strip(
            const size_t size,
            const char* data,
            std::string& out,
            const size_t alphabetSize = 2,
            const char* alphabet = " \r"
        ) {
            out.clear();
            out.resize(size);

            size_t newSize;
            char* dst = (char*)out.data();

            Strip(
                size,
                data,
                newSize,
                dst,
                /* alloc = */false,
                alphabetSize,
                alphabet
            );

            out.resize(newSize);
        }
    }
}