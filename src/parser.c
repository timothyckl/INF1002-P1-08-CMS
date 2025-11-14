#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: figure out magic numbers (anyhow chose so will need to justify)
// TODO: handle unexpected metadata and table data

/*
 * validate a student record
 * returns: VALID_RECORD if valid, error code otherwise
 */
ValidationStatus validate_record(const StudentRecord *record) {
  if (!record) {
    return INVALID_FIELD_COUNT;
  }

  // validate ID range
  if (record->id < 0 || record->id > 9999999) {
    return INVALID_ID_RANGE;
  }

  // validate mark range
  if (record->mark < 0.0f || record->mark > 100.0f) {
    return INVALID_MARK_RANGE;
  }

  // validate non-empty name
  if (strlen(record->name) == 0) {
    return INVALID_EMPTY_NAME;
  }

  // validate non-empty programme
  if (strlen(record->prog) == 0) {
    return INVALID_EMPTY_PROGRAMME;
  }

  return VALID_RECORD;
}

/*
 * convert validation error to human-readable string
 * returns: string description of validation error
 */
const char *validation_error_string(ValidationStatus error) {
  switch (error) {
  case VALID_RECORD:
    return "valid record";
  case INVALID_ID_RANGE:
    return "ID out of range";
  case INVALID_MARK_RANGE:
    return "mark out of range";
  case INVALID_EMPTY_NAME:
    return "empty name field";
  case INVALID_EMPTY_PROGRAMME:
    return "empty programme field";
  case INVALID_FIELD_COUNT:
    return "invalid field count";
  default:
    return "unknown validation error";
  }
}

/*
 * convert parse status to human-readable string
 * returns: string description of parse status
 */
const char *parse_status_string(ParseStatus status) {
  switch (status) {
  case PARSE_SUCCESS:
    return "parse success";
  case PARSE_ERROR_FORMAT:
    return "invalid format";
  case PARSE_ERROR_EMPTY:
    return "empty line";
  case PARSE_ERROR_INCOMPLETE:
    return "incomplete data";
  default:
    return "unknown parse error";
  }
}

/*
 * parse metadata line in format "Key: value"
 * returns: PARSE_SUCCESS on success, error code on failure
 */
ParseStatus parse_metadata(const char *line, char *key, char *value) {
  if (!line || !key || !value) {
    return PARSE_ERROR_FORMAT;
  }

  // find colon separator
  char *colon = strchr(line, ':');
  if (!colon)
    return PARSE_ERROR_FORMAT;

  // extract key (before colon)
  size_t key_len = colon - line;
  strncpy(key, line, key_len);
  key[key_len] = '\0';

  // extract value (after colon + space)
  char *value_start = colon + 1;
  while (*value_start == ' ' || *value_start == '\t') {
    value_start++; // skip whitespace
  }

  if (*value_start == '\0' || *value_start == '\n') {
    return PARSE_ERROR_EMPTY;
  }

  // copy value and remove trailing newline
  strcpy(value, value_start);
  size_t value_len = strlen(value);
  if (value_len > 0 && value[value_len - 1] == '\n') {
    value[value_len - 1] = '\0';
  }

  return PARSE_SUCCESS;
}

/*
 * parse data record line (tab-separated fields)
 * returns: PARSE_SUCCESS on success, error code on failure
 */
ParseStatus parse_record_line(const char *line, StudentRecord *record) {
  if (!line || !record) {
    return PARSE_ERROR_FORMAT;
  }

  char line_copy[256];
  strncpy(line_copy, line, sizeof(line_copy) - 1);
  line_copy[sizeof(line_copy) - 1] = '\0';

  // remove trailing newline
  size_t len = strlen(line_copy);
  if (len > 0 && line_copy[len - 1] == '\n') {
    line_copy[len - 1] = '\0';
  }

  // check for empty line
  if (len == 0 || line_copy[0] == '\0') {
    return PARSE_ERROR_EMPTY;
  }

  // tokenise by tab
  char *token = strtok(line_copy, "\t");
  if (!token) {
    return PARSE_ERROR_INCOMPLETE;
  }
  record->id = atoi(token);

  token = strtok(NULL, "\t");
  if (!token) {
    return PARSE_ERROR_INCOMPLETE;
  }
  strncpy(record->name, token, sizeof(record->name) - 1);
  record->name[sizeof(record->name) - 1] = '\0';

  token = strtok(NULL, "\t");
  if (!token) {
    return PARSE_ERROR_INCOMPLETE;
  }
  strncpy(record->prog, token, sizeof(record->prog) - 1);
  record->prog[sizeof(record->prog) - 1] = '\0';

  token = strtok(NULL, "\t");
  if (!token) {
    return PARSE_ERROR_INCOMPLETE;
  }
  record->mark = atof(token);

  return PARSE_SUCCESS;
}

/*
 * parse column header line (tab-separated)
 * returns: PARSE_SUCCESS on success, error code on failure
 */
