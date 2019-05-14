#pragma once

#include <memory>
#include <deque>
#include <utility>

namespace NAC {
    class TTmpMem {
    public:
        template<typename T>
        void Memorize(std::shared_ptr<T> ptr) {
            Memory.emplace_back(std::shared_ptr<void>(ptr, (void*)ptr.get()));
        }

        void Memorize(const TTmpMem& right) {
            for (const auto& node : right) {
                Memory.emplace_back(node);
            }
        }

        template<typename T, typename TDeleter>
        void Memorize(T* ptr, TDeleter d) {
            Memory.emplace_back(std::shared_ptr<void>((void*)ptr, d));
        }

        template<typename T>
        void Free(T* ptr) {
            Memorize(ptr, [](void* ptr_){ free(ptr_); });
        }

        std::deque<std::shared_ptr<void>>::const_iterator begin() const {
            return Memory.begin();
        }

        std::deque<std::shared_ptr<void>>::const_iterator end() const {
            return Memory.end();
        }

    private:
        std::deque<std::shared_ptr<void>> Memory;
    };

    class TWithTmpMem {
    public:
        virtual ~TWithTmpMem() {
        }

        template<typename... TArgs>
        void Memorize(TArgs&&... args) {
            Memory.Memorize(std::forward<TArgs>(args)...);
        }

        template<typename... TArgs>
        void MemorizeFree(TArgs&&... args) {
            Memory.Free(std::forward<TArgs>(args)...);
        }

        void MemorizeCopy(const TWithTmpMem* right) {
            Memorize(right->Memory);
        }

    private:
        TTmpMem Memory;
    };
}
