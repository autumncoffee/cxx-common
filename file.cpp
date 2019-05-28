#include "file.hpp"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

namespace NAC {
    TFile::TFile(const std::string& path, EAccess access) {
        if (access == ACCESS_INFO) {
            struct stat buf;

            if (stat(path.c_str(), &buf) == -1) {
                perror("stat");
                return;
            }

            Len_ = buf.st_size;
            Ok = true;

            return;
        }

        Fh = open(path.c_str(), ((access == ACCESS_RDONLY) ? O_RDONLY : O_RDWR));

        if (Fh == -1) {
            perror("open");
            return;
        }

        {
            struct stat buf;

            if (fstat(Fh, &buf) == -1) {
                perror("fstat");
                return;
            }

            Len_ = buf.st_size;
        }

        if (Len_ == 0) {
            return;
        }

        Addr_ = mmap(nullptr, Len_, ((access == ACCESS_RDONLY) ? PROT_READ : (PROT_READ | PROT_WRITE)), MAP_SHARED, Fh, 0);

        if (Addr_ == MAP_FAILED) {
            perror("mmap");
            return;
        }

        Ok = true;
    }

    TFile::~TFile() {
        if (Ok) {
            munmap(Addr_, Len_);
        }

        if (Fh != -1) {
            close(Fh);
        }
    }
}
