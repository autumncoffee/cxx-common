#pragma once

#include <string>
#include <sys/types.h>
#include <string.h>
#include <utility>
#include "str.hpp"

namespace NAC {
    class TFileIterator {
    public:
        TFileIterator(int fh, size_t len)
            : Fh(fh)
            , Len(len)
        {
        }

        virtual TBlob Next() = 0;

        explicit operator bool() const {
            return ((Fh != -1) && (Len > 0));
        }

    protected:
        int Fh = -1;
        size_t Len = 0;
    };

    class TFileChunkIterator : public TFileIterator {
    public:
        template<typename... TArgs>
        TFileChunkIterator(size_t chunkSize, TArgs... args)
            : TFileIterator(std::forward<TArgs>(args)...)
        {
            if (!*this || (chunkSize == 0)) {
                Fh = -1;
                return;
            }

            Chunk.Reserve(chunkSize);
        }

        TBlob Next() override;

    private:
        TBlob Chunk;
        size_t Offset = 0;
    };

    class TFilePartIterator : public TFileChunkIterator {
    public:
        template<typename... TArgs>
        TFilePartIterator(const TBlob& delimiter, size_t chunkSize, TArgs... args)
            : TFileChunkIterator(chunkSize, std::forward<TArgs>(args)...)
        {
            if (!*(TFileChunkIterator*)this) {
                Offset = -1;
                return;
            }

            Delimiter.Reserve(delimiter.Size());
            Delimiter.Append(delimiter.Size(), delimiter.Data());
        }

        TBlob Next() override;

        explicit operator bool() const {
            return (Offset != -1);
        }

    private:
        TBlob Find(const TBlob&, size_t) const;

    private:
        TBlob Delimiter;
        TBlob Buf;
        ssize_t Offset = 0;
    };

    class TFile {
    public:
        enum EAccess {
            ACCESS_INFO,
            ACCESS_RDONLY,
            ACCESS_RDWR,
            ACCESS_CREATE,
            ACCESS_TMP,
            ACCESS_CREATEX,
            ACCESS_WRONLY,

            ACCESS_CREATE_FSYNC,
            ACCESS_CREATEX_FSYNC,
            ACCESS_WRONLY_FSYNC,

            ACCESS_RDONLY_DIRECT,
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

        ino_t INodeNum() const {
            return INode_;
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
        TFile& Write(const off_t offset, const size_t size, const char* data);

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

        TFile& Write(const off_t offset, const char* src) {
            return Write(offset, strlen(src), src);
        }

        TFile& Write(const off_t offset, const std::string& src) {
            return Write(offset, src.size(), src.data());
        }

        TFileChunkIterator Chunks(size_t chunkSize) const {
            return TFileChunkIterator(chunkSize, Fh, Len_);
        }

        TFilePartIterator Parts(const TBlob& delimiter, size_t chunkSize) const {
            return TFilePartIterator(delimiter, chunkSize, Fh, Len_);
        }

        TFilePartIterator Parts(const std::string& delimiter, size_t chunkSize) const {
            return Parts(TBlob(delimiter.size(), delimiter.data()), chunkSize);
        }

        TFilePartIterator Lines(size_t chunkSize = 4096) const {
            return Parts("\n", chunkSize);
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
        ino_t INode_;

#ifndef __linux__
        bool AutoFSync = false;
#endif
    };
}
