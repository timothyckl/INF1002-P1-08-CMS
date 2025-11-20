#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include "commands/command.h"
#include <stddef.h>
#include <time.h>

/*
 * event log module for tracking user operations within a session
 *
 * provides functionality to:
 * - log operations with timestamps and status
 * - display operation history
 * - manage log lifecycle (init, free)
 *
 * capacity: starts at 50 entries, grows to 1000 maximum
 * overflow: implements circular buffer (overwrites oldest entries)
 */

// maximum number of events to store
#define EVENT_LOG_MAX_CAPACITY 1000
#define EVENT_LOG_INITIAL_CAPACITY 50

/*
 * represents a single logged operation event
 *
 * captures operation type, execution status, and timestamp
 * details field reserved for future enhancements (unused initially)
 */
typedef struct {
  time_t timestamp;     // when operation occurred (unix time)
  Operation operation;  // operation type from Operation enum
  OpStatus status;      // operation result status
  char details[128];    // optional context (unused initially, buffer reduced for
                        // safety)
} EventEntry;

/*
 * event log container managing dynamic array of events
 *
 * implements growth strategy:
 * - starts with 50 entries
 * - doubles when full
 * - caps at 1000 entries
 * - uses circular buffer when max reached
 */
struct EventLog {
  EventEntry *entries; // dynamic array of event entries
  size_t count;        // number of logged events (may exceed capacity for
                       // circular buffer)
  size_t capacity;     // current allocated capacity
};

/*
 * initialises a new event log
 *
 * allocates initial capacity of 50 entries
 * returns pointer to new log, or NULL on allocation failure
 *
 * note: caller responsible for freeing with event_log_free()
 */
EventLog *event_log_init(void);

/*
 * frees event log and all associated memory
 *
 * safely handles NULL pointer (no-op)
 * sets pointer to NULL after freeing
 */
void event_log_free(EventLog *log);

/*
 * logs an operation event
 *
 * automatically handles:
 * - capacity growth (doubles until 1000 max)
 * - circular buffer overflow (overwrites oldest)
 * - timestamp capture (current time)
 *
 * fails silently on errors (logging is non-critical infrastructure)
 * details field left empty (reserved for future use)
 *
 * note: never crashes application - logging failures are silent
 */
void log_event(EventLog *log, Operation op, OpStatus status);

/*
 * converts operation enum to display string
 *
 * returns operation name (e.g., "OPEN", "INSERT", "QUERY")
 * returns "UNKNOWN" for invalid operation values
 */
const char *event_operation_to_string(Operation op);

/*
 * converts operation status to display string
 *
 * returns status description (e.g., "SUCCESS", "ERROR")
 * returns "UNKNOWN" for invalid status values
 */
const char *event_status_to_string(OpStatus status);

/*
 * formats timestamp for display
 *
 * uses ISO 8601 format: YYYY-MM-DD HH:MM:SS
 * writes formatted string to provided buffer
 *
 * buffer_size: size of output buffer (recommend 32 bytes minimum)
 * returns pointer to buffer for convenience
 */
const char *format_timestamp(time_t timestamp, char *buffer,
                              size_t buffer_size);

#endif // EVENT_LOG_H
