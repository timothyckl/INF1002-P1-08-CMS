#include "statistics.h"
#include "database.h"
#include <string.h>

/*
 * calculates summary statistics for all student records in a table
 *
 * computes total count, average mark, highest/lowest marks with student details
 * uses double precision for accumulation to prevent overflow with large
 * datasets
 *
 * note: when multiple students share the same highest or lowest
 * mark, the first occurrence in the table is reported
 */
DBStatus calculate_statistics(StudentTable *table, StudentStatistics *stats) {
  // defensive null pointer checks
  if (!table || !stats) {
    return DB_ERROR_NULL_POINTER;
  }

  // validate table has records
  if (table->record_count == 0) {
    return DB_ERROR_INVALID_DATA;
  }

  // validate records array exists
  if (!table->records) {
    return DB_ERROR_INVALID_DATA;
  }

  // initialise total count
  stats->total_count = table->record_count;

  // initialise with first record's values
  // this ensures first occurrence is selected in case of ties
  stats->highest_mark = table->records[0].mark;
  stats->lowest_mark = table->records[0].mark;
  stats->highest_student_id = table->records[0].id;
  stats->lowest_student_id = table->records[0].id;

  // safely copy names with explicit null termination
  strncpy(stats->highest_student_name, table->records[0].name, 49);
  stats->highest_student_name[49] = '\0';
  strncpy(stats->lowest_student_name, table->records[0].name, 49);
  stats->lowest_student_name[49] = '\0';

  // use double for accumulation to prevent overflow with large datasets
  double sum = (double)table->records[0].mark;

  // iterate through remaining records to find max, min, and accumulate sum
  for (size_t i = 1; i < table->record_count; i++) {
    float current_mark = table->records[i].mark;

    // accumulate sum for average calculation
    sum += (double)current_mark;

    // update highest if strictly greater (first occurrence for ties)
    if (current_mark > stats->highest_mark) {
      stats->highest_mark = current_mark;
      stats->highest_student_id = table->records[i].id;
      strncpy(stats->highest_student_name, table->records[i].name, 49);
      stats->highest_student_name[49] = '\0';
    }

    // update lowest if strictly less (first occurrence for ties)
    if (current_mark < stats->lowest_mark) {
      stats->lowest_mark = current_mark;
      stats->lowest_student_id = table->records[i].id;
      strncpy(stats->lowest_student_name, table->records[i].name, 49);
      stats->lowest_student_name[49] = '\0';
    }
  }

  // calculate average mark
  stats->average_mark = (float)(sum / table->record_count);

  return DB_SUCCESS;
}
