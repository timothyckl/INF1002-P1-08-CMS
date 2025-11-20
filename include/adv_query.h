#ifndef ADV_QUERY_H
#define ADV_QUERY_H

/*
 * advanced query module
 *
 * provides a filter-based query system that allows chaining multiple
 * conditions. supports filtering by id, name, programme, and mark with various
 * operators. uses a pipeline syntax where filters are separated by '|'.
 */

#include "database.h"

typedef enum {
  ADV_QUERY_SUCCESS = 0,            // operation completed successfully
  ADV_QUERY_ERROR_INVALID_ARGUMENT, // invalid argument provided
  ADV_QUERY_ERROR_EMPTY_DATABASE,   // database is empty
  ADV_QUERY_ERROR_PARSE,            // failed to parse query
  ADV_QUERY_ERROR_MEMORY            // memory allocation failed
} AdvQueryStatus;

// executes a query pipeline and displays matching records
AdvQueryStatus adv_query_execute(StudentDatabase *db, const char *pipeline);

// converts query status code to readable string
const char *adv_query_status_string(AdvQueryStatus status);

// runs interactive query prompt with guided help
AdvQueryStatus adv_query_run_prompt(StudentDatabase *db);

#endif
