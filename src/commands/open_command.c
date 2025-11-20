#include "commands/command.h"
#include "commands/command_utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

OpStatus execute_open(StudentDatabase *db) {
  if (db->is_loaded) {
    char confirm[10];
    printf("A database is already opened. Do you want to reload? (Y/N): ");
    fflush(stdout);

    if (!fgets(confirm, sizeof confirm, stdin)) {
      return OP_ERROR_INPUT;
    }

    size_t len = strcspn(confirm, "\r\n");
    confirm[len] = '\0';

    if (len == 0 ||
        (toupper(confirm[0]) != 'Y' && toupper(confirm[0]) != 'N')) {
      printf("CMS: Invalid input. Operation cancelled.\n");
      return OP_ERROR_VALIDATION;
    }

    if (toupper(confirm[0]) == 'N') {
      cmd_wait_for_user();
      return OP_SUCCESS;
    }

    // user confirmed reload - clear existing data to prevent memory leak
    for (size_t i = 0; i < db->table_count; i++) {
      table_free(db->tables[i]);
    }
    db->table_count = 0;
  }

  char path_buf[256];
  const char *path = NULL;

  printf("Enter a file path (press ENTER for default data file): ");
  fflush(stdout);

  if (!fgets(path_buf, sizeof path_buf, stdin)) {
    // eof or error - use default
    printf(DEFAULT_FILE_MSG, DEFAULT_DATA_FILE);
    path = DEFAULT_DATA_FILE;
  } else {
    size_t len = strcspn(path_buf, "\r\n");
    path_buf[len] = '\0';

    if (len == 0) {
      printf(DEFAULT_FILE_MSG, DEFAULT_DATA_FILE);
      path = DEFAULT_DATA_FILE;
    } else {
      path = path_buf;
    }
  }

  DBStatus status = db_load(db, path);
  if (status != DB_SUCCESS) {
    printf("CMS: Failed to load database: %s\n", db_status_string(status));

    // if this was a reload, mark database as not loaded
    db->is_loaded = false;

    cmd_wait_for_user();

    return OP_ERROR_OPEN;
  }

  strncpy(db->filepath, path, sizeof db->filepath);
  db->filepath[sizeof db->filepath - 1] = '\0';

  db->is_loaded = true;
  printf("CMS: The database file \"%s\" is successfully opened.\n", path);

  cmd_wait_for_user();

  return OP_SUCCESS;
}
