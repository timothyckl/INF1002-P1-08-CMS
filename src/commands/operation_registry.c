#include "commands/command.h"
#include "event_log.h"
#include <stdio.h>

// operation entry mapping operation type to execution function
typedef struct {
  Operation op;
  CommandFunc func;
  const char *name;
} OperationEntry;

// registry of all available operations
static const OperationEntry operations[] = {
    {OPEN, execute_open, "open"},
    {SHOW_ALL, execute_show_all, "show_all"},
    {INSERT, execute_insert, "insert"},
    {QUERY, execute_query, "query"},
    {UPDATE, execute_update, "update"},
    {DELETE, execute_delete, "delete"},
    {SAVE, execute_save, "save"},
    {SORT, execute_sort, "sort"},
    {ADV_QUERY, execute_adv_query, "adv_query"},
    {STATISTICS, execute_statistics, "statistics"},
    {SHOW_LOG, execute_show_log, "show_log"},
};

static const size_t operation_count =
    sizeof(operations) / sizeof(operations[0]);

/*
 * determines if an operation should be logged
 *
 * excludes display-only operations and special operations
 * view operations (SHOW_ALL, STATISTICS, SHOW_LOG) are not logged
 * EXIT is not logged (session terminator)
 */
static bool should_log_operation(Operation op) {
  return (op != EXIT && op != SHOW_ALL && op != STATISTICS && op != SHOW_LOG);
}

OpStatus execute_operation(Operation op, StudentDatabase *db) {
  // handle EXIT operation specially
  if (op == EXIT) {
    printf("Goodbye!\n");
    return OP_SUCCESS;
  }

  // find and execute the operation
  OpStatus result = OP_ERROR_INVALID;
  for (size_t i = 0; i < operation_count; i++) {
    if (operations[i].op == op) {
      result = operations[i].func(db);
      break;
    }
  }

  // if operation not found, report error
  if (result == OP_ERROR_INVALID) {
    printf("CMS: Invalid operation\n");
    return result;
  }

  // log the operation if database exists and operation should be logged
  if (db && should_log_operation(op)) {
    // initialise event log on first use if needed
    if (!db->event_log) {
      db->event_log = event_log_init();
      // if init fails, continue silently (logging is non-critical)
    }

    // log the event (fails silently if log is NULL)
    if (db->event_log) {
      log_event(db->event_log, op, result);
    }
  }

  return result;
}
