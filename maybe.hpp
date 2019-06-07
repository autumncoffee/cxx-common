#pragma once

#include <utility>

namespace NAC {
    template<typename T>
    class TMaybe {
    public:
        using TValue = T;

    private:
        using TSelf = TMaybe<TValue>;

    public:
        TMaybe() = default;
        TMaybe(const TSelf&) = default;
        TMaybe(TSelf&&) = default;

        TMaybe(const TValue& value)
            : Defined(true)
            , Data(value)
        {
        }

        TMaybe(TValue&& value)
            : Defined(true)
            , Data(std::move(value))
        {
        }

        TSelf& operator=(const TSelf&) = default;
        TSelf& operator=(TSelf&&) = default;

        TSelf& operator=(const TValue& value) {
            Data = value;
            Defined = true;

            return *this;
        }

        TSelf& operator=(TValue&& value) {
            Data = std::move(value);
            Defined = true;

            return *this;
        }

        TValue& operator*() {
            return Data;
        }

        const TValue& operator*() const {
            return Data;
        }

        operator bool() const {
            return Defined;
        }

    private:
        bool Defined = false;
        union {
            char Dummy = 0;
            TValue Data;
        };
    };
}
