#include "blkgetsize.hpp"

// See https://github.com/mirage/mirage-block-unix/blob/master/lib/blkgetsize_stubs.c

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

namespace NAC {

#ifdef __linux__
#include <linux/fs.h>

    int BlkGetSize(int fd, uint64_t* size) {

#ifdef BLKGETSIZE64
        const int ret = ioctl(fd, BLKGETSIZE64, size);

#elif BLKGETSIZE
        unsigned long sectors(0);
        const int ret = ioctl(fd, BLKGETSIZE, &sectors);
        *size = sectors * 512ULL;

#else
    #error "Linux configuration error (blkgetsize)"

#endif
        return ret;
    }

#elif defined(__APPLE__)
#include <sys/disk.h>

    int BlkGetSize(int fd, uint64_t* size) {
        unsigned long blocksize(0);
        int ret = ioctl(fd, DKIOCGETBLOCKSIZE, &blocksize);

        if (!ret) {
            unsigned long nblocks(0);
            ret = ioctl(fd, DKIOCGETBLOCKCOUNT, &nblocks);

            if (!ret) {
                *size = (uint64_t)nblocks * blocksize;
            }
        }

        return ret;
    }

#elif defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/disk.h>

    int BlkGetSize(int fd, uint64_t* size) {
        return ioctl(fd, DIOCGMEDIASIZE, size);
    }

#else
    #error "Unable to query block device size: unsupported platform"

#endif
}
