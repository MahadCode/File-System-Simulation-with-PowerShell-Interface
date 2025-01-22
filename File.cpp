#include"File.h"

file::file(string title = "Disk.bin",
        int diskSize = kOneMiB_g,
        bool alreadyExists = false):
        Directory{title,diskSize,alreadyExists} {}

int file::createFile(const char *title, const char *srcFile) {
    /*
    objective: to create a file that can be written in multiple non-continuous sectors
    input: title: title of file , srcFile: source file to read from
    return: 0: success, 1: error
    effect: a new file is created in disk and directory entry is passed, if success
    */
    TypeCastEntry entry;
    if (strlen(title) == 0) {
        cout << "File title: ";
        cin >> entry.entry.name;
        cin.ignore(32767, '\n');
    } else {
        strcpy(entry.entry.name, title);
    }

    // default parent is root
    entry.entry.parent = currentDir;

    // not a directory, is a file
    entry.entry.type = 'F';

    // file contents & size
    char file_content[MAX_FILE_SIZE];
    if (strlen(srcFile) == 0) {
        cout << "Enter content of file:" << endl;
        cin.getline(file_content, sizeof(file_content));
        entry.entry.size = strlen(file_content);
    } else {
        ifstream fsrc;
        fsrc.open(srcFile, ios::binary | ios::in);
        fsrc.read(file_content, MAX_FILE_SIZE);
        fsrc.close();
        fsrc.open(srcFile, ios::binary | ios::in | ios::ate);
        entry.entry.size = fsrc.tellg();
        fsrc.close();
    }

    if (entry.entry.size < 0) {
        return 1;
    }


    // find sectors to write in
    int sectorsNeeded = (entry.entry.size - 1) / kSectorSize + 1;
    vector<int> sectorsFree(sectorsNeeded);
    if (findFreeSectors(sectorsNeeded, sectorsFree) == 1) {
        return 1;
    };

    entry.entry.startsAt = sectorsFree[0];

    char buf[kSectorSize];
    // find position for file entry
    bool positionFound = false;
    int byteforEntry = 0;
    int sectorForEntry = 0;
    for (int s = 0; s < sectorsForDir; ++s) {
        readSector(currentDir + s, buf);
        for (int b = 0; b < kSectorSize; b += 32) {
            TypeCastEntry test;
            for (int k = 0; k < 32; ++k) {
                test.str[k] = buf[b+k];
            }
            if (strlen(test.entry.name) == 0) {
                positionFound = true;
                sectorForEntry = s;
                byteforEntry = b;
                break;
            } else if (strcmp(test.entry.name, entry.entry.name) == 0) {
                cout << "File with same name already exists in directory."
                 << endl;
                 return 1;
            }
        }
        if (positionFound) break;
    }
    if (!positionFound) {
        cout << "Cannot contain more than " << sectorsForDir*16 << " entries" << endl;
        return 1;
    }
    // update buffer
    for (int i = byteforEntry, j = 0; j < 32; ++i, ++j) {
        buf[i] = entry.str[j];
    }
    // write the file entry
    writeSector(currentDir + sectorForEntry, buf);
    updateStatus(currentDir + sectorForEntry, DIR_ENTRY);

    // write data and update sector status
    for (int i = 0; i < sectorsNeeded; ++i) {
        for (int j = 0; j < kSectorSize; ++j) {
            buf[j] = file_content[kSectorSize*i + j];
        }
        writeSector(sectorsFree[i], buf);
        updateStatus(sectorsFree[i],
            (i != sectorsNeeded-1) ? sectorsFree[i+1] : 1);
    }
    cout << "CREATED FILE WITH" << endl;
    cout << "TITLE = " << entry.entry.name << endl;
    cout << "SIZE = " <<
        entry.entry.size << " bytes" << endl;
    cout << "STORED IN SECTORS = ";
    for (int i = 0; i < sectorsNeeded; ++i)
        cout << sectorsFree[i] << " ";
    cout << endl;

    return 0;
}


