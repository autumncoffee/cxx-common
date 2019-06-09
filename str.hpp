#pragma once

#include <string>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

namespace NAC {
    class TBlob {
    public:
        char* Data() const {
            return Data_;
        }

        size_t Size() const {
            return Size_;
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
