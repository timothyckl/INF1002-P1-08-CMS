#include "cms.h"
#include "adv_query.h"
#include "database.h"
#include "parser.h"
#include "sorting.h"
#include "utils.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define DECLARATION_FILE_PATH "assets/declaration.txt"
#define MENU_FILE_PATH "assets/menu.txt"
#define DEFAULT_DATA_FILE "data/P1_8-CMS.txt"
#define DEFAULT_FILE_MSG "No input received. Using default data file (%s).\n"
#define STUDENT_RECORDS_TABLE_INDEX 0

// we define these enums here (and not in the header) because the will not be
// used anywhere else in the codebase
typedef enum {
  EXIT = 0,
  OPEN,
  SHOW_ALL,
  INSERT,
  QUERY,
  UPDATE,
  DELETE,
  SAVE,
  SORT,
  ADV_QUERY,
} Operation;

// operation status codes for internal cms operations
typedef enum {
  OP_SUCCESS = 0,         // operation completed successfully
  OP_ERROR_OPEN,          // failed to open file
  OP_ERROR_INPUT,         // input reading failed
  OP_ERROR_VALIDATION,    // validation failed
  OP_ERROR_DB_NOT_LOADED, // database not loaded
  OP_ERROR_GENERAL,       // general operation error
  OP_ERROR_INVALID        // invalid operation
} OpStatus;

CMSStatus cms_init() {
  // print declaration
  int buf_size = 256;
  FILE *handle = get_file_handle(DECLARATION_FILE_PATH);

  if (handle == NULL) {
    return CMS_ERROR_FILE_OPEN;
  }

  print_file_lines(handle, buf_size, false);

  // clear screen
  char buf[256];
  printf("\nPress Enter to continue...");
  (void)fgets(buf, sizeof buf, stdin);
  fflush(stdout);
  // NOTE: use "cls" for windows
  system("clear");

  return CMS_SUCCESS;
}

CMSStatus display_menu(void) {
  int buf_size = 256;
  FILE *handle = get_file_handle(MENU_FILE_PATH);
  if (handle == NULL) {
    return CMS_ERROR_FILE_OPEN;
  }
  print_file_lines(handle, buf_size, false);

  return CMS_SUCCESS;
}

static OpStatus get_user_input(char *buf, size_t buf_size, Operation *op) {
  printf("Select an option: ");
  if (fgets(buf, buf_size, stdin) == NULL) {
    return OP_ERROR_INPUT;
  }
  fflush(stdout);
  putchar('\n');

  // strip trailing newline/carriage return
  size_t len = strcspn(buf, "\r\n");
  buf[len] = '\0';

  // handle empty input
  if (len == 0) {
    printf("CMS: Invalid input. Please enter a number.\n");
    return OP_ERROR_INVALID;
  }

  // parse using strtol for safe conversion
  char *endptr;
  errno = 0;
  long val = strtol(buf, &endptr, 10);

  // check for conversion errors
  if (endptr == buf || *endptr != '\0' || errno == ERANGE) {
    printf("CMS: Invalid input. Please enter a number.\n");
    return OP_ERROR_INVALID;
  }

  *op = (Operation)val;
  return OP_SUCCESS;
}

static void wait_for_user(void) {
  char continue_buf[256];
  printf("\nPress Enter to continue...");
  (void)fgets(continue_buf, sizeof continue_buf, stdin);
  fflush(stdout);
}

// reports cms error message, waits for user input, and returns status
// note: error_msg should NOT include "CMS: " prefix or trailing newline
static OpStatus report_error_and_return(const char *error_msg,
                                        OpStatus status) {
  printf("CMS: %s\n", error_msg);
  wait_for_user();
  return status;
}

// converts operation status code to human-readable string
// used for reporting operation failures in main_loop error handling
static const char *op_status_string(OpStatus status) {
  switch (status) {
  case OP_SUCCESS:
    return "operation succeeded";
  case OP_ERROR_OPEN:
    return "failed to open file";
  case OP_ERROR_INPUT:
    return "input reading failed";
  case OP_ERROR_VALIDATION:
    return "validation failed";
  case OP_ERROR_DB_NOT_LOADED:
    return "database not loaded";
  case OP_ERROR_GENERAL:
    return "general operation error";
  case OP_ERROR_INVALID:
    return "invalid operation";
  default:
    return "unknown operation error";
  }
}

