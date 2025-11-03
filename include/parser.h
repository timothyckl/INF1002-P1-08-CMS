#ifndef PARSER_H 
#define PARSER_H

#include <regex.h>
#include <stdbool.h>

#include "database.h"

bool parse_line(const char *line, StudentRecord *out);

#endif
