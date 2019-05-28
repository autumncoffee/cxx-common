#pragma once

#include <string>

namespace NAC {
    class TFile {
    public:
        enum EAccess {
            ACCESS_INFO,
            ACCESS_RDONLY,
            ACCESS_RDWR,
        };

    public:
        TFile() = delete;
        TFile(const TFile&) = delete;
        TFile(const std::string& path, EAccess access = ACCESS_RDONLY);

        bool IsOK() const {
            return Ok;
        }

        ~TFile();

        TFile(TFile&& right) {
            Fh = right.Fh;
            Len_ = right.Len_;
            Addr_ = right.Addr_;
            Ok = right.Ok;

            right.Fh = -1;
            right.Len_ = 0;
            right.Addr_ = nullptr;
            right.Ok = false;
        }

        TFile& operator=(const TFile&) = delete;

        TFile& operator=(TFile&& right) {
            Fh = right.Fh;
            Len_ = right.Len_;
            Addr_ = right.Addr_;
            Ok = right.Ok;

            right.Fh = -1;
            right.Len_ = 0;
            right.Addr_ = nullptr;
            right.Ok = false;

            return *this;
        }

        char* Data() const {
            return (char*)Addr_;
        }

        size_t Size() const {
            return Len_;
        }

    private:
        int Fh = -1;
        size_t Len_ = 0;
        void* Addr_ = nullptr;
        bool Ok = false;
    };
}
