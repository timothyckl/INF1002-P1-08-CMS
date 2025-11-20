#include "cms.h"
#include "commands/command.h"
#include "database.h"
#include "ui.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CMSStatus cms_init() {
  // display declaration
  CMSStatus status = ui_display_declaration();
  if (status != CMS_SUCCESS) {
    return status;
  }

  // wait for user and clear screen
  char buf[256];
  printf("\nPress Enter to continue...");
  (void)fgets(buf, sizeof buf, stdin);
  fflush(stdout);
  // NOTE: use "cls" for windows
  system("clear");

  return CMS_SUCCESS;
}

CMSStatus display_menu(void) { return ui_display_menu(); }

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
      continue;
    }

    op_status = execute_operation(op, db);
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
    return "unknown error";
  }
}
