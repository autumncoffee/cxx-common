#include "file.hpp"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>

namespace NAC {
    TFile::TFile(const std::string& path, EAccess access, mode_t mode)
        : Access(access)
        , Path_(path)
    {
        if (Access == ACCESS_INFO) {
            struct stat buf;

            if (stat(Path_.c_str(), &buf) == -1) {
                perror("stat");
                return;
            }

            Len_ = buf.st_size;
            Ok = true;

            return;
        }

        if (Access == ACCESS_TMP) {
            Access = ACCESS_CREATE;
            Fh = mkstemp(Path_.data());

        } else {
            Fh = open(Path_.c_str(), OpenAccess(), mode);
        }

        if (Fh == -1) {
            perror("open");
            return;
        }

        Ok = true;

        if (Access == ACCESS_CREATE) {
            return;
        }

        Stat();
        Map();
    }

    int TFile::OpenAccess() const {
        return (
            (Access == ACCESS_RDONLY)
                ? O_RDONLY
                : (O_RDWR | (
                    (Access == ACCESS_CREATE)
                        ? O_CREAT
                        : 0
                ))
        );
    }

    int TFile::MMapAccess() const {
        return (
            (Access == ACCESS_RDONLY)
                ? PROT_READ
                : (PROT_READ | PROT_WRITE)
        );
    }

    TFile::~TFile() {
        if (Ok && Addr_) {
            munmap(Addr_, Len_);
        }

        if (Fh != -1) {
            close(Fh);
        }
    }

    void TFile::Resize(off_t length) {
        if (!Ok || (Fh == -1)) {
            return;
        }

        if (Addr_) {
            munmap(Addr_, Len_);
            Addr_ = nullptr;
        }

        Len_ = length;

        if (ftruncate(Fh, length) == -1) {
            Ok = false;
            perror("ftruncate");
            return;
        }

        Map();
    }

    bool TFile::MSync() const {
        if (Ok && Addr_) {
            if (msync(Addr_, Len_, 0) == -1) {
                perror("msync");
                return false;
            }
        }

        return true;
    }

    bool TFile::FSync() const {
        if (Ok && Addr_) {
            if (fsync(Fh) == -1) {
                perror("fsync");
                return false;
            }
        }

        return true;
    }

    TFile& TFile::Append(const size_t size, const char* data) {
        if (!Ok || (size == 0) || (Fh == -1)) {
            return *this;
        }

        size_t offset(0);

        while (offset < size) {
            auto n = write(Fh, data + offset, size - offset);

            if (n < 0) {
                if (errno != EINTR) {
                    perror("write");
                    Ok = false;
                    break;
                }

            } else {
                offset += n;
            }
        }

        return *this;
    }

    void TFile::Map() {
        if (!Ok || Addr_) {
            return;
        }

        Ok = false;

        if (Len_ == 0) {
            return;
        }

        Addr_ = mmap(nullptr, Len_, MMapAccess(), MAP_SHARED, Fh, 0);

        if (Addr_ == MAP_FAILED) {
            perror("mmap");
            return;
        }

        Ok = true;
    }

    void TFile::Stat() {
        if ((Len_ > 0) || (Fh == -1)) {
            return;
        }

        struct stat buf;

        if (fstat(Fh, &buf) == -1) {
            perror("fstat");
            return;
        }

        Len_ = buf.st_size;
    }
}
