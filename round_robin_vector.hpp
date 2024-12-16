#pragma once

#include <cstdint>
#include <stdlib.h>
#include <vector>
#include <atomic>
#include <stdexcept>
#include "spin_lock.hpp"

namespace NAC {
    namespace NUtils {
        template<typename Value>
        class TRoundRobinVector : public std::vector<Value> {
        private:
            NUtils::TSpinLock Mutex;
            uint64_t Pos;

        private:
            Value NextImpl() {
                if(std::vector<Value>::empty())
                    throw std::logic_error ("next() called on empty round-robin vector");

                NUtils::TSpinLockGuard guard(Mutex);

                if(Pos >= std::vector<Value>::size())
                    Pos = 0;

                const auto& value = std::vector<Value>::at(Pos);
                ++Pos;

                return value;
            }
        public:
            TRoundRobinVector() : std::vector<Value>() {
                Pos = 0;
            }

            virtual ~TRoundRobinVector() {
            }

            inline Value Next();
        };
    }
}
