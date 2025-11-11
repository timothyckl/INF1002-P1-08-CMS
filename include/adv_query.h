#ifndef ADV_QUERY_H
#define ADV_QUERY_H

#include "database.h"

/*
 * Advanced query module exposes a pipeline executor that can run chained
 * filters, grep-style substring searches, sorting, limiting, and projection on
 * the in-memory StudentDatabase.
 *
 * The caller (CLI) is responsible for parsing the user command up to and
 * excluding the leading keyword (e.g., stripping "ADVQUERY" before calling the
 * executor) and for validating that a database has been opened.
 */

typedef enum {
  ADV_QUERY_OK = 0,
  ADV_QUERY_ERROR_INVALID_ARGUMENT,
  ADV_QUERY_ERROR_EMPTY_DATABASE,
  ADV_QUERY_ERROR_PARSE,
  ADV_QUERY_ERROR_MEMORY
} AdvQueryStatus;

/*
 * Execute an advanced query pipeline.
 * - db: pointer to a loaded StudentDatabase
 * - pipeline: string containing stages separated by '|'
 * Returns status code indicating success or parse/argument errors.
 */
AdvQueryStatus adv_query_execute(StudentDatabase *db, const char *pipeline);

/*
 * Convert advanced query status to human-readable text.
 */
const char *adv_query_status_string(AdvQueryStatus status);

#endif
