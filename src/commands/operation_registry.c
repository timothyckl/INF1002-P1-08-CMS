#include "commands/command.h"
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
};

static const size_t operation_count =
    sizeof(operations) / sizeof(operations[0]);

OpStatus execute_operation(Operation op, StudentDatabase *db) {
  // handle EXIT operation specially
  if (op == EXIT) {
    printf("Goodbye!\n");
    return OP_SUCCESS;
  }

  // find and execute the operation
  for (size_t i = 0; i < operation_count; i++) {
    if (operations[i].op == op) {
      return operations[i].func(db);
    }
  }

  // operation not found in registry
  printf("CMS: Invalid operation\n");
  return OP_ERROR_INVALID;
}
