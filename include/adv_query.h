#ifndef ADV_QUERY_H
#define ADV_QUERY_H

#include "database.h"

// status codes for advanced query pipeline execution
typedef enum {
  ADV_QUERY_OK = 0,
  ADV_QUERY_ERROR_INVALID_ARGUMENT,
  ADV_QUERY_ERROR_EMPTY_DATABASE,
  ADV_QUERY_ERROR_PARSE,
  ADV_QUERY_ERROR_MEMORY
} AdvQueryStatus;

AdvQueryStatus adv_query_execute(StudentDatabase *db, const char *pipeline);
const char *adv_query_status_string(AdvQueryStatus status);
AdvQueryStatus adv_query_run_prompt(StudentDatabase *db);

#endif

