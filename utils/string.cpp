#include "string.hpp"
#include <string.h>
#include <ctype.h>
#include <algorithm>

namespace NAC {
    namespace NStringUtils {
        void Strip(
            const size_t size,
            const char* data,
            size_t& offset,
            char*& out,
            bool alloc,
            const size_t alphabetSize,
            const char* alphabet
        ) {
            char mask[256];
            memset(mask, 0, 256);
            size_t i = 0;

            for (; i < alphabetSize; ++i) {
                mask[(unsigned char)alphabet[i]] = 1;
            }

            offset = 0;
            bool strip = true;

            if (alloc) {
                out = (char*)malloc(size);
            }

            for (i = 0; i < size; ++i) {
                if(strip && mask[(unsigned char)data[i]])
                    continue;

                out[offset++] = data[i];
                strip = false;
            }

            ssize_t lastIdx = -1;

            for (ssize_t i = (size - 1); i >= 0; --i) {
                lastIdx = i;

                if (mask[(unsigned char)data[i]])
                    continue;

                break;
            }

            offset -= (size - (lastIdx + 1));
        }

        void Strip(
            const size_t size,
            const char* data,
            std::string& out,
            const size_t alphabetSize,
            const char* alphabet
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

        std::string&& ToLower(std::string&& str) {
            std::transform(str.begin(), str.end(), str.begin(), tolower);
            return std::move(str);
        }

        void ToLower(std::string& str) {
            std::transform(str.begin(), str.end(), str.begin(), tolower);
        }

        std::string ToLower(const std::string& str) {
            std::string out(str);
            ToLower(out);
            return out;
        }

        TBlob NextTok(size_t size, const char* data, const char delim) {
            for (size_t i = 0; i < size; ++i) {
                if (data[i] == delim) {
                    return TBlob(i, data);
                }
            }

            return TBlob(size, data);
        }

        std::vector<TBlob> Split(size_t size, const char* data, const char delim) {
            std::vector<TBlob> out;

            while (size > 0) {
                auto&& part = NextTok(size, data, delim);

                if (part.Size() < size) {
                    size -= 1;
                    data += 1;
                }

                size -= part.Size();
                data += part.Size();

                out.emplace_back(std::move(part));
            }

            return out;
        }
    }
}