int  file::deleteFile(const char *title) {
    /*
    objective: to delete a file (in recoverable form) from current directory
    input: file title
    return:
        1: fail
        0: success
    effect: the sectors corresponding to a file are marked deleted (denoted by negative integers)
        the type in directory entry for the file changes for 'F' to 'f', denoting deleted.
    */

    char buf[kSectorSize];
    // find position for file entry
    bool positionFound = false;
    bool alreadyDeleted = false;
    int byteforEntry = 0;
    int sectorForEntry = 0;
    TypeCastEntry test;
    for (int s = 0; s < sectorsForDir; ++s) {
        readSector(currentDir + s, buf);
        for (int b = 0; b < kSectorSize; b += 32) {
            for (int k = 0; k < 32; ++k) {
                test.str[k] = buf[b+k];
            }
            if (strcmp(test.entry.name, title) == 0) {
                if (test.entry.type == 'F') {
                    positionFound = true;
                    sectorForEntry = s;
                    byteforEntry = b;
                    break;
                } else if (test.entry.type == 'f') {
                    alreadyDeleted = true;
                    sectorForEntry = s;
                    byteforEntry = b;
                    break;
                }
            }
        }
        if (positionFound || alreadyDeleted) break;
    }
    if (!positionFound && !alreadyDeleted) {
        std::cout << "No such file found." << std::endl;
        return 1;
    }
    if (alreadyDeleted) {
        // remove permanently
        int status = test.entry.startsAt;
        do {
            int nxt = getStatus(-status);
            updateStatus(-status, 0);
            status = nxt;
        } while (status != -1);

        for (int i = byteforEntry, j = 0; j < 32; ++i, ++j) {
            buf[i] = static_cast<char>(0);
        }
        writeSector(currentDir + sectorForEntry, buf);
        std::cout << "permanently deleted file: " << title << std::endl;
        return 0;
    }


    int status = test.entry.startsAt;
    do {
        int nxt = getStatus(status);
        updateStatus(status, -nxt);
        status = nxt;
    } while (status != 1);

    test.entry.type = 'f';
    test.entry.startsAt = -test.entry.startsAt;
    for (int i = byteforEntry, j = 0; j < 32; ++i, ++j) {
        buf[i] = test.str[j];
    }
    writeSector(currentDir + sectorForEntry, buf);
    std::cout << "deleted file: " << title << std::endl;
    return 0;
}

int file::undeleteFile(const char *title) {
    /*
    objective: (attempt) to undelete a file (recover) from current directory
    input: file title
    return:
        1: fail
        0: success
    effect:
        if success:
        the sectors corresponding to a file are marked as original un-deleted.
        the type in directory entry for the file changes from 'f' to 'F', denoting recovered.
    */

    char buf[kSectorSize];
    // find position for file entry
    bool positionFound = false;
    int byteforEntry = 0;
    int sectorForEntry = 0;
    TypeCastEntry test;
    for (int s = 0; s < sectorsForDir; ++s) {
        readSector(currentDir + s, buf);
        for (int b = 0; b < kSectorSize; b += 32) {
            for (int k = 0; k < 32; ++k) {
                test.str[k] = buf[b+k];
            }
            if (strcmp(test.entry.name, title) == 0
                && test.entry.type == 'f') {
                positionFound = true;
                sectorForEntry = s;
                byteforEntry = b;
                break;
            }
        }
        if (positionFound) break;
    }
    if (!positionFound) {
        std::cout << "No such file found." << std::endl;
        return 1;
    }


    int status = test.entry.startsAt;
    int sectorsToLookFor = test.entry.size / kSectorSize + 1;
    bool isRecoverable = true;
    int sec = 0;
    while (sec < sectorsToLookFor) {
        int nxt = getStatus(-status);
        if (nxt > -1) {
            isRecoverable = false;
            break;
        }
        ++sec;
        status = nxt;
    };


    if (isRecoverable) {
        status = test.entry.startsAt;
        do {
            int nxt = getStatus(-status);
            status = nxt;
            updateStatus(-status, -nxt);
        } while (status != -1);

        test.entry.type = 'F';
        test.entry.startsAt = -test.entry.startsAt;
        for (int i = byteforEntry, j = 0; j < 32; ++i, ++j) {
            buf[i] = test.str[j];
        }
        writeSector(currentDir + sectorForEntry, buf);
        std::cout << "recovered file: " << title << std::endl;
        return 0;
    } else {
        for (int i = byteforEntry, j = 0; j < 32; ++i, ++j) {
            buf[i] = static_cast<char>(0);
        }
        writeSector(currentDir + sectorForEntry, buf);
        std::cout << "un-recoverable file: " << title << std::endl;
        return 1;
    }
}

