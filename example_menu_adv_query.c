#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adv_query.h"

#define INPUT_BUF 256
#define MAX_SELECTIONS 8

typedef struct {
  int field; // 1=ID, 2=Name, 3=Programme, 4=Mark
  char op;   // only used for Mark comparisons
  char value[INPUT_BUF];
} QuerySelection;

static void trim_newline(char *text) {
  if (!text) {
    return;
  }
  size_t len = strlen(text);
  if (len > 0 && text[len - 1] == '\n') {
    text[len - 1] = '\0';
  }
}

static int read_line(const char *prompt, char *buffer, size_t size) {
  printf("%s", prompt);
  if (!fgets(buffer, size, stdin)) {
    return 0;
  }
  trim_newline(buffer);
  return 1;
}

static int prompt_int(const char *prompt, int *out_value) {
  char buffer[INPUT_BUF];
  if (!read_line(prompt, buffer, sizeof(buffer))) {
    return 0;
  }
  char *endptr = NULL;
  long parsed = strtol(buffer, &endptr, 10);
  if (endptr == buffer || *endptr != '\0') {
    return -1;
  }
  *out_value = (int)parsed;
  return 1;
}

static int prompt_field_choice(void) {
  while (1) {
    printf("\nPick a field to filter:\n");
    printf(" 1) Student ID\n");
    printf(" 2) Name\n");
    printf(" 3) Programme\n");
    printf(" 4) Mark\n");
    printf(" 0) Cancel\n");
    int choice = 0;
    int rc = prompt_int("Select option: ", &choice);
    if (rc == 0) {
      return 0;
    }
    if (rc == 1 && choice >= 0 && choice <= 4) {
      return choice;
    }
    printf("Invalid choice. Try again.\n");
  }
}

static int prompt_yes_no(const char *prompt) {
  char buffer[INPUT_BUF];
  while (1) {
    if (!read_line(prompt, buffer, sizeof(buffer))) {
      return 0;
    }
    if (buffer[0] == '\0') {
      continue;
    }
    char c = (char)tolower((unsigned char)buffer[0]);
    if (c == 'y') {
      return 1;
    }
    if (c == 'n') {
      return 0;
    }
    printf("Please enter Y or N.\n");
  }
}

static void sanitize_quotes(char *text) {
  for (char *cursor = text; *cursor; cursor++) {
    if (*cursor == '"') {
      *cursor = '\'';
    }
  }
}

static void prompt_text_value(const char *label, char *output, size_t size) {
  char buffer[INPUT_BUF];
  while (1) {
    char prompt[64];
    snprintf(prompt, sizeof(prompt), "Enter %s to search: ", label);
    if (!read_line(prompt, buffer, sizeof(buffer))) {
      continue;
    }
    if (buffer[0] == '\0') {
      printf("Input cannot be empty.\n");
      continue;
    }
    sanitize_quotes(buffer);
    strncpy(output, buffer, size - 1);
    output[size - 1] = '\0';
    break;
  }
}

static char prompt_mark_operator(void) {
  while (1) {
    printf("\nMark comparison\n");
    printf(" 1) Greater than\n");
    printf(" 2) Less than\n");
    printf(" 3) Equal to\n");
    int choice = 0;
    int rc = prompt_int("Select option: ", &choice);
    if (rc != 1) {
      printf("Please enter 1, 2, or 3.\n");
      continue;
    }
    if (choice == 1) {
      return '>';
    }
    if (choice == 2) {
      return '<';
    }
    if (choice == 3) {
      return '=';
    }
    printf("Please enter 1, 2, or 3.\n");
  }
}

static void prompt_mark_value(char *output, size_t size) {
  char buffer[INPUT_BUF];
  while (1) {
    if (!read_line("Enter mark value: ", buffer, sizeof(buffer))) {
      continue;
    }
    if (buffer[0] == '\0') {
      printf("Input cannot be empty.\n");
      continue;
    }
    char *endptr = NULL;
    strtod(buffer, &endptr);
    if (endptr == buffer || *endptr != '\0') {
      printf("Please enter a numeric value.\n");
      continue;
    }
    strncpy(output, buffer, size - 1);
    output[size - 1] = '\0';
    break;
  }
}

static const char *field_token(int field) {
  switch (field) {
  case 1:
    return "ID";
  case 2:
    return "NAME";
  case 3:
    return "PROGRAMME";
  default:
    return "";
  }
}

