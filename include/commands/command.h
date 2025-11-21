#ifndef COMMAND_H
#define COMMAND_H

#include "database.h"

// operation types for cms operations
typedef enum {
  EXIT = 0,
  OPEN,
  SHOW_ALL,
  INSERT,
  QUERY,
  UPDATE,
  DELETE,
  SAVE,
  SORT,
  ADV_QUERY,
  STATISTICS,
  SHOW_LOG,
  CHECKSUM,
} Operation;

// operation status codes for internal cms operations
typedef enum {
  OP_SUCCESS = 0,         // operation completed successfully
  OP_ERROR_OPEN,          // failed to open file
  OP_ERROR_INPUT,         // input reading failed
  OP_ERROR_VALIDATION,    // validation failed
  OP_ERROR_DB_NOT_LOADED, // database not loaded
  OP_ERROR_GENERAL,       // general operation error
  OP_ERROR_INVALID        // invalid operation
} OpStatus;

// function pointer type for command execution
typedef OpStatus (*CommandFunc)(StudentDatabase *db);

// operation registry functions
OpStatus execute_operation(Operation op, StudentDatabase *db);

// individual command functions
OpStatus execute_open(StudentDatabase *db);
OpStatus execute_show_all(StudentDatabase *db);
OpStatus execute_insert(StudentDatabase *db);
OpStatus execute_query(StudentDatabase *db);
OpStatus execute_update(StudentDatabase *db);
OpStatus execute_delete(StudentDatabase *db);
OpStatus execute_save(StudentDatabase *db);
OpStatus execute_sort(StudentDatabase *db);
OpStatus execute_adv_query(StudentDatabase *db);
OpStatus execute_statistics(StudentDatabase *db);
OpStatus execute_show_log(StudentDatabase *db);
OpStatus execute_checksum(StudentDatabase *db);

#endif // COMMAND_H
