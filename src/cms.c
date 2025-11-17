#include "cms.h"
#include "database.h"
#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define DECLARATION_FILE_PATH "assets/declaration.txt"
#define MENU_FILE_PATH "assets/menu.txt"
#define DEFAULT_DATA_FILE "data/P1_8-CMS.txt"
#define DEFAULT_FILE_MSG "No input received. Using default data file (%s).\n"

// we define these enums here (and not in the header) because the will not be
// used anywhere else in the codebase
typedef enum {
  OPEN = 1,
  SHOW_ALL,
  INSERT,
  QUERY,
  UPDATE,
  DELETE,
  SAVE,
  EXIT,
} Operation;

typedef enum {
  OP_SUCCESS,
  OPEN_FAILURE, // failed to open file
  OP_ERR,
  OP_INVALID,
} OperationStatus;

CMSStatus cms_init() {
  // print declaration
  int buf_size = 256;
  FILE *handle = get_file_handle(DECLARATION_FILE_PATH);

  if (handle == NULL) {
    return CMS_FILE_OPEN_ERR;
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
    return CMS_FILE_OPEN_ERR;
  }
  print_file_lines(handle, buf_size, false);

  return CMS_SUCCESS;
}

static OperationStatus get_user_input(char *buf, Operation *op) {
  printf("Select an option: ");
  if (fgets(buf, sizeof buf, stdin) == NULL) {
    return OP_ERR; // should be a CMS error instead
  }
  fflush(stdout);
  putchar('\n');

  // convert inp_buf to int
  // im kms... ts is super unsafe change to strtok and read the docs
  *op = atoi(buf);

  return OP_SUCCESS; // should be a CMS_SUCCESS instead
}

static void wait_for_user(void) {
  char continue_buf[256];
  printf("\nPress Enter to continue...");
  (void)fgets(continue_buf, sizeof continue_buf, stdin);
  fflush(stdout);
}

// reports cms error message, waits for user input, and returns status
// note: error_msg should NOT include "CMS: " prefix or trailing newline
static OperationStatus report_error_and_return(const char *error_msg,
                                               OperationStatus status) {
  printf("CMS: %s\n", error_msg);
  wait_for_user();
  return status;
}

