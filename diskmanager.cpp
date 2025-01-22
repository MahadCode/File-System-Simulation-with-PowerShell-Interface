#include"diskmanager.h"
DiskManager::DiskManager(std::string title = "Disk.bin",
        int diskSize = kOneMiB_g,
        bool alreadyExists = false) 
: FileSystem(title,diskSize,alreadyExists){
}

void DiskManager::format() {
    // format disk
    create(true, true);
}

void DiskManager::readSector(int sector_no, char *buf) const {
    /*
    objective: to read a sector from disk
    input:
        sector_no = sector number to read
        buf = char buffer to read into
    return: None
    effect: None
        sector data is printed if it exists & it is not reserved
        sector data is also set into *buf
    */
    int status = getStatus(sector_no);
    if (status == RESERVED) {
        cout << "It is a reserved sector." << endl;
    } else if (status == NOT_FOUND) {
        cout << "Sector does not exist." << endl;
        return;
    }
    ifstream fin;
    fin.open(kDiskTitle, ios::binary | ios::in);
    int byteOffset = sector_no*kSectorSize;
    fin.seekg(byteOffset);
    fin.read(buf, kSectorSize);
    fin.close();
}

void DiskManager::writeSector(int sector_no, char* buf) {
    /*
    objective: to write data into a sector
    input:
        sector_no = sector number to write into
        buf = buffer to write from (size = kSectorSize)
    return: None
    effect:
        data is written into a sector if it is FREE OR BUSY (over-written)
        else: nothing
    */
    int status = getStatus(sector_no);
    if (status == RESERVED) {
        cout << "It is a reserved sector" << endl;
        return;
    } else if (status == NOT_FOUND) {
        cout << "Sector does not exist." << endl;
        return;
    } else {
        ofstream fp;
        fp.open(kDiskTitle, ios::binary | ios::in | ios::out);
        fp.seekp(sector_no*kSectorSize);
        fp.write(buf, kSectorSize);
        fp.close();
    }
}

int DiskManager::findFreeSectors(int sectorsNeeded, vector<int> &sectorsFree) {
    int sectorsFound = 0;
    int sector = kReservedSectors + sectorsForDir;
    while (sectorsFound < sectorsNeeded) {
        int status = getStatus(sector);
        if (status == FREE) {
            // add sector to usable list
            sectorsFree[sectorsFound++] = sector;
        } else if (status == NOT_FOUND) {
            // look in deleted sectors now
            cout << "No more FREE sectors." << endl;
            break;
        }
        ++sector;
    }
    
    // look for available sectors that are marked deleted
    sector = kReservedSectors + sectorsForDir;
    while (sectorsFound < sectorsNeeded) {
        int status = getStatus(sector);
        if (status < 0) {
            sectorsFree[sectorsFound++] = sector;
        } else if (status == NOT_FOUND) {
            cout << "Not enough space in disk!" << endl;
            return 1;
        }
        ++sector;
    }
    return 0;
}