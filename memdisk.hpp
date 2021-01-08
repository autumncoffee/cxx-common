#pragma once

#include "str.hpp"
#include "file.hpp"
#include <memory>
#include <stdexcept>

namespace NAC {
    class TMemDisk {
    public:
        TMemDisk() = delete;
        TMemDisk(const TMemDisk&) = delete;
        TMemDisk(TMemDisk&&) = default;

        TMemDisk(size_t memMax, const std::string& diskMask);

        TMemDisk& operator=(const TMemDisk&) = delete;
        TMemDisk& operator=(TMemDisk&&) = default;

        void Finish();

        TMemDisk& Append(const size_t size, const char* data);

        TMemDisk& Append(const char* src) {
            return Append(strlen(src), src);
        }

        TMemDisk& Append(const std::string& src) {
            return Append(src.size(), src.data());
        }

        template<typename... TArgs>
        TMemDisk& operator<<(TArgs&&... args) {
            return Append(std::forward<TArgs&&>(args)...);
        }

        template<typename... TArgs>
        void Wrap(TArgs&&... args) {
            Mem.Wrap(std::forward<TArgs&&>(args)...);
        }

        char* Data() const {
            return Mem.Data();
        }

        size_t Size() const {
            return Mem.Size();
        }

        char operator[](const size_t index) const {
            if (index >= Mem.Size()) {
                throw std::runtime_error("Out of bounds");
            }

            return Mem[index];
        }

        explicit operator bool() const {
            return (Disk ? (bool)*Disk : (bool)Mem);
        }

    private:
        size_t MemMax = 0;
        TBlob Mem;
        std::string DiskMask;
        std::unique_ptr<TFile> Disk;
    };
}