int file::readFile(const char *title) {
    /*
    objective: to read a single file from multiple non-continuous sectors
    input:
        title: title of file
    return:
        0: success
        1: error
    effect: contents of file are printed, if success
    */
    TypeCastEntry entry;
    if (strlen(title) == 0) {
        std::cout << "File title: ";
        std::cin >> entry.entry.name;
        std::cin.ignore(32767, '\n');
    } else {
        strcpy(entry.entry.name, title);
    }

    entry.entry.startsAt = 0;

    char buf[kSectorSize];
    TypeCastEntry test;
    for (int i = 0; i < sectorsForDir; ++i) {
        readSector(currentDir + i, buf);
        for (int j = 0; j < kSectorSize; j += 32) {
            for (int k = 0; k < 32; ++k) {
                test.str[k] = buf[j+k];
            }
            if (strcmp(test.entry.name, entry.entry.name) == 0) {
                entry.entry.startsAt = test.entry.startsAt;
                entry.entry.size = test.entry.size;
                entry.entry.parent = test.entry.parent;
                if (test.entry.type != 'F') return 1;
                break;
            }
        }
        if (entry.entry.startsAt != 0) break;
    }
    if (entry.entry.startsAt == 0) {
        std::cout << "file not found" << std::endl;
        return 1;
    } else {
        // read file content
        std::cout << "READING FILE WITH" << std::endl;
        std::cout << "TITLE = " << test.entry.name << std::endl;
        std::cout << "SIZE = " << entry.entry.size << " bytes." << std::endl;

        int sec = entry.entry.startsAt;
        while (sec != 1) {
            readSector(sec, buf);
            for (int i = 0; i < kSectorSize; ++i) {
                std::cout << buf[i];
            }
            sec = getStatus(sec);
            if(sec == RESERVED){
                std::cout << endl << "It's  a recoverd file. It maybe corrupted" << std::endl;
                break;
            }
        }
        std::cout << std::endl;
    }
    return 0;
}

void printTable(std::string str, int val1, int val2 = 0) {
    /*
    formats output as a table
    */
    if (val2 == 0) {
        std::cout << std::left << std::setw(32) << str <<
            ": " << val1 << std::endl;
    } else {
        std::cout << std::left << std::setw(32) << str <<
            ": (" << val1 << "/" << val2 << ")" << std::endl;
    }
}

void file::info() {
    /*
    objective: prints info about file system
    input: None
    return: None
    output: info about file system
    */
    int sectorsFree = 0, sectorsDeleted = 0, sectorsBusy = 0,
    sectorsForDir = 0;
    for (int i = kReservedSectors; i < kNumberOfSectors; ++i) {
        int status = getStatus(i);
        if (status == FREE) {
            ++sectorsFree;
        } else if (status < 0) {
            ++sectorsDeleted;
        } else if (status == DIR_ENTRY) {
            ++sectorsForDir;
        } else {
            ++sectorsBusy;
        }
    }
    printTable("FileSystem Size (bytes)", kDiskSize);
    printTable("Sector Size (bytes)", kSectorSize);
    printTable("Number of sectors", kNumberOfSectors);
    printTable("Reserved Sectors", kReservedSectors, kNumberOfSectors);
    printTable("Sectors Free", sectorsFree, kNumberOfSectors);
    printTable("Sectors Busy", sectorsBusy, kNumberOfSectors);
    printTable("Sectors for DIR_ENTRY", sectorsForDir, kNumberOfSectors);
    printTable("Sectors marked DELETED", sectorsDeleted, kNumberOfSectors);
}