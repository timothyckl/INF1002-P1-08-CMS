#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adv_query.h"

static void print_case_result(const char *name, AdvQueryStatus status,
                              AdvQueryStatus expected) {
  if (status == expected) {
    printf("[OK] %s\n", name);
  } else {
    printf("[FAIL] %s (got %s)\n", name, adv_query_status_string(status));
  }
}

static AdvQueryStatus run_pipeline(const char *label, StudentDatabase *db,
                                   const char *pipeline) {
  printf("\n-- %s\nPipeline: %s\n", label, pipeline);
  AdvQueryStatus status = adv_query_execute(db, pipeline);
  printf("Status: %s\n", adv_query_status_string(status));
  return status;
}

int main(void) {
  printf("Running adv_query_execute tests...\n");

  StudentDatabase *db = db_init();
  if (!db) {
    fprintf(stderr, "Failed to init database\n");
    return EXIT_FAILURE;
  }

  print_case_result("NULL database", adv_query_execute(NULL, ""),
                    ADV_QUERY_ERROR_INVALID_ARGUMENT);
  print_case_result("NULL pipeline", adv_query_execute(db, NULL),
                    ADV_QUERY_ERROR_INVALID_ARGUMENT);

  print_case_result("Empty DB",
                    adv_query_execute(db, "GREP NAME = test"),
                    ADV_QUERY_ERROR_EMPTY_DATABASE);

  DBStatus load_status = db_load(db, "data/P1_8-CMS.txt");
  if (load_status != DB_SUCCESS) {
    fprintf(stderr, "Failed to load database: %s\n",
            db_status_string(load_status));
  }

  print_case_result("Empty pipeline",
                    run_pipeline("Empty pipeline", db, ""),
                    ADV_QUERY_ERROR_PARSE);
  print_case_result("Unknown command",
                    run_pipeline("Unknown", db, "HELLO NAME = x"),
                    ADV_QUERY_ERROR_PARSE);
  print_case_result("Duplicate mark",
                    run_pipeline("Mark duplicate", db,
                                  "MARK > 50 | MARK < 60"),
                    ADV_QUERY_ERROR_PARSE);
  print_case_result("Duplicate text field",
                    run_pipeline("Name twice", db,
                                  "GREP NAME = Jo | GREP NAME = an"),
                    ADV_QUERY_ERROR_PARSE);
  print_case_result("Invalid field",
                    run_pipeline("Bad field", db, "GREP CITY = SG"),
                    ADV_QUERY_ERROR_PARSE);
  print_case_result("Invalid mark op",
                    run_pipeline("Bad mark", db, "MARK != 55"),
                    ADV_QUERY_ERROR_PARSE);

  print_case_result("Valid GREP",
                    run_pipeline("Valid GREP", db, "GREP NAME = Jo"),
                    ADV_QUERY_OK);
  print_case_result("Valid MARK",
                    run_pipeline("Valid MARK", db, "MARK > 60"),
                    ADV_QUERY_OK);
  print_case_result("Combined",
                    run_pipeline("Name + Mark", db,
                                  "GREP PROGRAMME = Software | MARK > 60"),
                    ADV_QUERY_OK);

  db_free(db);
  return EXIT_SUCCESS;
}
