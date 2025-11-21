# INF1002-P1-08-CMS

A terminal-based Class Management System for managing student records. \
The system provides core CRUD operations: `OPEN`, `SHOW`, `INSERT`, `QUERY`, `UPDATE`, `DELETE`, and `SAVE`.

## Project Structure

```bash
INF1002-P1-08-C/
├── src/                    # source code
│   ├── main.c
│   ├── cms.c
│   ├── database.c
│   ├── parser.c
│   ├── ui.c
│   ├── sorting.c
│   ├── statistics.c
│   ├── event_log.c
│   ├── adv_query.c
│   ├── checksum.c
│   ├── utils.c
│   └── commands/           # command implementations for all operations
│       ├── operation_registry.c
│       ├── command_utils.c
│       ├── open_command.c
│       ├── show_all_command.c
│       ├── insert_command.c
│       ├── query_command.c
│       ├── update_command.c
│       ├── delete_command.c
│       ├── save_command.c
│       ├── sort_command.c
│       ├── adv_query_command.c
│       ├── statistics_command.c
│       ├── event_log_command.c
│       └── checksum_command.c
├── include/                # header files
│   ├── cms.h
│   ├── database.h
│   ├── parser.h
│   ├── ui.h
│   ├── sorting.h
│   ├── statistics.h
│   ├── event_log.h
│   ├── adv_query.h
│   ├── checksum.h
│   ├── utils.h
│   └── commands/
│       ├── command.h
│       └── command_utils.h
├── tests/                  # test suite
│   ├── test_parser.c
│   ├── test_database.c
│   ├── test_sorting.c
│   ├── test_statistics.c
│   ├── test_event_log.c
│   ├── test_commands.c
│   ├── test_checksum.c
│   ├── test_adv_query.c
│   ├── test_query.c
│   ├── test_utils.h
│   ├── test_utils.c
│   ├── fixtures/           # test data files for validation
│   └── README.md
├── data/                   # database files
│   └── P1_8-CMS.txt
├── assets/                 # UI text files
│   ├── menu.txt
│   └── declaration.txt
├── Makefile
└── README.md
```


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
./build/main
```

## Testing

See [tests/README.md](tests/README.md) for comprehensive test documentation.
