#include "database.h"
#include "parser.h"
#include <stdio.h>
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
 * remove record from table by student id
 * returns: DB_SUCCESS on success, DB_ERROR_NOT_FOUND if id not found, error
 * code on failure
 */
DBStatus table_remove_record(StudentTable *table, int student_id) {
  if (!table) {
    return DB_ERROR_NULL_POINTER;
  }

  // search for record with matching id using sentinel value pattern
  size_t deleted_index = (size_t)-1;
  for (size_t i = 0; i < table->record_count; i++) {
    if (table->records[i].id == student_id) {
      deleted_index = i;
      break;
    }
  }

  // check if record was found
  if (deleted_index == (size_t)-1) {
    return DB_ERROR_NOT_FOUND;
  }

  // delete record using safe array shifting
  // only shift if deleted record is not the last element
  if (deleted_index < table->record_count - 1) {
    memmove(&table->records[deleted_index], &table->records[deleted_index + 1],
            (table->record_count - deleted_index - 1) * sizeof(StudentRecord));
  }

  // decrement record count
  table->record_count--;

  // zero out the last element for safety
  memset(&table->records[table->record_count], 0, sizeof(StudentRecord));

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
  db->is_loaded = false;
  db->filepath[0] = '\0';

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
 * save database to file
 * writes database metadata, table structure, and all records to file
 * returns: DB_SUCCESS on success, error code on failure
 */
DBStatus db_save(StudentDatabase *db, const char *filename) {
  // validate parameters
  if (!db || !filename) {
    return DB_ERROR_NULL_POINTER;
  }

  // validate database has tables
  if (db->table_count == 0) {
    return DB_ERROR_INVALID_DATA;
  }

  // access the first table (by convention, StudentRecords table)
  StudentTable *table = db->tables[0];
  if (!table) {
    return DB_ERROR_NULL_POINTER;
  }

  // validate table has column headers
  if (!table->column_headers || table->column_count == 0) {
    return DB_ERROR_INVALID_DATA;
  }

  // open file for writing
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    return DB_ERROR_FILE_NOT_FOUND;
  }

  // write header metadata
  fprintf(fp, "Database Name: %s\n", db->db_name);
  fprintf(fp, "Authors: %s\n", db->authors);
  fprintf(fp, "\n");
  fprintf(fp, "Table Name: %s\n", table->table_name);

  // write column headers (ID, Name, Programme, Mark)
  for (size_t i = 0; i < table->column_count; i++) {
    fprintf(fp, "%s", table->column_headers[i]);
    if (i + 1 < table->column_count) {
      fputc('\t', fp);
    }
  }
  fputc('\n', fp);

  // write all records
  for (size_t i = 0; i < table->record_count; i++) {
    const StudentRecord *r = &table->records[i];
    fprintf(fp, "%d\t%s\t%s\t%.2f\n", r->id, r->name, r->prog, r->mark);
  }

  // close file
  if (fclose(fp) != 0) {
    return DB_ERROR_FILE_READ;
  }

  return DB_SUCCESS;
}

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
