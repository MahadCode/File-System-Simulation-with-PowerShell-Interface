#ifndef DIRECTORY_H
#define DIRECTORY_H
#include"diskmanager.h"


// directory entry
typedef struct {
    int  startsAt;
    int  size;
    char type;    // y/n
    int  parent;
    char name[19];
} dirEntry;

typedef struct {
    union {
        dirEntry entry;
        char str[32];
    };
} TypeCastEntry;

class Directory: public DiskManager{
    protected:
    int currentDir;
    int parentDir;
    public:
    Directory(string title, int diskSize, bool alreadyExists);
    int createDir(const char *title);
    int changeDir(const char *title);
    int printWorkingDir();
    void listDirectoryContents();
};
#endif