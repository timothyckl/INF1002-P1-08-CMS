#include "parser.h"

#define DB_NAME_PATTERN "Database Name:\s*(.+)" 
#define AUTHOR_PATTERN "Authors:\s*(.+)"
#define TABLE_PATTERN "Table Name:\s*(.+)"
#define FIELD_PATTERN "[^\t\r\n]+"

// TODO: check if line is empty
// TODO: setup regex pattern
// TODO: extract fields from line 
// TODO: validate types (?)
bool parse_line(const char *line, StudentRecord *out) {

  return true;
}
