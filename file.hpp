#pragma once

#include <string>
#include <sys/types.h>
#include <string.h>
#include <utility>

namespace NAC {
    class TFile {
    public:
        enum EAccess {
            ACCESS_INFO,
            ACCESS_RDONLY,
            ACCESS_RDWR,
            ACCESS_CREATE,
            ACCESS_TMP,
        };

    public:
        TFile() = delete;
        TFile(const TFile&) = delete;
        TFile(const std::string& path, EAccess access = ACCESS_RDONLY, mode_t mode = 0644);

        explicit operator bool() const {
            return Ok;
        }

        const std::string& Path() const {
            return Path_;
        }

        ~TFile();

        TFile(TFile&& right) {
            Fh = right.Fh;
            Len_ = right.Len_;
            Addr_ = right.Addr_;
            Ok = right.Ok;
            Access = right.Access;

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
            Access = right.Access;

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

        char operator[](const size_t index) const {
            return Data()[index];
        }

        void Resize(off_t length);
        void Seek(off_t offset, int whence);
        void SeekToEnd();
        void SeekToStart();
        void Stat();
        void Map();

        bool MSync() const;
        bool FSync() const;

        TFile& Append(const size_t size, const char* data);

        TFile& Append(const char* src) {
            return Append(strlen(src), src);
        }

        TFile& Append(const std::string& src) {
            return Append(src.size(), src.data());
        }

        template<typename... TArgs>
        TFile& operator<<(TArgs&&... args) {
            return Append(std::forward<TArgs&&>(args)...);
        }

    private:
        int OpenAccess() const;
        int MMapAccess() const;

    private:
        int Fh = -1;
        size_t Len_ = 0;
        void* Addr_ = nullptr;
        bool Ok = false;
        EAccess Access;
        std::string Path_;
    };
}
