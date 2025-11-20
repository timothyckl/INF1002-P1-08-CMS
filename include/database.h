#ifndef DATABASE_H
#define DATABASE_H

/*
 * database module
 *
 * manages student records organised in tables within a database structure.
 * uses dynamic arrays that grow automatically as needed (doubling capacity).
 * supports loading from and saving to text files, along with basic
 * operations like adding and removing records.
 */

#include <stdbool.h>
#include <stddef.h>

// forward declaration to avoid circular dependency
typedef struct EventLog EventLog;

// capacity constants
#define INITIAL_TABLE_CAPACITY 2
#define INITIAL_RECORD_CAPACITY 10
#define MAX_FILE_PATH 260

// operation status codes
typedef enum {
  DB_SUCCESS = 0,          // operation succeeded
  DB_ERROR_NULL_POINTER,   // null pointer passed to function
  DB_ERROR_MEMORY,         // malloc/realloc failed
  DB_ERROR_FILE_NOT_FOUND, // cannot open file
  DB_ERROR_FILE_READ,      // error reading from file
  DB_ERROR_DUPLICATE_ID,   // duplicate student ID (for future insert)
  DB_ERROR_NOT_FOUND,      // record not found (for future query/update/delete)
  DB_ERROR_INVALID_DATA    // invalid data format or values
} DBStatus;

// our struct is defined here because out parser functions will be making use of
// it
typedef struct {
  int id;
  char name[50];
  char prog[50];
  float mark;
} StudentRecord;

// table container for column headers and records
typedef struct {
  char table_name[50]; // name of this table

  // column headers
  char **column_headers; // array of header strings
  size_t column_count;   // number of columns

  // record storage (dynamic array)
  StudentRecord *records; // heap-allocated array
  size_t record_count;    // current number of records
  size_t record_capacity; // allocated capacity for records
} StudentTable;

// database container for tables and metadata
typedef struct {
  // database-level metadata
  char db_name[100]; // from "Database Name:" line
  char authors[200]; // from "Authors:" line

  // table storage (dynamic array)
  StudentTable **tables; // array of table pointers
  size_t table_count;    // current number of tables
  size_t table_capacity; // allocated capacity for tables

  // state tracking
  // tracks if database has been loaded from file
  bool is_loaded;

  // where the database was loaded from (for saving)
  char filepath[MAX_FILE_PATH];

  // session event log for tracking operations
  // pointer to event log (NULL until first event or until initialised)
  EventLog *event_log;
} StudentDatabase;

// table lifecycle
// creates a new empty table with the given name
StudentTable *table_init(const char *table_name);

// frees all memory associated with a table
void table_free(StudentTable *table);

// sets column headers for a table (takes ownership of headers array)
DBStatus table_set_column_headers(StudentTable *table, char **headers,
                                  size_t count);

// adds a record to the table (grows capacity if needed)
DBStatus table_add_record(StudentTable *table, StudentRecord *record);

// removes a record from the table by student id
DBStatus table_remove_record(StudentTable *table, int student_id);

// database lifecycle
// creates a new empty database
StudentDatabase *db_init(void);

// frees all memory associated with a database
void db_free(StudentDatabase *db);

// adds a table to the database (grows capacity if needed)
DBStatus db_add_table(StudentDatabase *db, StudentTable *table);

// file operations
// loads database from a text file
DBStatus db_load(StudentDatabase *db, const char *filename);

// saves database to a text file
DBStatus db_save(StudentDatabase *db, const char *filename);

// helper to convert status to string (for error messages)
const char *db_status_string(DBStatus status);

// updates a student record by id (only non-null fields will be updated)
DBStatus db_update_record(StudentDatabase *db, int id, const char *new_name,
                          const char *new_prog, const float *new_mark);

#endif
