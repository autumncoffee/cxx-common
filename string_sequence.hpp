#pragma once

#include <memory>
#include <deque>
#include <utility>
#include "tmpmem.hpp"

namespace NAC {
    class TBlob;

    class TBlobSequence : public TWithTmpMem {
    public:
        struct TItem {
            size_t Len;
            const char* Data;
        };

    public:
        void Concat(size_t len, const char* data) {
            Sequence.emplace_back(TItem{len, data});
        }

        void Concat(const std::shared_ptr<TBlob>& data);
        void Concat(const TBlob& data);
        void Concat(TBlob&& data);
        void Concat(const TBlobSequence& data);

        TBlobSequence() = default;
        TBlobSequence(const TBlobSequence&) = default;
        TBlobSequence(TBlobSequence&&) = default;

        template<typename... TArgs>
        static TBlobSequence Construct(TArgs&&... args) {
            TBlobSequence out;
            out.Concat(std::forward<TArgs>(args)...);

            return out;
        }

        template<typename T>
        const char* Read(
            size_t index,
            size_t offset = 0,
            T* len = nullptr,
            size_t* next = nullptr
        ) const {
            if (index >= Sequence.size())
                throw std::logic_error("Reading past the end of string sequence (1)");

            const TItem* node = &Sequence[index];

            while (offset >= node->Len) {
                if(++index >= Sequence.size()) {
                    throw std::logic_error("Reading past the end of string sequence (2)");

                } else {
                    offset -= node->Len;
                    node = &Sequence[index];
                }
            }

            if (len != nullptr) {
                *len = node->Len - offset;
            }

            if (next != nullptr) {
                *next = index;
            }

            return node->Data + offset;
        }

        std::deque<TItem>::const_iterator begin() const {
            return Sequence.begin();
        }

        std::deque<TItem>::const_iterator end() const {
            return Sequence.end();
        }

    private:
        std::deque<TItem> Sequence;
    };
}
