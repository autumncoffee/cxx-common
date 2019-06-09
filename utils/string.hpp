#pragma once

#include <sys/types.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <ac-common/str.hpp>
#include <utility>

namespace NAC {
    namespace NStringUtils {
        void Strip(
            const size_t size,
            const char* data,
            size_t& offset,
            char*& out,
            bool alloc = true,
            const size_t alphabetSize = 2,
            const char* alphabet = " \r"
        );

        void Strip(
            const size_t size,
            const char* data,
            std::string& out,
            const size_t alphabetSize = 2,
            const char* alphabet = " \r"
        );

        template<typename T>
        void FromString(const size_t size, const char* data, T& out) {
            std::stringstream ss;
            ss.write(data, size);
            ss >> out;
        }

        template<typename T>
        void FromString(const std::string& in, T& out) {
            FromString(in.size(), in.data(), out);
        }

        template<typename T>
        void FromString(const TBlob& in, T& out) {
            FromString(in.Size(), in.Data(), out);
        }

        template<typename T, typename... TArgs>
        T FromString(TArgs&&... args) {
            T out;
            FromString(std::forward<TArgs>(args)..., out);
            return out;
        }

        std::string&& ToLower(std::string&&);
        void ToLower(std::string&);
        std::string ToLower(const std::string&);
    }
}
