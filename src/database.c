#include "database.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

/*
 * initialise a new table
 * returns: pointer to table on success, NULL on failure
 */
StudentTable *table_init(const char *table_name) {
  StudentTable *table = malloc(sizeof(StudentTable));
  if (!table) {
    return NULL;
  }

  // copy table name
  strncpy(table->table_name, table_name, sizeof(table->table_name) - 1);
  table->table_name[sizeof(table->table_name) - 1] = '\0';

  // initialise column headers
  table->column_headers = NULL;
  table->column_count = 0;

  // allocate initial record capacity
  table->records = malloc(INITIAL_RECORD_CAPACITY * sizeof(StudentRecord));
  if (!table->records) {
    free(table);
    return NULL;
  }

  table->record_count = 0;
  table->record_capacity = INITIAL_RECORD_CAPACITY;

  return table;
}

/*
 * free all memory associated with table
 */
void table_free(StudentTable *table) {
  if (!table) {
    return;
  }

  // free column headers
  for (size_t i = 0; i < table->column_count; i++) {
    free(table->column_headers[i]);
  }
  free(table->column_headers);

  // free records
  free(table->records);
  free(table);
}

/*
 * set column headers for table
 * returns: DB_SUCCESS on success, error code on failure
 */
DBStatus table_set_column_headers(StudentTable *table, char **headers,
                                  size_t count) {
  if (!table || !headers) {
    return DB_ERROR_NULL_POINTER;
  }

  // free existing headers if any
  for (size_t i = 0; i < table->column_count; i++) {
    free(table->column_headers[i]);
  }
  free(table->column_headers);

  // store new headers
  table->column_headers = headers;
  table->column_count = count;

  return DB_SUCCESS;
}

/*
 * add record to table with capacity growth
 * returns: DB_SUCCESS on success, error code on failure
 */
DBStatus table_add_record(StudentTable *table, StudentRecord *record) {
  if (!table || !record) {
    return DB_ERROR_NULL_POINTER;
  }

  // grow capacity if needed
  if (table->record_count >= table->record_capacity) {
    size_t new_capacity = table->record_capacity * 2;
    StudentRecord *temp =
        realloc(table->records, new_capacity * sizeof(StudentRecord));

    if (!temp) {
      return DB_ERROR_MEMORY;
    }

    table->records = temp;
    table->record_capacity = new_capacity;
  }

  // add record
  table->records[table->record_count] = *record;
  table->record_count++;

  return DB_SUCCESS;
}

/*
 * initialise a new empty database
 * returns: pointer to database on success, NULL on failure
 */
StudentDatabase *db_init(void) {
  StudentDatabase *db = malloc(sizeof(StudentDatabase));
  if (!db) {
    return NULL;
  }

  memset(db->db_name, 0, sizeof(db->db_name));
  memset(db->authors, 0, sizeof(db->authors));

  // allocate initial table capacity
  db->tables = malloc(INITIAL_TABLE_CAPACITY * sizeof(StudentTable *));
  if (!db->tables) {
    free(db);
    return NULL;
  }

  db->table_count = 0;
  db->table_capacity = INITIAL_TABLE_CAPACITY;

  return db;
}

/*
 * free all memory associated with database
 */
void db_free(StudentDatabase *db) {
  if (!db) {
    return;
  }

  // free all tables
  for (size_t i = 0; i < db->table_count; i++) {
    table_free(db->tables[i]);
  }
  free(db->tables);
  free(db);
}

/*
 * add table to database with capacity growth
 * returns: DB_SUCCESS on success, error code on failure
 */
DBStatus db_add_table(StudentDatabase *db, StudentTable *table) {
  if (!db || !table) {
    return DB_ERROR_NULL_POINTER;
  }

  // grow capacity if needed
  if (db->table_count >= db->table_capacity) {
    size_t new_capacity = db->table_capacity * 2;
    StudentTable **temp =
        realloc(db->tables, new_capacity * sizeof(StudentTable *));

    if (!temp) {
      return DB_ERROR_MEMORY;
    }

    db->tables = temp;
    db->table_capacity = new_capacity;
  }

  // add table
  db->tables[db->table_count] = table;
  db->table_count++;

  return DB_SUCCESS;
}

