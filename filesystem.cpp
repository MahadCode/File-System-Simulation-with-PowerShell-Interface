#include "filesystem.h"

FileSystem::FileSystem (
    std::string title = "Disk.bin",
        int diskSize = kOneMiB_g,
        bool alreadyExists = false) : 
        SectorSize(kSectorSize),
        kDiskTitle(title),
        kDiskSize(diskSize) {
    // constructor
    kNumberOfSectors = kDiskSize/kSectorSize;
    kReservedSectors = kNumberOfSectors/128;
    create(false, alreadyExists);
}

void FileSystem::create(bool format, bool alreadyExists) {
    /* objective: to reserve disk space to kDiskSize bytes
    input: format: whether format disk (write all zeroes) or not and 
    return: None 
    effect: memory=kDiskSize (bytes) for a disk is reserved at path=kDiskTitle
    */
    if (format || !alreadyExists) {
        TypeCastInt cast;
        cast.num = 0;
        char buffer[kSectorSize];
        ofstream os;
        os.open(kDiskTitle, ios::binary | ios::out);
        for(int i = 0; i< kSectorSize; ++i) {
            buffer[i] = cast.str[i%4];
        }
        // reserve disk space by filling 0s everywhere
        for(int i = 0; i < kNumberOfSectors; ++i) {
            os.write(buffer, kSectorSize);
        }
        os.close();

        // reserve space for root
        for (int i = 0; i < sectorsForDir; ++i) {
            updateStatus(kReservedSectors + i, DIR_ENTRY);
        }

        if (!alreadyExists) {
            cout << "New FileSystem created of size = " <<
            kDiskSize/kOneMiB_g << "MiB." << endl;
        }

        if (format) {
            cout << "FileSystem formatted." << endl;
        }
    } else {
        cout << "Using pre existing FileSystem : " << kDiskTitle <<
        ", of size = " << kDiskSize/kOneMiB_g << "MiB" << endl;
    }
}

int FileSystem::getStatus(int sector_no) const {
    /*
    objective: to return a Status of a given sector
    input: sector_no = sector number
    return:
        RESERVED: if sector is reserved for status storage
        BUSY: if sector is already in use for data
        FREE: sector is free
        cast.num: otherwise (negative cast.num shows marked as deleted)
    */
    if (sector_no > kNumberOfSectors - 1) {
        return NOT_FOUND;
    } else if (sector_no < kReservedSectors) {
        return RESERVED;
    } else {
        int pos = (sector_no - kReservedSectors)*4;

        // sector to read from disk  (required reserved sector)
        int sector_offset = pos / kSectorSize;

         // bytes to extract from given sector at sector offset  (required bytes of re)
        int byte_in_sector = pos % kSectorSize;

        char buf[kSectorSize];
        ifstream fin;
        fin.open(kDiskTitle, ios::binary | ios::in);
        fin.seekg(sector_offset * kSectorSize);
        fin.read(buf, kSectorSize);
        fin.close();
        
        //storing status of sector in cast
        TypeCastInt cast;
        for (int i = 0; i < 4; ++i) {
            cast.str[i] = buf[byte_in_sector + i];
        }

        if (cast.num == 1) {
            return BUSY;
        } else if (cast.num == 0) {
            return FREE;
        } else {
            return cast.num;
        }
    }
}

#include "filesystem.h"

int FileSystem::updateStatus(int sector_no, int x) {
    /*
    objective: to update status of a given sector
    input: sector_no = sector number , x = status (0 = FREE, 1 = BUSY, other: continuation sector number)
    return: (failure) -1 (success) otherwise
    effect: a sector is marked in disk
    */
    int status = getStatus(sector_no);
    if (status != RESERVED && status != NOT_FOUND) {
        TypeCastInt cast;
        int pos = (sector_no - kReservedSectors)*4;

        // sector to read from disk
        int sector_offset = pos / kSectorSize;

         // bytes to extract from given sector at sector offset
        int byte_in_sector = pos % kSectorSize;

        char buffer[kSectorSize];
        cast.num = x;
        fstream fp;
        fp.open(kDiskTitle, ios::binary | ios::in | ios::out);
        fp.seekg(sector_offset * kSectorSize);
        fp.read(buffer, kSectorSize);
        // edit buffer
        // for (int i = 0; i < 4; ++i) {
        // buffer[byte_in_sector + i] = cast.str[i];
        // }
        for (int i = 0, j = 0; j < 4; ++i, ++j) {
            buffer[byte_in_sector + i] = cast.str[j];
        }
        fp.seekp(sector_offset * kSectorSize);
        fp.write(buffer, kSectorSize);
        fp.close();
        return x;
    } else {
        return -1;
    }
}