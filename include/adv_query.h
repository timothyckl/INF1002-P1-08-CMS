#ifndef ADV_QUERY_H
#define ADV_QUERY_H

#include "database.h"

// status codes for advanced query pipeline execution
typedef enum {
  ADV_QUERY_SUCCESS = 0,            // operation completed successfully
  ADV_QUERY_ERROR_INVALID_ARGUMENT, // invalid argument provided
  ADV_QUERY_ERROR_EMPTY_DATABASE,   // database is empty
  ADV_QUERY_ERROR_PARSE,            // failed to parse query
  ADV_QUERY_ERROR_MEMORY            // memory allocation failed
} AdvQueryStatus;

AdvQueryStatus adv_query_execute(StudentDatabase *db, const char *pipeline);
const char *adv_query_status_string(AdvQueryStatus status);
AdvQueryStatus adv_query_run_prompt(StudentDatabase *db);

#endif
