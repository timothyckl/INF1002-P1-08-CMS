#include "event_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * initialises a new event log with initial capacity
 *
 * allocates memory for log structure and entries array
 * fails gracefully by returning NULL on allocation failure
 */
EventLog *event_log_init(void) {
  // allocate log structure
  EventLog *log = malloc(sizeof(EventLog));
  if (!log) {
    return NULL; // allocation failed
  }

  // allocate initial entries array
  log->entries = malloc(EVENT_LOG_INITIAL_CAPACITY * sizeof(EventEntry));
  if (!log->entries) {
    free(log);
    return NULL; // allocation failed
  }

  // initialise counters
  log->count = 0;
  log->capacity = EVENT_LOG_INITIAL_CAPACITY;

  return log;
}

/*
 * frees event log and all associated memory
 *
 * defensive null pointer check prevents crashes
 * frees entries array before freeing log structure
 */
void event_log_free(EventLog *log) {
  if (!log) {
    return; // null-safe: no-op for null pointer
  }

  // free entries array
  if (log->entries) {
    free(log->entries);
  }

  // free log structure
  free(log);
}

/*
 * logs an operation event with automatic capacity management
 *
 * implements growth strategy:
 * 1. doubles capacity when full (up to 1000 max)
 * 2. uses circular buffer when max capacity reached
 * 3. fails silently on allocation errors (non-critical)
 *
 * note: logging infrastructure should never crash application
 */
void log_event(EventLog *log, Operation op, OpStatus status) {
  // defensive null check
  if (!log) {
    return; // silent fail - logging is non-critical
  }

  // determine if we need to grow capacity
  if (log->count < log->capacity) {
    // space available - add entry normally
    size_t idx = log->count;

    log->entries[idx].timestamp = time(NULL);
    log->entries[idx].operation = op;
    log->entries[idx].status = status;
    log->entries[idx].details[0] = '\0'; // empty details (reserved for future)

    log->count++;
  } else if (log->capacity < EVENT_LOG_MAX_CAPACITY) {
    // at capacity but can grow - attempt to double capacity
    size_t new_capacity = log->capacity * 2;

    // cap at maximum
    if (new_capacity > EVENT_LOG_MAX_CAPACITY) {
      new_capacity = EVENT_LOG_MAX_CAPACITY;
    }

    // attempt reallocation
    EventEntry *temp = realloc(log->entries, new_capacity * sizeof(EventEntry));

    if (!temp) {
      return; // silent fail - allocation failed, don't crash
    }

    // reallocation successful - update capacity
    log->entries = temp;
    log->capacity = new_capacity;

    // add entry
    size_t idx = log->count;
    log->entries[idx].timestamp = time(NULL);
    log->entries[idx].operation = op;
    log->entries[idx].status = status;
    log->entries[idx].details[0] = '\0';

    log->count++;
  } else {
    // at maximum capacity - use circular buffer (overwrite oldest)
    size_t idx = log->count % EVENT_LOG_MAX_CAPACITY;

    log->entries[idx].timestamp = time(NULL);
    log->entries[idx].operation = op;
    log->entries[idx].status = status;
    log->entries[idx].details[0] = '\0';

    log->count++; // keep incrementing for display purposes
  }
}

/*
 * converts operation enum to display string
 *
 * provides human-readable operation names for display
 * maintains consistent uppercase naming convention
 */
const char *event_operation_to_string(Operation op) {
  switch (op) {
  case EXIT:
    return "EXIT";
  case OPEN:
    return "OPEN";
  case SHOW_ALL:
    return "SHOW_ALL";
  case INSERT:
    return "INSERT";
  case QUERY:
    return "QUERY";
  case UPDATE:
    return "UPDATE";
  case DELETE:
    return "DELETE";
  case SAVE:
    return "SAVE";
  case SORT:
    return "SORT";
  case ADV_QUERY:
    return "ADV_QUERY";
  case STATISTICS:
    return "STATISTICS";
  default:
    return "UNKNOWN";
  }
}

/*
 * converts operation status to display string
 *
 * provides human-readable status descriptions for display
 * uppercase for consistency with operation names
 */
const char *event_status_to_string(OpStatus status) {
  switch (status) {
  case OP_SUCCESS:
    return "SUCCESS";
  case OP_ERROR_OPEN:
    return "ERROR_OPEN";
  case OP_ERROR_INPUT:
    return "ERROR_INPUT";
  case OP_ERROR_VALIDATION:
    return "ERROR_VALIDATION";
  case OP_ERROR_DB_NOT_LOADED:
    return "ERROR_DB_NOT_LOADED";
  case OP_ERROR_GENERAL:
    return "ERROR_GENERAL";
  case OP_ERROR_INVALID:
    return "ERROR_INVALID";
  default:
    return "UNKNOWN";
  }
}

/*
 * formats timestamp for display using ISO 8601 format
 *
 * format: YYYY-MM-DD HH:MM:SS (british english compatible)
 * provides clear, unambiguous timestamp representation
 */
const char *format_timestamp(time_t timestamp, char *buffer,
                             size_t buffer_size) {
  if (!buffer || buffer_size == 0) {
    return ""; // defensive check
  }

  struct tm *tm_info = localtime(&timestamp);
  if (!tm_info) {
    strncpy(buffer, "INVALID", buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return buffer;
  }

  // format as ISO 8601: YYYY-MM-DD HH:MM:SS
  strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);

  return buffer;
}