static const char *field_label(int field) {
  switch (field) {
  case 1:
    return "student ID";
  case 2:
    return "name";
  case 3:
    return "programme";
  case 4:
    return "mark";
  default:
    return "field";
  }
}

static int field_already_selected(const QuerySelection *selections,
                                  size_t count, int field) {
  for (size_t i = 0; i < count; i++) {
    if (selections[i].field == field) {
      return 1;
    }
  }
  return 0;
}

static int collect_field_selections(QuerySelection *selections,
                                    size_t *selection_count) {
  size_t count = 0;
  while (count < MAX_SELECTIONS) {
    int choice = prompt_field_choice();
    if (choice == 0) {
      if (count == 0) {
        printf("Cancelled advanced search.\n");
        return 0;
      }
      printf("Use the Y/N prompt to finish.\n");
      continue;
    }
    if (field_already_selected(selections, count, choice)) {
      printf("You already selected %s. Pick another field.\n",
             field_label(choice));
      continue;
    }
    selections[count].field = choice;
    selections[count].op = '=';
    selections[count].value[0] = '\0';
    count++;

    if (count >= MAX_SELECTIONS) {
      printf("Reached maximum number of fields (%d).\n", MAX_SELECTIONS);
      break;
    }
    if (!prompt_yes_no("Add another field? (Y/N): ")) {
      break;
    }
  }

  *selection_count = count;
  return count > 0;
}

static void collect_field_values(QuerySelection *selections, size_t count) {
  for (size_t i = 0; i < count; i++) {
    if (selections[i].field == 4) {
      selections[i].op = prompt_mark_operator();
      prompt_mark_value(selections[i].value, sizeof(selections[i].value));
    } else {
      prompt_text_value(field_label(selections[i].field),
                        selections[i].value, sizeof(selections[i].value));
    }
  }
}

static void build_pipeline(const QuerySelection *selections, size_t count,
                           char *pipeline, size_t size) {
  pipeline[0] = '\0';
  for (size_t i = 0; i < count; i++) {
    char stage[INPUT_BUF * 2];
    if (selections[i].field == 4) {
      snprintf(stage, sizeof(stage), "MARK %c %s", selections[i].op,
               selections[i].value);
    } else {
      snprintf(stage, sizeof(stage), "GREP %s = \"%s\"",
               field_token(selections[i].field), selections[i].value);
    }
    if (pipeline[0] != '\0') {
      strncat(pipeline, " | ", size - strlen(pipeline) - 1);
    }
    strncat(pipeline, stage, size - strlen(pipeline) - 1);
  }
}

static void run_advanced_search(StudentDatabase *db) {
  QuerySelection selections[MAX_SELECTIONS];
  size_t selection_count = 0;
  if (!collect_field_selections(selections, &selection_count)) {
    return;
  }

  collect_field_values(selections, selection_count);

  char pipeline[INPUT_BUF * MAX_SELECTIONS] = {0};
  build_pipeline(selections, selection_count, pipeline, sizeof(pipeline));

  AdvQueryStatus status = adv_query_execute(db, pipeline);
  if (status != ADV_QUERY_OK) {
    printf("Advanced search failed: %s\n", adv_query_status_string(status));
  }
}

int main(void) {
  printf("Advanced search harness\n");

  StudentDatabase *db = db_init();
  if (!db) {
    fprintf(stderr, "Failed to initialise database.\n");
    return EXIT_FAILURE;
  }

  DBStatus status = db_load(db, "data/P1_8-CMS.txt");
  if (status != DB_SUCCESS) {
    fprintf(stderr, "Failed to load database: %s\n", db_status_string(status));
    db_free(db);
    return EXIT_FAILURE;
  }

  int running = 1;
  while (running) {
    printf("\nMenu\n");
    printf(" 1) Advanced search\n");
    printf(" 2) Exit\n");
    int option = 0;
    int rc = prompt_int("Select option: ", &option);
    if (rc == 0 || (rc == 1 && option == 2)) {
      break;
    }
    if (rc != 1) {
      printf("Enter a number.\n");
      continue;
    }
    if (option == 1) {
      run_advanced_search(db);
    } else {
      printf("Choose 1 or 2.\n");
    }
  }

  db_free(db);
  return EXIT_SUCCESS;
}