/*
 * load database from file
 * returns: DB_SUCCESS on success, error code on failure
 */
DBStatus db_load(StudentDatabase *db, const char *filename) {
  if (!db || !filename) {
    return DB_ERROR_NULL_POINTER;
  }
  return parse_file(filename, db);
}

/*
 * find student record by ID
 * returns: pointer to record on success, NULL if not found or invalid args
 */
StudentRecord *db_find_record_by_id(StudentDatabase *db, int id) {
  if (!db) {
    return NULL;
  }

  for (size_t t = 0; t < db->table_count; t++) {
    StudentTable *table = db->tables[t];
    if (!table) {
      continue;
    }

    for (size_t r = 0; r < table->record_count; r++) {
      StudentRecord *record = &table->records[r];
      if (record->id == id) {
        return record;
      }
    }
  }

  return NULL;
}

/*
 * update a student record by ID
 * - db: loaded StudentDatabase
 * - id: student ID to search for
 * - new_name / new_prog / new_mark:
 *      pass NULL to leave a field unchanged
 *      pass a pointer (or string) to update that field
 *
 * Returns:
 *   DB_SUCCESS on success
 *   DB_ERROR_NOT_FOUND if no record with that ID exists
 *   DB_ERROR_INVALID_DATA if the updated record fails validation
 *   DB_ERROR_NULL_POINTER if db is NULL
 */
DBStatus db_update_record(
  StudentDatabase *db,
  int id,
  const char *new_name,
  const char *new_prog,
  const float *new_mark) {
    
    if (!db) {
      return DB_ERROR_NULL_POINTER;
    }

    // locate the record using existing helper
    StudentRecord *record = db_find_record_by_id(db, id);
    if (!record) {
      return DB_ERROR_NOT_FOUND;
    }

    // make a copy so we can validate safely before committing
    StudentRecord updated = *record;

    if (new_name) {
      // copy at most sizeof(name) - 1 chars and null-terminate
      strncpy(updated.name, new_name, sizeof(updated.name) - 1);
      updated.name[sizeof(updated.name) - 1] = '\0';
    }

    if (new_prog) {
      strncpy(updated.prog, new_prog, sizeof(updated.prog) - 1);
      updated.prog[sizeof(updated.prog) - 1] = '\0';
    }

    if (new_mark) {
      updated.mark = *new_mark;
    }

    // validate using existing parser/validator
    ValidationError v = validate_record(&updated);
    if (v != VALID_RECORD) {
      return DB_ERROR_INVALID_DATA;
    }

  // commit the changes now that they're known to be valid
  *record = updated;
  return DB_SUCCESS;
}


//   if (!db_loaded) {
//     print "open first"
//     return
//   }
//   prompt "> Enter student ID: "
//   read line
//   if (invalid or out of range) {
//     print "invalid ID"
//     return
//   }
//   record = find_record_by_id(db, id)
//   if (!record) {
//     print "No record found"
//   } else {
//     print_record(record)
//   }
//

/*
 * convert status code to human-readable string
 * returns: string description of status
 */
const char *db_status_string(DBStatus status) {
  switch (status) {
  case DB_SUCCESS:
    return "success";
  case DB_ERROR_NULL_POINTER:
    return "null pointer error";
  case DB_ERROR_MEMORY:
    return "memory allocation failed";
  case DB_ERROR_FILE_NOT_FOUND:
    return "file not found";
  case DB_ERROR_FILE_READ:
    return "file read error";
  case DB_ERROR_DUPLICATE_ID:
    return "duplicate ID";
  case DB_ERROR_NOT_FOUND:
    return "record not found";
  case DB_ERROR_INVALID_DATA:
    return "invalid data";
  default:
    return "unknown error";
  }
}
