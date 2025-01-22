#ifndef FILE_H
#define FILE_H
#include"Directory.h"

class file:public Directory{
    public:
    void info();
    file(string title, int diskSize, bool alreadyExists);
    int createFile(const char *title, const char *srcFile);
    int readFile(const char *title);
    int deleteFile(const char *title);
    int undeleteFile(const char *title);
};
#endif