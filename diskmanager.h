#ifndef DISK_MANAGER
#define DISK_MANAGER

#include"filesystem.h"

class DiskManager: public FileSystem {
 protected:
    int findFreeSectors(int sectorsNeeded, vector<int> &sectorsFree);
 public:
    DiskManager(string title, int diskSize, bool alreadyExists);
    void format();
    void readSector(int sector_no, char *buf) const;
    void writeSector(int sector_no, char *buf);
};

#endif
