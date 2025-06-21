# C-ShellProgram
![C](https://img.shields.io/badge/Language-C-blue.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Status](https://img.shields.io/badge/Status-Active-green)

This project is a fully functional Unix-like shell implemented in the C programming language. It supports executing both built-in and external commands, parsing user input with quote handling, and basic directory and path operations. It replicates core behavior found in standard shells like Bash

## Features
### 1. Built-in Commands
The shell supports the following built-in commands:
- **echo**: Prints the provided arguments while handling quotes correctly.
- **exit**: Exits the shell when `exit 0` is entered.
- **type**: Identifies whether a command is a built-in shell command or an external command.
- **pwd**: Displays the current working directory.
- **cd**: Changes the working directory, supporting relative and absolute paths, as well as `~` for the home directory.

### 2. Command Execution
- Executes external commands by searching for them in the system's `PATH`.
- Uses `fork()` to create a child process and `execv()` to execute the command.
- Waits for command execution to complete using `waitpid()`.

### 3. Input Parsing with Quote Handling
- Supports both **single (' ')** and **double (" ")** quotes.
- Properly handles escaped characters inside double quotes (e.g., `\"` becomes `"`).
- Tokenizes input while preserving quoted arguments as a single entity.

### 4. Path Resolution
- Uses `getcwd()` to retrieve the current directory for `pwd` and `cd`.
- Searches for executables in `PATH` when executing external commands.
- Uses `access()` to verify if a file is executable.

### 5. Error Handling
- Displays error messages when commands are not found or when invalid arguments are provided.
- Ensures proper error handling for `cd` when dealing with invalid paths.
- Implements memory safety by dynamically allocating necessary buffers.

## Getting Started
### Prerequisites
Ensure you have a GCC compiler installed. You can verify this with:
```sh
gcc --version
```


## Compilation & Execution
### Compile the Shell
```sh
gcc -o my_shell main.c
```
### Run the Shell
```sh
./my_shell
```

## Example Usage
### Running Built-in Commands
```sh
$ echo "Hello, World!"
Hello, World!

$ pwd
/home/user

$ cd Documents
$ pwd
/home/user/Documents

$ type cd
cd is a shell builtin
```
### Running External Commands
```sh
$ ls -l
$ cat file.txt
$ grep "pattern" file.txt
```
### Handling Quotes & Spaces
```sh
$ echo 'Hello   World'
Hello   World

$ echo "Hello \"quoted\" World"
Hello "quoted" World
```
## License

This project is licensed under the MIT License. You are free to use, modify, and distribute it.
