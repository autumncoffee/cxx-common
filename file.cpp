#include "file.hpp"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>

#ifdef __linux__

#define FSYNC_ACCESS(base) case base ## _FSYNC: \
    Access = base; \
    openAccess |= O_DIRECT | O_SYNC; \
    break;

#define WRITE_FSYNC_KLUDGE()

#else

#define FSYNC_ACCESS(base) case base ## _FSYNC: \
    Access = base; \
    AutoFSync = true; \
    break;

#define WRITE_FSYNC_KLUDGE() if (AutoFSync && !FSync()) { \
    Ok = false; \
    break; \
}

#endif

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
            Fh = mkstemp(Path_.data());

        } else {
            int openAccess = OpenAccess();

            switch (Access) {
                FSYNC_ACCESS(ACCESS_CREATE)
                FSYNC_ACCESS(ACCESS_CREATEX)
                FSYNC_ACCESS(ACCESS_WRONLY)

                default:
                    break;
            }

            Fh = open(Path_.c_str(), openAccess, mode);
        }

        if (Fh == -1) {
            perror("open");
            return;
        }

        Ok = true;

        switch (Access) {
            case ACCESS_CREATE:
            case ACCESS_TMP:
            case ACCESS_CREATEX:
            case ACCESS_WRONLY:
                return;

            default:
                break;
        }

        Stat();
        Map();
    }

    int TFile::OpenAccess() const {
        return (
            (Access == ACCESS_RDONLY)
                ? O_RDONLY
                : ((Access == ACCESS_WRONLY)
                    ? O_WRONLY
                    : (O_RDWR | (
                        (Access == ACCESS_CREATE)
                            ? O_CREAT
                            : (Access == ACCESS_CREATEX)
                                ? (O_CREAT | O_EXCL)
                                : 0
                    ))
                )
        );
    }

    int TFile::MMapAccess() const {
        return (
            (Access == ACCESS_RDONLY)
                ? PROT_READ
                : ((Access == ACCESS_WRONLY)
                    ? PROT_WRITE
                    : (PROT_READ | PROT_WRITE)
                )
        );
    }

    TFile::~TFile() {
        if (Ok && Addr_) {
            munmap(Addr_, Len_);
        }

        if (Fh != -1) {
            close(Fh);
        }

        if (Access == ACCESS_TMP) {
            unlink(Path_.c_str());
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
                WRITE_FSYNC_KLUDGE()

                offset += n;
            }
        }

        return *this;
    }

    TFile& TFile::Write(const off_t offset_, const size_t size, const char* data) {
        if (!Ok || (size == 0) || (Fh == -1)) {
            return *this;
        }

        size_t offset(0);

        while (offset < size) {
            auto n = pwrite(Fh, data + offset, size - offset, offset_ + offset);

            if (n < 0) {
                if (errno != EINTR) {
                    perror("pwrite");
                    Ok = false;
                    break;
                }

            } else {
                WRITE_FSYNC_KLUDGE()

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

    void TFile::Seek(off_t offset, int whence) {
        if (!Ok || (Fh == -1)) {
            return;
        }

        if (lseek(Fh, offset, whence) == -1) {
            perror("lseek");
            Ok = false;
        }
    }

    void TFile::SeekToEnd() {
        Seek(0, SEEK_END);
    }

    void TFile::SeekToStart() {
        Seek(0, SEEK_SET);
    }
}
