# File System Simulation with PowerShell Interface  

This project implements a simulation of a file system in C++ using a binary file as a virtual disk. It provides a command-line interface through PowerShell, offering functionalities for managing files, directories, and simulating disk operations.

---

## Project Structure  

The project is designed with a hierarchical class structure:  

1. **FileSystem**  
   - Encapsulates the entire system and provides a centralized interface for managing the disk and its contents.  

2. **DiskManager**  
   - Handles low-level operations on the virtual disk, such as reading and writing sectors, formatting, and checking sector statuses.  

3. **Directory**  
   - Represents directories within the file system, allowing creation, deletion, navigation, and listing of directory contents.  

4. **File**  
   - Simulates files, including their creation, deletion, reading, and writing functionality.  

A **driver program (`main.cpp`)** acts as the entry point, integrating all components and parsing user commands to perform the desired operations.

---

## Features  

- **File and Directory Management**  
  - Create, delete, and navigate directories.  
  - Create, delete, read, and write files.  

- **Disk Operations**  
  - Format the virtual disk.  
  - Check the status of disk sectors.  
  - Recover deleted files.  

- **Interactive Command-Line Interface**  
  - Use PowerShell commands to interact with the file system seamlessly.  

---

## Supported Commands  

The following commands are available to interact with the file system:  

| Command                           | Description                                                                 |
|-----------------------------------|-----------------------------------------------------------------------------|
| `help` / `h`                      | Display a list of all supported commands.                                   |
| `read SECTOR` / `r SECTOR`        | Read data from a specific sector.                                           |
| `write SECTOR` / `w SECTOR`       | Write data to a specific sector.                                            |
| `check SECTOR` / `c SECTOR`       | Check the status of a sector (busy, free, reserved, etc.).                  |
| `format` / `f`                    | Reset the disk to its initial state.                                        |
| `new FILE_NAME [SRC_FILE]` / `n`  | Create a new file in the current directory (optionally copying from a source file). |
| `cat FILE_NAME`                   | Display the content of a file.                                              |
| `del FILE_NAME`                   | Delete a file from the directory.                                           |
| `undel FILE_NAME`                 | Attempt to recover a deleted file.                                          |
| `mkdir DIR`                       | Create a new directory in the current working directory.                    |
| `chdir DIR` / `cd DIR`            | Change the current directory to the specified directory.                    |
| `pwd`                             | Display the current working directory path.                                 |
| `list` / `ls`                     | List all files and directories in the current directory.                    |
| `info`                            | Print detailed information about the file system.                           |
| `exit` / `q`                      | Exit the program.                                                           |

---

## Getting Started  

### Prerequisites  
- A C++ compiler (e.g., GCC, Clang, or MSVC).  
- PowerShell for running the interactive command-line interface.  

### Setup  
1. Clone this repository:  
   ```bash
   git clone https://github.com/your-username/file-system-simulation.git
   cd file-system-simulation
   g++ main.cpp FileSystem.cpp DiskManager.cpp Directory.cpp File.cpp -o FileSystemSim
   ./FileSystemSim
