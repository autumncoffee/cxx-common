#pragma once

#include <string>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>

namespace NAC {
    class TBlob {
    public:
        char* Data() const {
            return Data_;
        }

        size_t Size() const {
            return Size_;
        }

        size_t Capacity() const {
            return StorageSize_;
        }

        explicit operator std::string() const {
            return std::string(Data(), Size());
        }

        TBlob& Reserve(const size_t size) {
            return Reserve(size, /* exact = */true);
        }

        TBlob& Shrink(const size_t size) {
            assert(size <= Size_);
            Size_ = size;

            return *this;
        }

        TBlob& Chop(const size_t offset) {
            assert(offset <= Size_);

            TBlob tmp;
            tmp.Reserve(Size_ - offset);
            tmp.Append(Size_ - offset, Data_ + offset);

            Wrap(tmp.Size(), tmp.Data(), /* own = */true);
            tmp.Reset();

            return *this;
        }

        TBlob& Append(const size_t size, const char* data) {
            if (size == 0) {
                return *this;
            }

            Reserve(size, /* exact = */false);

            memcpy(Data_ + Size_, data, size);

            Size_ += size;

            return *this;
        }

        TBlob& Append(const char* src) {
            return Append(strlen(src), src);
        }

        TBlob& Append(const std::string& src) {
            return Append(src.size(), src.data());
        }

        template<typename... TArgs>
        TBlob& operator<<(TArgs&&... args) {
            return Append(std::forward<TArgs&&>(args)...);
        }

        char operator[](const size_t index) const {
            return Data_[index];
        }

        explicit operator bool() const {
            return (bool)Data_;
        }

        void Reset() {
            Data_ = nullptr;
            Size_ = 0;
            StorageSize_ = 0;
            Own = true;
        }

        bool Owning() const {
            return Own;
        }

        void Wrap(
            const size_t size,
            char* data,
            bool own = false
        ) {
            if(Own && Data_) {
                free(Data_);
            }

            StorageSize_ = Size_ = size;
            Data_ = data;
            Own = own;
        }

        void Wrap(
            const size_t size,
            const char* data,
            bool own = false
        ) {
            Wrap(size, (char*)data, own);
        }

        TBlob() = default;
        TBlob(const TBlob&) = delete;

        TBlob(
            const size_t size,
            char* data,
            bool own = false
        ) {
            Wrap(size, data, own);
        }

        TBlob(
            const size_t size,
            const char* data,
            bool own = false
        ) {
            Wrap(size, data, own);
        }

        TBlob& operator=(const TBlob&) = delete;

        TBlob& operator=(TBlob&& right) {
            if (Own && Data_) {
                free(Data_);
            }

            MoveImpl(right);

            return *this;
        }

        TBlob(TBlob&& right) {
            MoveImpl(right);
        }

        ~TBlob() {
            if (Own && Data_) {
                free(Data_);
                Data_ = nullptr;
            }
        }

        int Cmp(const size_t size, const char* data) const {
            const int rv(memcmp(Data(), data, std::min(Size(), size)));

            if (rv == 0) {
                if (Size() == size) {
                    return 0;

                } else if (Size() < size) {
                    return -1;

                } else {
                    return 1;
                }

            } else {
                return rv;
            }
        }

        int Cmp(const TBlob& right) const {
            return Cmp(right.Size(), right.Data());
        }

        int Cmp(const std::string& right) const {
            return Cmp(right.size(), right.data());
        }

        template<typename T>
        bool operator<(const T& right) const {
            return Cmp(right) < 0;
        }

        template<typename T>
        bool operator==(const T& right) const {
            return Cmp(right) == 0;
        }

        template<typename T>
        bool operator>(const T& right) const {
            return Cmp(right) > 0;
        }

        bool StartsWith(const size_t size, const char* data) const {
            if (Size() < size) {
                return false;
            }

            return (memcmp(Data(), data, size) == 0);
        }

        bool StartsWith(const TBlob& right) const {
            return StartsWith(right.Size(), right.Data());
        }

        bool StartsWith(const std::string& right) const {
            return StartsWith(right.size(), right.data());
        }

        int PrefixCmp(const size_t size, const char* data) const {
            if (StartsWith(size, data)) {
                return 0;
            }

            return Cmp(size, data);
        }

        int PrefixCmp(const TBlob& right) const {
            return PrefixCmp(right.Size(), right.Data());
        }

        int PrefixCmp(const std::string& right) const {
            return PrefixCmp(right.size(), right.data());
        }

    private:
        TBlob& Reserve(const size_t size, const bool exact) {
            Own = true;

            if (Data_) {
                const size_t newStorageSize = (Size_ + size);

                if (newStorageSize > StorageSize_) {
                    StorageSize_ = (newStorageSize * (exact ? 1 : 2));
                    Data_ = (char*)realloc(Data_, StorageSize_);
                }

            } else {
                Data_ = (char*)malloc(size);
                StorageSize_ = size;
            }

            return *this;
        }

        void MoveImpl(TBlob& right) {
            Data_ = right.Data_;
            Size_ = right.Size_;
            StorageSize_ = right.StorageSize_;
            Own = right.Own;

            right.Data_ = nullptr;
        }

    private:
        char* Data_ = nullptr;
        size_t Size_ = 0;
        size_t StorageSize_ = 0;
        bool Own = true;
    };
}
