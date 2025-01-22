#include"File.h"

std::vector<std::string> split(const std::string &s, const char delim) {
    /*
    castective: to split a string by given delimitter
    input:
        s: given string
        delim: delimitter
    return: vector of strings split at given delimitter
    */
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

void createBuffer(char temp, char* buf) {
    // creates a buffer based on a char
    if (temp == '.') {
        std::cout << "Enter alphabet: ";
        std::cin >> temp;
        std::cin.ignore(32767, '\n');
    }
    for (int i = 0; i < kSectorSize; i++) {
        buf[i] = temp;
    }
}

int handleInput(file &disk) {
    /*
    objective: to create a terminal like CLI to handle disk operations
    input: reference to disk
    return:
        0: continue
        1: exit
    */
    std::string command;
    std::cout << KGRN << "(myFileSystem)> " << KRST;
    getline(std::cin, command);
    if (command.empty()) return 0;

    // split command into function and argument part
    std::vector<std::string> tokens;
    tokens = split(command, ' ');
    std::string fun = tokens[0];
    std::string arg = "";
    if (tokens.size() > 1) {
        arg = tokens[1];
    }

    std::string arg2 = "";
    if (tokens.size() > 2) {
        arg2 = tokens[2];
    }

    enum Choice {
        HELP, INFO,READ, CHECK, WRITE, FORMAT, EXIT, NEW_FILE, CAT, DEL, UNDEL, MKDIR, CD, PWD, LS, END
    };
    std::string CHOICES[] = {
        "help", "info","read", "check", "write", "format", "exit", "new", "cat", "del", "undel", "mkdir", "chdir", "pwd", "list", ""
    };
    std::string choices[] = {
        "h", "info", "r", "c", "w", "f", "q", "n", "cat", "del", "undel", "mkdir", "cd", "pwd", "ls", ""
    };

    Choice selected = END;
    for (int i = 0; i < END; ++i) {
        if (CHOICES[i] == fun || choices[i] == fun) {
            selected = static_cast<Choice>(i);
            break;
        }
    }

    int sector;
    // handle command
    switch (selected) {
        case READ: {
            if (arg.empty()) {
                std::cout << "usage: read SECTOR" << std::endl;
                return 0;
            }
            // convert string to int
            std::istringstream(arg) >> sector;
            char readBuffer[kSectorSize];
            disk.readSector(sector, readBuffer);
            for (int i = 0; i < kSectorSize; ++i) {
                std::cout << readBuffer[i];
            }
            std::cout << std::endl;
            break;
        }
        case WRITE: {
            if (arg.empty()) {
                std::cout << "usage: write SECTOR" << std::endl;
                return 0;
            }
            std::istringstream(arg) >> sector;
            if (disk.getStatus(sector) == FileSystem::BUSY) {
                std::cout << "Overwrite sector data (y/n)? _\b";
                char t;
                std::cin >> t;
                if (t == 'y' || t == 'Y') {
                    char buf[kSectorSize];
                    createBuffer('.', buf);
                    disk.writeSector(sector, buf);
                } else {
                    std::cin.ignore(32767, '\n');
                    return 0;
                }
            } else {
                char buf[kSectorSize];
                createBuffer('.', buf);
                disk.writeSector(sector, buf);
                disk.updateStatus(sector, 1);
            }
            break;
        }
        case CHECK: {
            if (arg.empty()) {
                std::cout << "usage: check SECTOR" << std::endl;
                return 0;
            }
            std::istringstream(arg) >> sector;
            int status = disk.getStatus(sector);
            std::cout << "sector #" << sector << " is ";
            if (status < disk.END && status > -1) {
                std::cout << disk.statusMsg[status] << std::endl;
            } else {
                std::cout << "pointing to sector #" << status << std::endl;
            }
            break;
        }
        case FORMAT:
            disk.format();
            break;
        case HELP: {
            std::ifstream helpFile;
            helpFile.open("help.txt", std::ios::in | std::ios::ate);
            int helpFileSize = helpFile.tellg();
            char* helpMsg = new char[helpFileSize];
            helpFile.seekg(0);
            helpFile.read(helpMsg, helpFileSize);
            helpFile.close();
            std::cout << helpMsg << std::endl;
            delete[] helpMsg;
            break;
        }
        case INFO:
            disk.info();
            break;
        case NEW_FILE: {
            if (arg.empty()) {
                std::cout << "usage: new FILE_NAME [SRC_FILE]" << std::endl;
                return 0;
            }
            disk.createFile(arg.c_str(), arg2.c_str());
            break;
        }
        case CAT: {
            if (arg.empty()) {
                std::cout << "usage: cat FILE_NAME" << std::endl;
                return 0;
            }
            disk.readFile(arg.c_str());
            break;
        }
        case DEL: {
            if (arg.empty()) {
                std::cout << "usage: del FILE_NAME" << std::endl;
                return 0;
            }
            disk.deleteFile(arg.c_str());
            break;
        }
        case UNDEL: {
            if (arg.empty()) {
                std::cout << "usage: undel FILE_NAME" << std::endl;
                return 0;
            }
            disk.undeleteFile(arg.c_str());
            break;
        }
        case MKDIR: {
            if (arg.empty()) {
                std::cout << "usage: mkdir DIR_NAME" << std::endl;
                return 0;
            }
            disk.createDir(arg.c_str());
            break;
        }
        case CD: {
            if (arg.empty()) {
                std::cout << "usage: cd DIR_NAME" << std::endl;
                return 0;
            }
            std::vector<std::string> dir = split(arg, '/');
            for (int i = 0; i < dir.size(); ++i) {
                disk.changeDir(dir[i].c_str());
            }
            break;
        }
        case PWD:
            disk.printWorkingDir();
            break;
        case LS:
            disk.listDirectoryContents();
            break;
        case EXIT:
            return 1;
        default:
            std::cout << "(myFileSystem): command not found: " << fun << std::endl;
            return 0;
    }
    return 0;
}

int main(int argc, char const *argv[]) {
    std::string diskTitle;
    int diskSizeBytes;

    // get disk title
    if (argc > 1 && argv[1]) {
        diskTitle = argv[1];
    } else {
        std::cout << "Enter path to disk: ";
        std::cin >> diskTitle;
    }

    // check if disk doesn't exists or is empty
    //  and if exists, get it's size in bytes
    bool alreadyExists = false;
    std::fstream fin;
    fin.open(diskTitle, std::ios::binary | std::ios::in | std::ios::ate);
    diskSizeBytes = fin.tellg();
    fin.close();
    alreadyExists = (diskSizeBytes > -1);

    // get size if not exists
    if (!alreadyExists) {
        int diskSizeMB;
        if (argc > 2) {
            std::istringstream iss(argv[2]);
            iss >> diskSizeMB;
        } else {
            std::cout << "Enter disk size in MiB : ";
            do{
                std::cin >> diskSizeMB;
            }while(diskSizeMB<=0);
            std::cin.ignore(32767, '\n');
        }
        diskSizeBytes = kOneMiB_g*diskSizeMB;
    }

    // initialize disk
    file disk(diskTitle, diskSizeBytes, alreadyExists);
    std::cout << "Type `help` to see available commands." << std::endl;

    int response = handleInput(disk);
    while (response != 1) {
        response = handleInput(disk);
    }

    return 0;
}