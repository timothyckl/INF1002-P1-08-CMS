#ifndef SORTING_H
#define SORTING_H

#include "database.h"
#include <stddef.h>

// sort field options for student records
typedef enum {
  SORT_FIELD_ID,
  SORT_FIELD_MARK,
} SortField;

// sort order options
typedef enum {
  SORT_ORDER_ASC,
  SORT_ORDER_DESC,
} SortOrder;

// sort student records by specified field and order
// uses stable bubble sort algorithm
// modifies records array in-place
void sort_records(StudentRecord *records, size_t count, SortField field,
                  SortOrder order);

#endif // SORTING_H
