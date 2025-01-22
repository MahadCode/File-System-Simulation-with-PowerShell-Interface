#include"Directory.h"

Directory::Directory(string title = "Disk.bin",
        int diskSize = kOneMiB_g,
        bool alreadyExists = false)
 :DiskManager(title,diskSize,alreadyExists){
    currentDir = kReservedSectors;  // root dir
    parentDir = kReservedSectors;
}


int Directory::createDir(const char *title) {
    /*
    objective: creates a directory in directory corresponing to currentDir
    input: title of directory
    return: 0: success, 1: error
    effect:
        a directory entry is passed into the currentDir, and
        sectors are reserved for directory entries that will
        correspond to files/directories in created directory.
    */
    if (strcmp(title, "/") == 0) {
        // there can only be one root dir, anywhere
        cout << "There can only be one root dir, anywhere." << endl;
        return 1;
    }
    TypeCastEntry entry;
    strcpy(entry.entry.name, title);
    entry.entry.type = 'D';
    entry.entry.parent = parentDir;
    entry.entry.size = 0;


    // look for free space and reserve space for this directory's entries
    vector<int> sectorsFree;
    int sec = kReservedSectors + sectorsForDir;
    while (sectorsFree.size() != sectorsForDir) {
        int status = getStatus(sec);
        if (status == FREE) {
            sectorsFree.push_back(sec);
        } else {
            sectorsFree.clear();
        }
        if (status == NOT_FOUND) {
            cout << "Not enough space" << endl;
            return 1;
        }
        ++sec;
    }

    entry.entry.startsAt = sectorsFree[0];
    cout << "Found free space for new dir at sector = " <<
        entry.entry.startsAt << endl;

    // reserve space
    for (int i = 0; i < sectorsForDir; ++i) {
        updateStatus(sectorsFree[i], DIR_ENTRY);
    }

    // find position for file entry
    bool positionFound = false;
    int byteforEntry = 0;
    int sectorForEntry = 0;
    char buf[kSectorSize];
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
                cout << "Directory with same name already exists in current directory."
                 << endl;
                 return 1;
            }
        }
        if (positionFound) break;
    }

    // update buffer for current dir
    cout << "CREATE DIR WITH PARENT = " << entry.entry.parent << endl;
    for (int i = byteforEntry, j = 0; j < 32; ++i, ++j) {
        buf[i] = entry.str[j];
    }

    // write the file entry for current (parent) dir
    writeSector(currentDir + sectorForEntry, buf);
    updateStatus(currentDir + sectorForEntry, DIR_ENTRY);

    return 0;
}

int Directory::changeDir(const char *title) {
    /*
    objective: to change the current working directory
    input: directory to change to
    return:
        0: sucess
        1: fail (child directory doesn not exists in parent directory)
    effect: the currentDir and parentDir are updated
    */
    TypeCastEntry entry;
    strncpy(entry.entry.name, title, 19);

    if (strcmp(entry.entry.name, "..") == 0) {
        // go to one level up in directory
        if (currentDir == parentDir) {
            return 0;
        }
        char buf[kSectorSize];
        TypeCastEntry test;
        for (int i = 0; i < sectorsForDir; ++i) {
            readSector(parentDir + i, buf);
            for (int j = 0; j < kSectorSize; j += 32) {
                for (int k = 0; k < 32; ++k) {
                    test.str[k] = buf[j+k];
                }
                if (test.entry.startsAt == currentDir) {
                    currentDir = parentDir;
                    parentDir = test.entry.parent;
                    return 0;
                }
            }
        }
    } else if (strcmp(entry.entry.name, "\\") == 0) {
        // go to root directory
        if (currentDir == parentDir) {
            return 0;
        }
        currentDir = kReservedSectors;
        parentDir = kReservedSectors;
        return 0;
    } else {
        char buf[kSectorSize];
        TypeCastEntry test;
        for (int i = 0; i < sectorsForDir; ++i) {
            readSector(currentDir + i, buf);
            for (int j = 0; j < kSectorSize; j += 32) {
                for (int k = 0; k < 32; ++k) {
                    test.str[k] = buf[j+k];
                }
                if (strcmp(test.entry.name, entry.entry.name) == 0) {
                    // if right name found in Dir Entry
                    parentDir = currentDir;
                    currentDir = test.entry.startsAt;
                    return 0;
                }
            }
        }
    }

    cout << "Directory not found" << endl;
    return 1;
}

int Directory::printWorkingDir() {
    /*
    objective: to print the current working directory
    input: none
    effect: the current working directory with respect to root directory
    */
    if (currentDir == kReservedSectors) {
        cout << "/" << endl;
    } else {
        int par = parentDir;
        int cur = currentDir;
        string dirPath = "";
        do {
            char buf[kSectorSize];
            readSector(par, buf);
            for (int j = 0; j < kSectorSize; j += 32) {
                TypeCastEntry entry;
                for (int k = 0; k < 32; ++k) {
                    entry.str[k] = buf[j+k];
                }
                if (entry.entry.startsAt == cur) {
                    cur = par;
                    par = entry.entry.parent;
                    string temp = entry.entry.name;
                    dirPath = temp + "/" +  dirPath;
                    break;
                }
            }
        } while (cur != kReservedSectors);

        cout << "/" + dirPath << endl;
    }
    return 0;
}


void Directory::listDirectoryContents() {
    /*
    objective: prints all files and directories in disk
    input: None
    return: None
    output: list of files and directories in disk
    */
    char buf[kSectorSize];
    TypeCastEntry entry;
    for (int i = 0; i < sectorsForDir; ++i) {
        readSector(currentDir + i, buf);
        for (int j = 0; j < kSectorSize; j += 32) {
            for (int k = 0; k < 32; ++k) {
                entry.str[k] = buf[j+k];
            }
                if (entry.entry.type == 'F') {
                    cout << left << setw(22) <<
                    entry.entry.name << right <<
                    setw(10) << entry.entry.size << " bytes" << endl;
                } else if (entry.entry.type == 'f') {
                    cout << left << setw(22) << "(" <<
                    entry.entry.name << ")" << right <<
                    setw(10) << entry.entry.size << " bytes" << endl;
                } else if ((entry.entry.type == 'D')) {
                    cout << left << KBLU << setw(22) <<
                    entry.entry.name << KRST << right <<
                    setw(10) << "[DIR]" << endl;
                }
        }
    }
}
