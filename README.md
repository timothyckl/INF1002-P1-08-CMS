# INF1002-P1-08-C

This repository contains our group’s project for the Programming Fundamentals module.
The project is implemented in C and focuses on applying key programming concepts such as control flow, data structures, and modular design.

## 🧩 Overview
> (Update this section once the project specifications are released.) \
> Briefly describe what the program does, its purpose, and any key features or objectives.

## ⚙️ Getting Started

### Prerequisites
- A C compiler (e.g. gcc or clang)
- `make` installed (optional but recommended)
- Criterion for unit tests (?)

### Build

To compile the program:
```bash
make
```
See [Support Guide](SUPPORT.md) to setup Make.exe in windows enviroment.

or manually:

(Windows users using GCC)
```bash
mkdir -p build
gcc -Iinclude src\*.c -o build\main.exe
```
(MacOS users using Clang.)
```bash
mkdir -p build
clang -Iinclude src/*.c -o build/main.exe
```

### Run
```bash
# Windows
.\build\main.exe data\P1_8-CMS.txt

# macOS/Linux
./build/main.exe data/P1_8-CMS.txt
```

## 🧪 Testing
> (Add more details here when testing setup is ready.)
