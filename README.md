# My File System (MFS)

My File System (MFS) is a simple file system implementation in C language. It provides basic functionality for managing files and directories within a file system image. This README file provides an overview of the code and instructions on how to use it.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Commands](#commands)
- [License](#license)

## Introduction
MFS is designed to work with file system images that follow the FAT32 file system specification. It allows users to open an image file, navigate through directories, list directory contents, change directories, retrieve files from the file system, read file data, and perform other basic file system operations.

## Features
- Open and close file system image
- Print information about the file system
- List directory contents
- Change directories
- Retrieve files from the file system
- Read file data
- Print file attributes
- Undelete files (experimental)

## Getting Started
To use MFS, follow these steps:

1. Clone the MFS repository from GitHub:
   ```
   git clone https://github.com/username/mfs.git
   ```

2. Compile the code using a C compiler:
   ```
   gcc -o mfs mfs.c
   ```

3. Run the compiled executable:
   ```
   ./mfs
   ```

## Usage
Once you have successfully compiled and executed the MFS program, you can start using the available commands to interact with the file system image. The program will display a prompt "mfs>" where you can enter commands.

Example usage:
```
mfs> open image.img
image.img opened.
mfs> ls
Directory listing:
file1.txt
file2.txt
dir1
dir2
mfs> cd dir1
mfs/dir1> ls
Directory listing:
file3.txt
file4.txt
mfs/dir1> get file3.txt
file3.txt retrieved successfully.
mfs/dir1> read file4.txt 0 256
Content of file4.txt (256 bytes):
...
mfs/dir1> cd ..
mfs> close
File system image closed.
mfs> exit
```

## Commands
The following commands are available in the MFS program:

- `open <image>`: Open a file system image.
- `info`: Print information about the file system.
- `ls`: List directory contents.
- `cd <directory>`: Change to the specified directory.
- `get <file>`: Retrieve a file from the file system.
- `read <file> <position> <bytes>`: Read a specified number of bytes from a file.
- `stat <file>`: Print file attributes.
- `close`: Close the file system image.
- `exit`: Exit the MFS program.

## License
This project is licensed under the [MIT License](LICENSE). Feel free to use, modify, and distribute the code according to the terms of the license.
