#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <iomanip>
#include <vector>
#include <sstream>
using namespace std;

#ifndef COLORS    // colors for terminal
#define COLORS ""
#define KRST  "\x1B[0m"
#define KGRN "\x1b[32m"
#define KYEL "\x1b[33m"
#define KBLU  "\x1B[34m"
#define DASH u8"\u2015" << u8"\u2015" << " "
#endif

// int to char[4] typecast
typedef struct {
    union {
        int num;
        char str[4];
    };
} TypeCastInt;


const int kSectorSize = 512;
const int kOneMiB_g = pow(2, 20);   // 2^20 bytes = 1MiB
const int MAX_FILE_SIZE = 524288;

class FileSystem {
    // FileSystem class
 protected:
    const int SectorSize;
    string kDiskTitle;
    int kDiskSize;
    int kReservedSectors;
    int kNumberOfSectors;
    const int sectorsForDir = 2;
    void create(bool format = false, bool alreadyExists = false);
 public:
    FileSystem(string title, int diskSize, bool alreadyExists);
    enum Status {FREE = 0, BUSY, RESERVED, NOT_FOUND, DIR_ENTRY, DELETED, END};
    const vector<string> statusMsg {
        "FREE", "BUSY", "RESERVED", "NOT_FOUND", "DIR_ENTRY", "DELETED", "END"
    };
    int getStatus(int sector_no) const;
    int updateStatus(int sector_no, int x);
 };
#endif