// CMS operations (not to be confused with db operations)
// are defined here. feel free to rename them and modify the function signatures
static OperationStatus open(StudentDatabase *db) {
  // check if database is already loaded
  if (db->is_loaded) {
    // warn user and confirm reload
    char confirm[10];
    printf("A database is already opened. Do you want to reload? (Y/N): ");
    fflush(stdout);

    if (!fgets(confirm, sizeof confirm, stdin)) {
      return OP_ERR;
    }

    // validate input (case-insensitive)
    size_t len = strcspn(confirm, "\r\n");
    confirm[len] = '\0';

    if (len == 0 ||
        (toupper(confirm[0]) != 'Y' && toupper(confirm[0]) != 'N')) {
      printf("CMS: Invalid input. Operation cancelled.\n");
      return OP_ERR;
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

    return OPEN_FAILURE;
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

static OperationStatus show_all(StudentDatabase *db) {
  // validate database pointer
  if (!db) {
    return report_error_and_return("Database error.", OP_ERR);
  }

  // validate database is loaded
  if (!db->is_loaded || db->table_count == 0) {
    return report_error_and_return("Database not loaded.", OP_ERR);
  }

  // access the StudentRecords table
  // note: assumes tables[0] is always StudentRecords per database schema
  StudentTable *table = db->tables[0];
  if (!table) {
    return report_error_and_return("Table error.", OP_ERR);
  }

  // handle empty table
  if (table->record_count == 0) {
    printf("CMS: No records found in table \"%s\".\n", table->table_name);

    wait_for_user();

    return OP_SUCCESS;
  }

  // print header message
  printf("Table Name: %s\n\n",
         table->table_name);

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
    len = snprintf(format_buf, sizeof format_buf, "%.1f", rec->mark);
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
    printf("%-*d  %-*s  %-*s  %*.1f\n", (int)max_id_width, rec->id,
           (int)max_name_width, rec->name, (int)max_prog_width, rec->prog,
           (int)max_mark_width, rec->mark);
  }

  // add trailing newline
  printf("\n");

  wait_for_user();

  return OP_SUCCESS;
}

OperationStatus insert() {
  // your code here
  // printf("you selected insert!\n");
  printf("hi!\n");

  return OP_SUCCESS;
}

OperationStatus query() {
  // your code here
  printf("you selected query!\n");

  return OP_SUCCESS;
}

OperationStatus update() {
  // your code here
  printf("you selected update!\n");

  return OP_SUCCESS;
}

OperationStatus delete() {
  // your code here
  printf("you selected delete!\n");

  return OP_SUCCESS;
}

static OperationStatus save(StudentDatabase *db) {
  // validate database pointer
  if (!db) {
    return report_error_and_return("Database error.", OP_ERR);
  }

  // validate database is loaded
  if (!db->is_loaded || db->table_count == 0) {
    return report_error_and_return("Database not loaded.", OP_ERR);
  }

  // validate we have a filepath from OPEN
  if (db->filepath[0] == '\0') {
    return report_error_and_return("No file path stored for this database.", OP_ERR);
  }

  // access the StudentRecords table (by convention, index 0)
  StudentTable *table = db->tables[0];
  if (!table) {
    return report_error_and_return("Table error.", OP_ERR);
  }

  FILE *fp = fopen(db->filepath, "w");
  if (!fp) {
    return report_error_and_return("Failed to save file.", OP_ERR);
  }

  // write header metadata
  fprintf(fp, "Database Name: %s\n", db->db_name);
  fprintf(fp, "Authors: %s\n", db->authors);
  fprintf(fp, "\n");
  fprintf(fp, "Table Name: %s\n", table->table_name);

  // write column headers (ID, Name, Programme, Mark)
  for (size_t i = 0; i < table->column_count; i++) {
    fprintf(fp, "%s", table->column_headers[i]);
    if (i + 1 < table->column_count) {
      fputc('\t', fp);
    }
  }
  fputc('\n', fp);

  // write all records (including inserted/updated ones)
  for (size_t i = 0; i < table->record_count; i++) {
    const StudentRecord *r = &table->records[i];
    fprintf(fp, "%d\t%s\t%s\t%.2f\n",
            r->id, r->name, r->prog, r->mark);
  }

  if (fclose(fp) != 0) {
    return report_error_and_return("Failed to close file.", OP_ERR);
  }

  printf("CMS: The database file \"%s\" is successfully saved.\n", db->filepath);
  wait_for_user();

  return OP_SUCCESS;
}


static OperationStatus operation_router(Operation op, StudentDatabase *db) {
  OperationStatus status;

  switch (op) {
  case OPEN:
    status = open(db);
    return status;
  case SHOW_ALL:
    status = show_all(db);
    return status;
  case INSERT:
    status = insert();
    return status;
  case QUERY:
    status = query();
    return status;
  case UPDATE:
    status = update();
    return status;
  case DELETE:
    status = delete();
    return status;
  case SAVE:
    status = save(db);
    return status;
  case EXIT:
    printf("Goodbye!\n");
    return OP_SUCCESS;
  default:
    printf("invalid operation type shi\n");
    return OP_INVALID;
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
    return CMS_DB_INIT_FAILURE;
  }

  char inp_buf[100];
  Operation op;
  OperationStatus op_status;

  // main loop
  do {
    status = display_menu();
    if (status != CMS_SUCCESS) {
      fprintf(stderr, "Failed to display menu: %s\n",
              cms_status_string(status));
      return status;
    }
    op_status = get_user_input(inp_buf, &op);
    // if (op_status != OP_SUCCESS) {
    //   fprintf(stderr, "Failed to perform operation: %s\n",
    //   operation_status_string(op_status)); return status;
    // }
    op_status = operation_router(op, db);
    // if (op_status != OP_SUCCESS) {
    //   fprintf(stderr, "Failed to perform operation: %s\n",
    //   operation_status_string(op_status)); return status;
    // }
  } while (op != EXIT);

  return CMS_SUCCESS;
}

const char *cms_status_string(CMSStatus status) {
  switch (status) {
  case CMS_SUCCESS:
    return "operation completed successfully";
  case CMS_INIT_FAILURE:
    return "failed initialisation step";
  case CMS_DB_INIT_FAILURE:
    return "database failed to initialise";
  case CMS_INVALID_ARG:
    return "invalid argument received";
  case CMS_FILE_OPEN_ERR:
    return "failed to retrieve file handle";
  case CMS_FILE_IO_ERR:
    return "failed to complete operation on file";
  default:
    return "unknown error";
  }
}