ParseStatus parse_column_headers(const char *line, char ***headers,
                                 size_t *count) {
  if (!line || !headers || !count) {
    return PARSE_ERROR_FORMAT;
  }

  char line_copy[512];
  strncpy(line_copy, line, sizeof(line_copy) - 1);
  line_copy[sizeof(line_copy) - 1] = '\0';

  // remove newline
  size_t len = strlen(line_copy);
  if (len > 0 && line_copy[len - 1] == '\n') {
    line_copy[len - 1] = '\0';
  }

  // first pass: count columns
  char *temp_copy = strdup(line_copy);
  if (!temp_copy) {
    return PARSE_ERROR_FORMAT;
  }

  *count = 0;
  char *token = strtok(temp_copy, "\t");
  while (token) {
    (*count)++;
    token = strtok(NULL, "\t");
  }
  free(temp_copy);

  if (*count == 0) {
    return PARSE_ERROR_EMPTY;
  }

  // allocate header array
  *headers = malloc(*count * sizeof(char *));
  if (!*headers)
    return PARSE_ERROR_FORMAT;

  // second pass: copy header strings
  size_t idx = 0;
  token = strtok(line_copy, "\t");
  while (token && idx < *count) {
    (*headers)[idx] = strdup(token);
    if (!(*headers)[idx]) {
      // cleanup on failure
      for (size_t i = 0; i < idx; i++) {
        free((*headers)[i]);
      }
      free(*headers);
      return PARSE_ERROR_FORMAT;
    }
    idx++;
    token = strtok(NULL, "\t");
  }

  return PARSE_SUCCESS;
}

/*
 * parse entire file into database with state machine
 * returns: DB_SUCCESS on success, error code on failure
 */
DBStatus parse_file(const char *filename, StudentDatabase *db) {
  if (!filename || !db) {
    return DB_ERROR_NULL_POINTER;
  }

  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Error: Cannot open file %s\n", filename);
    return DB_ERROR_FILE_NOT_FOUND;
  }

  char line[512];
  int line_num = 0;
  StudentTable *current_table = NULL;
  int awaiting_headers = 0; // flag: next line is column headers

  while (fgets(line, sizeof(line), fp)) {
    line_num++;

    // skip empty lines
    if (line[0] == '\n' || line[0] == '\r' ||
        (line[0] == ' ' && line[1] == '\n')) {
      continue;
    }

    // parse database metadata
    if (strstr(line, "Database Name:")) {
      char key[50], value[100];
      if (parse_metadata(line, key, value) == PARSE_SUCCESS) {
        strncpy(db->db_name, value, sizeof(db->db_name) - 1);
        db->db_name[sizeof(db->db_name) - 1] = '\0';
      }
    } else if (strstr(line, "Authors:")) {
      char key[50], value[200];
      if (parse_metadata(line, key, value) == PARSE_SUCCESS) {
        strncpy(db->authors, value, sizeof(db->authors) - 1);
        db->authors[sizeof(db->authors) - 1] = '\0';
      }
    } else if (strstr(line, "Table Name:")) {
      // create new table
      char key[50], value[50];
      if (parse_metadata(line, key, value) == PARSE_SUCCESS) {
        current_table = table_init(value);
        if (!current_table) {
          fclose(fp);
          return DB_ERROR_MEMORY;
        }

        DBStatus add_status = db_add_table(db, current_table);
        if (add_status != DB_SUCCESS) {
          table_free(current_table);
          fclose(fp);
          return add_status;
        }

        awaiting_headers = 1; // next line should be headers
      }
    } else if (awaiting_headers && current_table) {
      // parse column headers
      char **headers;
      size_t count;
      ParseStatus status = parse_column_headers(line, &headers, &count);

      if (status == PARSE_SUCCESS) {
        table_set_column_headers(current_table, headers, count);
        awaiting_headers = 0; // now we can parse records
      } else {
        fprintf(stderr, "Warning: Failed to parse column headers at line %d\n",
                line_num);
        awaiting_headers = 0; // skip to records anyway
      }
    } else if (current_table && !awaiting_headers) {
      // parse data record
      StudentRecord record;
      ParseStatus parse_status = parse_record_line(line, &record);

      if (parse_status == PARSE_SUCCESS) {
        ValidationStatus validation = validate_record(&record);

        if (validation == VALID_RECORD) {
          DBStatus add_status = table_add_record(current_table, &record);
          if (add_status != DB_SUCCESS) {
            fclose(fp);
            return add_status;
          }
        } else {
          fprintf(stderr, "Warning: %s at line %d\n",
                  validation_error_string(validation), line_num);
        }
      } else if (parse_status != PARSE_ERROR_EMPTY) {
        fprintf(stderr, "Warning: %s at line %d\n",
                parse_status_string(parse_status), line_num);
      }
    }
  }

  fclose(fp);
  return DB_SUCCESS;
}