// CMS operations (not to be confused with db operations)
// are defined here. feel free to rename them and modify the function signatures
static OpStatus open(StudentDatabase *db) {
  // check if database is already loaded
  if (db->is_loaded) {
    // warn user and confirm reload
    char confirm[10];
    printf("A database is already opened. Do you want to reload? (Y/N): ");
    fflush(stdout);

    if (!fgets(confirm, sizeof confirm, stdin)) {
      return OP_ERROR_INPUT;
    }

    // validate input (case-insensitive)
    size_t len = strcspn(confirm, "\r\n");
    confirm[len] = '\0';

    if (len == 0 ||
        (toupper(confirm[0]) != 'Y' && toupper(confirm[0]) != 'N')) {
      printf("CMS: Invalid input. Operation cancelled.\n");
      return OP_ERROR_VALIDATION;
    }

    // user cancelled
    if (toupper(confirm[0]) == 'N') {
      wait_for_user();
      return OP_SUCCESS;
    }

    // user confirmed reload - clear existing data to prevent memory leak
    for (size_t i = 0; i < db->table_count; i++) {
      table_free(db->tables[i]);
    }
    db->table_count = 0;
  }

  // prompt user for file path
  char path_buf[256];
  const char *path = NULL;

  printf("Enter a file path (press ENTER for default data file): ");
  fflush(stdout);

  if (!fgets(path_buf, sizeof path_buf, stdin)) {
    // eof or error - use default
    printf(DEFAULT_FILE_MSG, DEFAULT_DATA_FILE);
    path = DEFAULT_DATA_FILE;
  } else {
    // strip trailing newline/cr
    size_t len = strcspn(path_buf, "\r\n");
    path_buf[len] = '\0';

    if (len == 0) {
      // empty input - use default
      printf(DEFAULT_FILE_MSG, DEFAULT_DATA_FILE);
      path = DEFAULT_DATA_FILE;
    } else {
      path = path_buf;
    }
  }

  // load database from file
  DBStatus status = db_load(db, path);
  if (status != DB_SUCCESS) {
    printf("CMS: Failed to load database: %s\n", db_status_string(status));

    // if this was a reload, mark database as not loaded
    db->is_loaded = false;

    wait_for_user();

    return OP_ERROR_OPEN;
  }

  // remember the path inside the database struct
  strncpy(db->filepath, path, sizeof db->filepath);
  db->filepath[sizeof db->filepath - 1] = '\0';

  // success - mark database as loaded
  db->is_loaded = true;
  printf("CMS: The database file \"%s\" is successfully opened.\n", path);

  wait_for_user();

  return OP_SUCCESS;
}

static OpStatus show_all(StudentDatabase *db) {
  // validate database pointer
  if (!db) {
    return report_error_and_return("Database error.", OP_ERROR_GENERAL);
  }

  // validate database is loaded
  if (!db->is_loaded || db->table_count == 0) {
    return report_error_and_return("Database not loaded.",
                                   OP_ERROR_DB_NOT_LOADED);
  }

  // access the StudentRecords table
  // note: assumes tables[0] is always StudentRecords per database schema
  StudentTable *table = db->tables[STUDENT_RECORDS_TABLE_INDEX];
  if (!table) {
    return report_error_and_return("Table error.", OP_ERROR_GENERAL);
  }

  // handle empty table
  if (table->record_count == 0) {
    printf("CMS: No records found in table \"%s\".\n", table->table_name);

    wait_for_user();

    return OP_SUCCESS;
  }

  // print header message
  printf("Table Name: %s\n\n", table->table_name);

  // calculate dynamic column widths
  size_t max_id_width = 2;   // "ID" header minimum
  size_t max_name_width = 4; // "Name" header minimum
  size_t max_prog_width = 9; // "Programme" header minimum
  size_t max_mark_width = 4; // "Mark" header minimum

  for (size_t i = 0; i < table->record_count; i++) {
    StudentRecord *rec = &table->records[i];

    char format_buf[32];
    int len;

    // calculate id width
    len = snprintf(format_buf, sizeof format_buf, "%d", rec->id);
    if (len > 0 && (size_t)len > max_id_width) {
      max_id_width = (size_t)len;
    }

    // calculate name width
    size_t name_len = strlen(rec->name);
    if (name_len > max_name_width) {
      max_name_width = name_len;
    }

    // calculate programme width
    size_t prog_len = strlen(rec->prog);
    if (prog_len > max_prog_width) {
      max_prog_width = prog_len;
    }

    // calculate mark width
    len = snprintf(format_buf, sizeof format_buf, "%.2f", rec->mark);
    if (len > 0 && (size_t)len > max_mark_width) {
      max_mark_width = (size_t)len;
    }
  }

  // print column headers with calculated widths
  printf("%-*s  %-*s  %-*s  %*s\n", (int)max_id_width, "ID",
         (int)max_name_width, "Name", (int)max_prog_width, "Programme",
         (int)max_mark_width, "Mark");

  // print all records
  for (size_t i = 0; i < table->record_count; i++) {
    StudentRecord *rec = &table->records[i];
    printf("%-*d  %-*s  %-*s  %*.2f\n", (int)max_id_width, rec->id,
           (int)max_name_width, rec->name, (int)max_prog_width, rec->prog,
           (int)max_mark_width, rec->mark);
  }

  // add trailing newline
  printf("\n");

  wait_for_user();

  return OP_SUCCESS;
}

