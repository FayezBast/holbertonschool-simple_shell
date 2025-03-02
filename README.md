# Simple Shell

## Overview
This project is a simple UNIX command-line interpreter (shell) implemented in C. It reads user input, parses commands, and executes them using system calls.

## Features
- Display a shell prompt (`#fb$ `)
- Read user input and parse commands
- Execute external programs using `execve`
- Handle built-in commands:
  - `exit` : Exit the shell
  - `env` : Print the current environment variables
- Basic error handling
- Works in interactive and non-interactive mode

## Compilation
To compile the shell, use:
```sh
gcc -Wall -Werror -Wextra -pedantic *.c -o hsh
```

## Usage
Run the shell with:
```sh
./hsh
```
Once inside the shell, you can type commands like:
```sh
#fb$ ls -l
#fb$ /bin/pwd
#fb$ env
```
To exit the shell, type:
```sh
#fb$ exit
```

## Built-in Commands
| Command | Description |
|---------|-------------|
| `exit` | Exits the shell |
| `env`  | Prints the environment variables |

## Example Usage
```sh
$ ./hsh
#fb$ ls
file1.c  file2.c  hsh  README.md
#fb$ env
HOME=/home/user
PATH=/usr/bin:/bin:/usr/sbin:/sbin
#fb$ exit
```.

## Authors
- Fayez Bast




