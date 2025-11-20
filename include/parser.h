#ifndef PARSER_H
#define PARSER_H

/*
 * parser module
 *
 * handles reading and parsing student database files.
 * processes metadata lines, table headers, and student records.
 * validates data during parsing to ensure correctness.
 */

#include "database.h"

typedef enum {
  VALID_RECORD = 0,        // record is valid
  INVALID_ID_RANGE,        // id outside 0-9999999
  INVALID_MARK_RANGE,      // mark outside 0.0-100.0
  INVALID_EMPTY_NAME,      // name field is empty
  INVALID_EMPTY_PROGRAMME, // programme field is empty
  INVALID_FIELD_COUNT      // incorrect number of fields
} ValidationStatus;

typedef enum {
  PARSE_SUCCESS = 0,     // line parsed successfully
  PARSE_ERROR_FORMAT,    // line format is invalid
  PARSE_ERROR_EMPTY,     // line is empty or whitespace only
  PARSE_ERROR_INCOMPLETE // line missing required fields
} ParseStatus;

// parsing statistics for tracking warnings during file load
typedef struct {
  int total_records_attempted; // total data lines processed
  int records_loaded;          // successfully loaded records
  int records_skipped;         // records skipped due to errors
  int validation_errors;       // count of validation errors
  int parse_errors;            // count of parse format errors
} ParseStatistics;

// parse entire file into database
// if stats is provided, it will be populated with parsing statistics
DBStatus parse_file(const char *filename, StudentDatabase *db,
                    ParseStatistics *stats);

// parse single metadata line
ParseStatus parse_metadata(const char *line, char *key, char *value);

// parse single data record line
ParseStatus parse_record_line(const char *line, StudentRecord *record);

// parse column header line
ParseStatus parse_column_headers(const char *line, char ***headers,
                                 size_t *count);

// validation
ValidationStatus validate_record(const StudentRecord *record);

// helper to convert codes to strings
const char *parse_status_string(ParseStatus status);
const char *validation_error_string(ValidationStatus error);

#endif
