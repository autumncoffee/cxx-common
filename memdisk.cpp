#include <stdexcept>
#include "memdisk.hpp"

namespace NAC {
    TMemDisk::TMemDisk(size_t memMax, const std::string& diskMask)
        : MemMax(memMax)
        , DiskMask(diskMask)
    {
    }

    TMemDisk& TMemDisk::Append(const size_t size, const char* data) {
        if (Disk) {
            Disk->Append(size, data);

        } else if ((Mem.Size() + size) > MemMax) {
            Disk.reset(new TFile(DiskMask, TFile::ACCESS_TMP));
            Disk->Append(Mem.Size(), Mem.Data());
            Disk->Append(size, data);
            Mem = TBlob();

        } else {
            Mem.Append(size, data);
        }

        return *this;
    }

    void TMemDisk::Finish() {
        if (Disk) {
            Disk->Stat();
            Disk->Map();

            if (*Disk) {
                Mem.Wrap(Disk->Size(), Disk->Data());
            }
        }
    }
}
