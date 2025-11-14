#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "update.h"
#include "database.h"

DBStatus cms_handle_update(StudentDatabase *db) {
  if (!db || !db->is_loaded || db->table_count == 0) {
    printf("CMS: Database not loaded.\n");
    return DB_ERROR_NULL_POINTER;  // cms.c will decide how to treat this
  }

  int id;
  printf("Enter student ID to update: ");
  if (scanf("%d", &id) != 1) {
    printf("CMS: Invalid ID input.\n");
    // clear invalid input
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
    return DB_ERROR_INVALID_DATA;
  }

  // clear leftover newline
  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF) {}

  // check if record exists first
  StudentRecord *rec = db_find_record_by_id(db, id);
  if (!rec) {
    printf("CMS: The record with ID=%d does not exist.\n", id);
    return DB_ERROR_NOT_FOUND;
  }

  // choose field to update
  printf("Select field to update:\n");
  printf("1) Name\n");
  printf("2) Programme\n");
  printf("3) Mark\n");
  printf("Choice: ");

  int choice;
  if (scanf("%d", &choice) != 1) {
    printf("CMS: Invalid field selection.\n");
    while ((ch = getchar()) != '\n' && ch != EOF) {}
    return DB_ERROR_INVALID_DATA;
  }
  while ((ch = getchar()) != '\n' && ch != EOF) {}

  const char *new_name = NULL;
  const char *new_prog = NULL;
  float new_mark_value;
  const float *new_mark = NULL;

  if (choice == 1) {
    char name_buf[128];
    printf("Enter new Name: ");
    if (!fgets(name_buf, sizeof name_buf, stdin)) {
      printf("CMS: Failed to read name.\n");
      return DB_ERROR_INVALID_DATA;
    }
    size_t len = strcspn(name_buf, "\r\n");
    name_buf[len] = '\0';
    if (len == 0) {
      printf("CMS: Name cannot be empty.\n");
      return DB_ERROR_INVALID_DATA;
    }
    new_name = name_buf;

  } else if (choice == 2) {
    char prog_buf[128];
    printf("Enter new Programme: ");
    if (!fgets(prog_buf, sizeof prog_buf, stdin)) {
      printf("CMS: Failed to read programme.\n");
      return DB_ERROR_INVALID_DATA;
    }
    size_t len = strcspn(prog_buf, "\r\n");
    prog_buf[len] = '\0';
    if (len == 0) {
      printf("CMS: Programme cannot be empty.\n");
      return DB_ERROR_INVALID_DATA;
    }
    new_prog = prog_buf;

  } else if (choice == 3) {
    printf("Enter new Mark: ");
    if (scanf("%f", &new_mark_value) != 1) {
      printf("CMS: Invalid mark input.\n");
      while ((ch = getchar()) != '\n' && ch != EOF) {}
      return DB_ERROR_INVALID_DATA;
    }
    while ((ch = getchar()) != '\n' && ch != EOF) {}
    new_mark = &new_mark_value;

  } else {
    printf("CMS: Invalid field selection.\n");
    return DB_ERROR_INVALID_DATA;
  }

  // Now call your data-layer function
  DBStatus st = db_update_record(db, id, new_name, new_prog, new_mark);

  // Print detailed CMS messages based on result
  if (st == DB_SUCCESS) {
    printf("CMS: The record with ID=%d is successfully updated.\n", id);
  } else if (st == DB_ERROR_NOT_FOUND) {
    // Should not happen since we checked above, but just in case
    printf("CMS: The record with ID=%d does not exist.\n", id);
  } else if (st == DB_ERROR_INVALID_DATA) {
    printf("CMS: Invalid data. The record was not updated.\n");
  } else {
    printf("CMS: Update failed: %s\n", db_status_string(st));
  }

  return st;
}