/*
 * insert new student record into database
 * prompts user for student details and validates input
 * returns: OP_SUCCESS on successful insertion, OpStatus error code on failure
 */
static OpStatus insert(StudentDatabase *db) {
  // validate database pointer
  if (!db) {
    return report_error_and_return("Database error.", OP_ERROR_GENERAL);
  }

  // validate database is loaded
  if (!db->is_loaded || db->table_count == 0) {
    return report_error_and_return("Database not loaded.",
                                   OP_ERROR_DB_NOT_LOADED);
  }

  // access the StudentRecords table
  StudentTable *table = db->tables[STUDENT_RECORDS_TABLE_INDEX];
  if (!table) {
    return report_error_and_return("Table error.", OP_ERROR_GENERAL);
  }

  // prompt for student ID
  char id_buf[256];
  printf("Enter student ID: ");
  fflush(stdout);

  if (!fgets(id_buf, sizeof id_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  // strip trailing newline/carriage return
  size_t id_len = strcspn(id_buf, "\r\n");
  id_buf[id_len] = '\0';

  // validate ID is not empty
  if (id_len == 0) {
    return report_error_and_return("Student ID cannot be empty.",
                                   OP_ERROR_VALIDATION);
  }

  // parse ID using strtol for safe conversion
  char *endptr;
  long id_long = strtol(id_buf, &endptr, 10);

  // check for conversion errors
  if (*endptr != '\0' || endptr == id_buf) {
    return report_error_and_return(
        "Invalid student ID format. Please enter a number.",
        OP_ERROR_VALIDATION);
  }

  // check ID range (fits in int)
  if (id_long < 0 || id_long > 9999999) {
    return report_error_and_return("Student ID must be between 0 and 9999999.",
                                   OP_ERROR_VALIDATION);
  }

  int student_id = (int)id_long;

  // check for duplicate ID
  for (size_t i = 0; i < table->record_count; i++) {
    if (table->records[i].id == student_id) {
      char err_msg[256];
      snprintf(err_msg, sizeof err_msg, "The record with ID=%d already exists.",
               student_id);
      return report_error_and_return(err_msg, OP_ERROR_VALIDATION);
    }
  }

  // prompt for student name
  char name_buf[256];
  printf("Enter student name: ");
  fflush(stdout);

  if (!fgets(name_buf, sizeof name_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  // strip trailing newline/carriage return
  size_t name_len = strcspn(name_buf, "\r\n");
  name_buf[name_len] = '\0';

  // validate name is not empty
  if (name_len == 0) {
    return report_error_and_return("Student name cannot be empty.",
                                   OP_ERROR_VALIDATION);
  }

  // check name length fits in StudentRecord
  if (name_len >= 50) {
    return report_error_and_return(
        "Student name is too long (max 49 characters).", OP_ERROR_VALIDATION);
  }

  // prompt for programme
  char prog_buf[256];
  printf("Enter programme: ");
  fflush(stdout);

  if (!fgets(prog_buf, sizeof prog_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  // strip trailing newline/carriage return
  size_t prog_len = strcspn(prog_buf, "\r\n");
  prog_buf[prog_len] = '\0';

  // validate programme is not empty
  if (prog_len == 0) {
    return report_error_and_return("Programme cannot be empty.",
                                   OP_ERROR_VALIDATION);
  }

  // check programme length fits in StudentRecord
  if (prog_len >= 50) {
    return report_error_and_return(
        "Programme name is too long (max 49 characters).", OP_ERROR_VALIDATION);
  }

  // prompt for mark
  char mark_buf[256];
  printf("Enter mark: ");
  fflush(stdout);

  if (!fgets(mark_buf, sizeof mark_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  // strip trailing newline/carriage return
  size_t mark_len = strcspn(mark_buf, "\r\n");
  mark_buf[mark_len] = '\0';

  // validate mark is not empty
  if (mark_len == 0) {
    return report_error_and_return("Mark cannot be empty.",
                                   OP_ERROR_VALIDATION);
  }

  // parse mark using strtof for safe conversion
  char *mark_endptr;
  float mark = strtof(mark_buf, &mark_endptr);

  // check for conversion errors
  if (*mark_endptr != '\0' || mark_endptr == mark_buf) {
    return report_error_and_return(
        "Invalid mark format. Please enter a number.", OP_ERROR_VALIDATION);
  }

  // create student record
  StudentRecord record;
  record.id = student_id;
  strncpy(record.name, name_buf, sizeof record.name - 1);
  record.name[sizeof record.name - 1] = '\0';
  strncpy(record.prog, prog_buf, sizeof record.prog - 1);
  record.prog[sizeof record.prog - 1] = '\0';
  record.mark = mark;

  // validate record using existing validation function
  ValidationStatus val_status = validate_record(&record);
  if (val_status != VALID_RECORD) {
    char err_msg[256];
    snprintf(err_msg, sizeof err_msg, "Invalid record: %s",
             validation_error_string(val_status));
    return report_error_and_return(err_msg, OP_ERROR_VALIDATION);
  }

  // insert record into table
  DBStatus db_status = table_add_record(table, &record);
  if (db_status != DB_SUCCESS) {
    char err_msg[256];
    snprintf(err_msg, sizeof err_msg, "Failed to insert record: %s",
             db_status_string(db_status));
    return report_error_and_return(err_msg, OP_ERROR_GENERAL);
  }

  // success - display message
  printf("CMS: A new record with ID=%d is successfully inserted.\n",
         student_id);

  wait_for_user();

  return OP_SUCCESS;
}

static OpStatus query(StudentDatabase *db) {
  // validate database pointer
  if (!db) {
    return report_error_and_return("Database error.", OP_ERROR_GENERAL);
  }

  // ensure database is loaded before querying
  if (!db->is_loaded || db->table_count == 0) {
    return report_error_and_return("Database not loaded.",
                                   OP_ERROR_DB_NOT_LOADED);
  }

  // retrieve student table reference
  StudentTable *table = db->tables[STUDENT_RECORDS_TABLE_INDEX];
  if (!table) {
    return report_error_and_return("Table error.", OP_ERROR_GENERAL);
  }

  if (table->record_count == 0) {
    printf("CMS: No records available to query.\n");
    wait_for_user();
    return OP_SUCCESS;
  }

  // prompt user for student ID
  char input_buf[64];
  printf("Enter student ID to search: ");
  fflush(stdout);

  if (!fgets(input_buf, sizeof input_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  size_t len = strcspn(input_buf, "\r\n");
  input_buf[len] = '\0';

  if (len == 0) {
    return report_error_and_return("Student ID cannot be empty.",
                                   OP_ERROR_VALIDATION);
  }

  char *endptr = NULL;
  long parsed_id = strtol(input_buf, &endptr, 10);
  if (endptr == input_buf || *endptr != '\0') {
    return report_error_and_return("Please enter a numeric student ID.",
                                   OP_ERROR_VALIDATION);
  }

  if (parsed_id < 0 || parsed_id > INT_MAX) {
    return report_error_and_return("Student ID must be within 0 to 2147483647.",
                                   OP_ERROR_VALIDATION);
  }

  int student_id = (int)parsed_id;

  // search for record with matching ID in student records table only
  StudentRecord *record = NULL;
  for (size_t r = 0; r < table->record_count; r++) {
    if (table->records[r].id == student_id) {
      record = &table->records[r];
      break;
    }
  }

  if (!record) {
    printf("CMS: The record with ID=%d does not exist.\n", student_id);
    wait_for_user();
    return OP_SUCCESS;
  }

  // display found record with dynamic column width formatting
  printf("CMS: The record with ID=%d is found in table \"%s\".\n", record->id,
         table->table_name);
  printf("\n");

  // calculate column widths for single record
  char format_buf[32];
  int id_width = snprintf(format_buf, sizeof format_buf, "%d", record->id);
  int mark_width =
      snprintf(format_buf, sizeof format_buf, "%.2f", record->mark);
  int name_width = (int)strlen(record->name);
  int prog_width = (int)strlen(record->prog);

  // ensure minimum widths for headers
  if (id_width < 2)
    id_width = 2; // "ID"
  if (mark_width < 4)
    mark_width = 4; // "Mark"
  if (name_width < 4)
    name_width = 4; // "Name"
  if (prog_width < 9)
    prog_width = 9; // "Programme"

  // print header with dynamic widths
  printf("%-*s  %-*s  %-*s  %*s\n", id_width, "ID", name_width, "Name",
         prog_width, "Programme", mark_width, "Mark");

  // print record with dynamic widths
  printf("%-*d  %-*s  %-*s  %*.2f\n", id_width, record->id, name_width,
         record->name, prog_width, record->prog, mark_width, record->mark);

  wait_for_user();

  return OP_SUCCESS;
}

static OpStatus adv_query(StudentDatabase *db) {
  if (!db) {
    return report_error_and_return("Database error.", OP_ERROR_GENERAL);
  }
  // guided prompt that builds a GREP/MARK pipeline then runs adv query
  AdvQueryStatus adv_status = adv_query_run_prompt(db);
  if (adv_status == ADV_QUERY_ERROR_EMPTY_DATABASE) {
    wait_for_user();
    return OP_SUCCESS;
  }
  if (adv_status != ADV_QUERY_SUCCESS) {
    printf("CMS: Advanced query failed: %s\n",
           adv_query_status_string(adv_status));
    wait_for_user();
    return OP_ERROR_GENERAL;
  }

  wait_for_user();
  return OP_SUCCESS;
}

/*
 * update existing student record in database
 * prompts user for student id, field selection, and new value
 * returns: OP_SUCCESS on successful update or cancellation, OpStatus error
 * code on failure
 */
static OpStatus update(StudentDatabase *db) {
  // validate database pointer
  if (!db) {
    return report_error_and_return("Database error.", OP_ERROR_GENERAL);
  }

  // ensure database is loaded
  if (!db->is_loaded || db->table_count == 0) {
    return report_error_and_return("Database not loaded.", OP_ERROR_DB_NOT_LOADED);
  }

  // retrieve student table reference
  StudentTable *table = db->tables[STUDENT_RECORDS_TABLE_INDEX];
  if (!table) {
    return report_error_and_return("Table error.", OP_ERROR_GENERAL);
  }

  // check for empty table
  if (table->record_count == 0) {
    printf("CMS: No records available to update.\n");
    wait_for_user();
    return OP_SUCCESS;
  }

  // prompt user for student ID
  char input_buf[64];
  printf("Enter student ID to update: ");
  fflush(stdout);

  if (!fgets(input_buf, sizeof input_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  size_t len = strcspn(input_buf, "\r\n");
  input_buf[len] = '\0';

  if (len == 0) {
    return report_error_and_return("Student ID cannot be empty.",
                                   OP_ERROR_VALIDATION);
  }

  char *endptr = NULL;
  long parsed_id = strtol(input_buf, &endptr, 10);
  if (endptr == input_buf || *endptr != '\0') {
    return report_error_and_return("Please enter a numeric student ID.",
                                   OP_ERROR_VALIDATION);
  }

  if (parsed_id < 0 || parsed_id > INT_MAX) {
    char err_msg[128];
    snprintf(err_msg, sizeof err_msg,
             "Student ID must be within 0 to %d.", INT_MAX);
    return report_error_and_return(err_msg, OP_ERROR_VALIDATION);
  }

  // search for record (same logic as QUERY)
  StudentRecord *record = NULL;

  for (size_t r = 0; r < table->record_count; r++) {
    if (table->records[r].id == (int)parsed_id) {
      record = &table->records[r];
      break;
    }
  }

  if (!record) {
    printf("CMS: The record with ID=%ld does not exist.\n", parsed_id);
    wait_for_user();
    return OP_SUCCESS;
  }

  // field selection
  printf("Select field to update:\n");
  printf("  [1] Name\n");
  printf("  [2] Programme\n");
  printf("  [3] Mark\n");
  printf("Enter your choice: ");
  fflush(stdout);

  char choice_buf[16];
  if (!fgets(choice_buf, sizeof choice_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  size_t choice_len = strcspn(choice_buf, "\r\n");
  choice_buf[choice_len] = '\0';

  if (choice_len != 1 || (choice_buf[0] < '1' || choice_buf[0] > '3')) {
    return report_error_and_return("Invalid choice.", OP_ERROR_VALIDATION);
  }

  int choice = choice_buf[0] - '0';

  const char *new_name = NULL;
  const char *new_prog = NULL;
  float new_mark = 0.0f;
  float *new_mark_ptr = NULL;

  char buffer[256];

  if (choice == 1) {
    printf("Enter new Name: ");
    fflush(stdout);
    if (!fgets(buffer, sizeof buffer, stdin)) {
      return report_error_and_return("Failed to read name.", OP_ERROR_INPUT);
    }
    size_t nlen = strcspn(buffer, "\r\n");
    buffer[nlen] = '\0';
    if (nlen == 0) {
      return report_error_and_return("Name cannot be empty.", OP_ERROR_VALIDATION);
    }
    new_name = buffer;

  } else if (choice == 2) {
    printf("Enter new Programme: ");
    fflush(stdout);
    if (!fgets(buffer, sizeof buffer, stdin)) {
      return report_error_and_return("Failed to read programme.", OP_ERROR_INPUT);
    }
    size_t plen = strcspn(buffer, "\r\n");
    buffer[plen] = '\0';
    if (plen == 0) {
      return report_error_and_return("Programme cannot be empty.", OP_ERROR_VALIDATION);
    }
    new_prog = buffer;

  } else if (choice == 3) {
    printf("Enter new Mark: ");
    fflush(stdout);

    char mark_buf[64];
    if (!fgets(mark_buf, sizeof mark_buf, stdin)) {
      return report_error_and_return("Failed to read mark.", OP_ERROR_INPUT);
    }

    // strip trailing newline/carriage return
    size_t mark_len = strcspn(mark_buf, "\r\n");
    mark_buf[mark_len] = '\0';

    char *m_endptr;
    new_mark = strtof(mark_buf, &m_endptr);
    if (m_endptr == mark_buf || *m_endptr != '\0') {
      return report_error_and_return("Invalid mark entered.", OP_ERROR_VALIDATION);
    }
    new_mark_ptr = &new_mark;
  }

  // call database-layer update
  DBStatus db_status =
      db_update_record(db, (int)parsed_id, new_name, new_prog, new_mark_ptr);

  if (db_status != DB_SUCCESS) {
    char err_msg[256];
    snprintf(err_msg, sizeof err_msg, "Failed to update record (error: %s).",
             db_status_string(db_status));
    return report_error_and_return(err_msg, OP_ERROR_GENERAL);
  }

  printf("CMS: The record with ID=%d is successfully updated.\n",
         (int)parsed_id);
  wait_for_user();
  return OP_SUCCESS;
}

/*
 * delete existing student record from database
 * prompts user for student id, confirms deletion, and removes record
 * returns: OP_SUCCESS on successful deletion or cancellation, OpStatus error
 * code on failure
 */
static OpStatus delete(StudentDatabase *db) {
  // validate database pointer
  if (!db) {
    return report_error_and_return("Database error.", OP_ERROR_GENERAL);
  }

  // validate database is loaded
  if (!db->is_loaded || db->table_count == 0) {
    return report_error_and_return("Database not loaded.",
                                   OP_ERROR_DB_NOT_LOADED);
  }

  // access the StudentRecords table
  StudentTable *table = db->tables[STUDENT_RECORDS_TABLE_INDEX];
  if (!table) {
    return report_error_and_return("Table error.", OP_ERROR_GENERAL);
  }

  // check if table has records before prompting
  if (table->record_count == 0) {
    return report_error_and_return("No records available to delete.",
                                   OP_ERROR_GENERAL);
  }

  // prompt for student id
  char id_buf[256];
  printf("Enter student ID: ");
  fflush(stdout);

  if (!fgets(id_buf, sizeof id_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  // strip trailing newline/carriage return
  size_t id_len = strcspn(id_buf, "\r\n");
  id_buf[id_len] = '\0';

  // validate ID is not empty
  if (id_len == 0) {
    return report_error_and_return("Student ID cannot be empty.",
                                   OP_ERROR_VALIDATION);
  }

  // parse ID using strtol for safe conversion with overflow detection
  char *endptr;
  errno = 0;
  long id_long = strtol(id_buf, &endptr, 10);

  // check for conversion errors and overflow
  if (errno == ERANGE || *endptr != '\0' || endptr == id_buf) {
    return report_error_and_return(
        "Invalid student ID format. Please enter a number.",
        OP_ERROR_VALIDATION);
  }

  // check ID range (fits in int)
  if (id_long < 0 || id_long > 9999999) {
    return report_error_and_return("Student ID must be between 0 and 9999999.",
                                   OP_ERROR_VALIDATION);
  }

  int student_id = (int)id_long;

  // ask user to confirm deletion
  char confirm[10];
  printf("CMS: Are you sure you want to delete record with ID=%d? Type \"Y\" "
         "to Confirm or type \"N\" to cancel.\n",
         student_id);
  fflush(stdout);

  if (!fgets(confirm, sizeof confirm, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  // strip trailing newline/carriage return
  size_t confirm_len = strcspn(confirm, "\r\n");
  confirm[confirm_len] = '\0';

  // validate confirmation input
  if (confirm_len == 0 ||
      (toupper(confirm[0]) != 'Y' && toupper(confirm[0]) != 'N')) {
    return report_error_and_return("Invalid input. Operation cancelled.",
                                   OP_ERROR_VALIDATION);
  }

  // handle cancellation
  if (toupper(confirm[0]) == 'N') {
    printf("CMS: The deletion is cancelled.\n");
    wait_for_user();
    return OP_SUCCESS;
  }

  // user confirmed - attempt to remove the record
  DBStatus db_status = table_remove_record(table, student_id);

  // handle record not found
  if (db_status == DB_ERROR_NOT_FOUND) {
    printf("CMS: The record with ID=%d does not exist.\n", student_id);
    wait_for_user();
    return OP_SUCCESS;
  }

  // handle other database errors
  if (db_status != DB_SUCCESS) {
    char err_msg[256];
    snprintf(err_msg, sizeof err_msg, "Failed to delete record: %s",
             db_status_string(db_status));
    return report_error_and_return(err_msg, OP_ERROR_GENERAL);
  }

  printf("CMS: The record with ID=%d is successfully deleted.\n", student_id);
  wait_for_user();

  return OP_SUCCESS;
}

/*
 * sort student records by id or mark in ascending or descending order
 * prompts user for sort field and order, then performs in-place sort
 * returns: OP_SUCCESS on success, OpStatus error code on failure
 */
static OpStatus sort(StudentDatabase *db) {
  // validate database pointer
  if (!db) {
    return report_error_and_return("Database error.", OP_ERROR_GENERAL);
  }

  // validate database is loaded
  if (!db->is_loaded || db->table_count == 0) {
    return report_error_and_return("Database not loaded.",
                                   OP_ERROR_DB_NOT_LOADED);
  }

  // access the StudentRecords table
  StudentTable *table = db->tables[STUDENT_RECORDS_TABLE_INDEX];
  if (!table) {
    return report_error_and_return("Table error.", OP_ERROR_GENERAL);
  }

  // validate records array exists
  if (!table->records) {
    return report_error_and_return("Table records array is NULL.",
                                   OP_ERROR_GENERAL);
  }

  // validate record count is consistent
  if (table->record_count > table->record_capacity) {
    return report_error_and_return("Table record count exceeds capacity.",
                                   OP_ERROR_VALIDATION);
  }

  // check if table has records
  if (table->record_count == 0) {
    return report_error_and_return("No records available to sort.",
                                   OP_ERROR_GENERAL);
  }

  char field_buf[10];
  printf("Select field to sort by:\n");
  printf("  [1] ID\n");
  printf("  [2] Mark\n");
  printf("Enter your choice (or press ENTER to cancel): ");
  fflush(stdout);

  if (!fgets(field_buf, sizeof field_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  // strip trailing newline/carriage return
  size_t field_len = strcspn(field_buf, "\r\n");
  field_buf[field_len] = '\0';

  // allow empty input to cancel
  if (field_len == 0) {
    printf("CMS: Sort operation cancelled.\n");
    wait_for_user();
    return OP_SUCCESS;
  }

  // validate field is exactly "1" or "2"
  char field;
  if (field_len == 1 && (field_buf[0] == '1' || field_buf[0] == '2')) {
    field = field_buf[0];
  } else {
    return report_error_and_return(
        "Invalid field. Enter '1' for ID or '2' for Mark.",
        OP_ERROR_VALIDATION);
  }

  char order_buf[10];
  printf("Select sort order:\n");
  printf("  [A] Ascending\n");
  printf("  [D] Descending\n");
  printf("Enter your choice (or press ENTER to cancel): ");
  fflush(stdout);

  if (!fgets(order_buf, sizeof order_buf, stdin)) {
    return report_error_and_return("Failed to read input.", OP_ERROR_INPUT);
  }

  // strip trailing newline/carriage return
  size_t order_len = strcspn(order_buf, "\r\n");
  order_buf[order_len] = '\0';

  // allow empty input to cancel
  if (order_len == 0) {
    printf("CMS: Sort operation cancelled.\n");
    wait_for_user();
    return OP_SUCCESS;
  }

  // validate order is exactly "A", "a", "D", or "d"
  char order;
  if (order_len == 1 &&
      (toupper(order_buf[0]) == 'A' || toupper(order_buf[0]) == 'D')) {
    order = toupper(order_buf[0]);
  } else {
    return report_error_and_return(
        "Invalid order. Enter 'A' for Ascending or 'D' for Descending.",
        OP_ERROR_VALIDATION);
  }

  // convert user input to sorting module enums
  SortField sort_field = (field == '1') ? SORT_FIELD_ID : SORT_FIELD_MARK;
  SortOrder sort_order = (order == 'A') ? SORT_ORDER_ASC : SORT_ORDER_DESC;

  // perform sort using sorting module
  sort_records(table->records, table->record_count, sort_field, sort_order);

  const char *field_name = (field == '1') ? "ID" : "Mark";
  const char *order_name = (order == 'A') ? "ascending" : "descending";

  printf("CMS: %zu record%s successfully sorted by %s in %s order.\n",
         table->record_count, (table->record_count == 1) ? "" : "s", field_name,
         order_name);

  wait_for_user();

  return OP_SUCCESS;
}

/*
 * save database to file
 * delegates file writing to database module
 * returns: OP_SUCCESS on success, OpStatus error code on failure
 */
static OpStatus save(StudentDatabase *db) {
  // validate database pointer
  if (!db) {
    return report_error_and_return("Database error.", OP_ERROR_GENERAL);
  }

  // validate database is loaded
  if (!db->is_loaded || db->table_count == 0) {
    return report_error_and_return("Database not loaded.",
                                   OP_ERROR_DB_NOT_LOADED);
  }

  // validate we have a filepath from OPEN
  if (db->filepath[0] == '\0') {
    return report_error_and_return("No file path stored for this database.",
                                   OP_ERROR_VALIDATION);
  }

  // delegate save operation to database module
  DBStatus db_status = db_save(db, db->filepath);
  if (db_status != DB_SUCCESS) {
    char err_msg[256];
    snprintf(err_msg, sizeof err_msg, "Failed to save database: %s",
             db_status_string(db_status));
    return report_error_and_return(err_msg, OP_ERROR_GENERAL);
  }

  printf("CMS: The database file \"%s\" is successfully saved.\n",
         db->filepath);
  wait_for_user();

  return OP_SUCCESS;
}

static OpStatus operation_router(Operation op, StudentDatabase *db) {
  OpStatus status;

  switch (op) {
  case OPEN:
    status = open(db);
    return status;
  case SHOW_ALL:
    status = show_all(db);
    return status;
  case INSERT:
    status = insert(db);
    return status;
  case QUERY:
    status = query(db);
    return status;
  case UPDATE:
    status = update(db);
    return status;
  case DELETE:
    status = delete(db);
    return status;
  case SAVE:
    status = save(db);
    return status;
  case SORT:
    status = sort(db);
    return status;
  case ADV_QUERY:
    status = adv_query(db);
    return status;
  case EXIT:
    printf("Goodbye!\n");
    return OP_SUCCESS;
  default:
    printf("invalid operation type shi\n");
    return OP_ERROR_INVALID;
  }
}

// TODO: i need a better name for this function ;-;
CMSStatus main_loop(void) {
  CMSStatus status;

  status = cms_init();
  if (status != CMS_SUCCESS) {
    fprintf(stderr, "Failed to display menu: %s\n", cms_status_string(status));
    return status;
  }

  // init db
  StudentDatabase *db = db_init();
  if (!db) {
    fprintf(stderr, "Failed to initialise database\n");
    return CMS_ERROR_DB_INIT;
  }

  char inp_buf[100];
  Operation op;
  OpStatus op_status;

  // main loop
  do {
    status = display_menu();
    if (status != CMS_SUCCESS) {
      fprintf(stderr, "Failed to display menu: %s\n",
              cms_status_string(status));
      return status;
    }
    op_status = get_user_input(inp_buf, sizeof inp_buf, &op);
    if (op_status != OP_SUCCESS) {
      // user input errors already print helpful messages
      // continue to allow retry
      continue;
    }

    op_status = operation_router(op, db);
    // operation errors are handled within operation functions via
    // report_error_and_return(), which displays error and waits for user
    // continue regardless to allow user to try another operation
    (void)op_status;
  } while (op != EXIT);

  db_free(db);
  return CMS_SUCCESS;
}

const char *cms_status_string(CMSStatus status) {
  switch (status) {
  case CMS_SUCCESS:
    return "operation succeeded";
  case CMS_ERROR_INIT:
    return "CMS initialisation failed";
  case CMS_ERROR_DB_INIT:
    return "database initialisation failed";
  case CMS_ERROR_INVALID_ARGUMENT:
    return "invalid argument provided";
  case CMS_ERROR_FILE_OPEN:
    return "failed to open file";
  case CMS_ERROR_FILE_IO:
    return "file I/O operation failed";
  default:
    return "unknown CMS error";
  }
}
