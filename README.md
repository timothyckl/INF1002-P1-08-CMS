# INF1002-P1-08-C

This repository contains our group's project for the Programming Fundamentals module.
The project is implemented in C and focuses on applying key programming concepts such as control flow, data structures, and modular design.

## Overview

A terminal-based Class Management System for managing student records. The system provides core CRUD operations: OPEN, SHOW, INSERT, QUERY, UPDATE, DELETE, and SAVE.

## Getting Started

### Prerequisites
- A C compiler (e.g. gcc or clang)
- `make` installed (optional but recommended)

### Build

To compile the programme:
```bash
make
```

Or manually:
```bash
mkdir -p build
gcc -Iinclude src/*.c src/commands/*.c -o build/main
```

### Run
```bash
./build/main data/P1_8-CMS.txt
```

## Testing

See [tests/README.md](tests/README.md) for comprehensive test documentation.
