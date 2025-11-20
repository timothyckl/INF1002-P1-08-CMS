#include "adv_query.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strtok_r(str, delim, saveptr) strtok_s(str, delim, saveptr)
#endif

typedef enum {
  QUERY_FIELD_NAME = 0,
  QUERY_FIELD_PROGRAMME,
  QUERY_FIELD_MARK,
  QUERY_FIELD_INVALID
} QueryField;

// duplicate string to heap; caller frees
static char *dup_string(const char *src) {
  if (!src) {
    return NULL;
  }
  size_t len = strlen(src);
  char *copy = malloc(len + 1);
  if (!copy) {
    return NULL;
  }
  memcpy(copy, src, len + 1);
  return copy;
}

// trim leading/trailing whitespace in-place
static char *trim(char *text) {
  if (!text) {
    return text;
  }
  while (*text && isspace((unsigned char)*text)) {
    text++;
  }
  if (*text == '\0') {
    return text;
  }
  char *end = text + strlen(text) - 1;
  while (end > text && isspace((unsigned char)*end)) {
    *end = '\0';
    end--;
  }
  return text;
}

// case-insensitive string equals
static int strcaseequal(const char *a, const char *b) {
  if (!a || !b) {
    return 0;
  }
  while (*a && *b) {
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
      return 0;
    }
    a++;
    b++;
  }
  return *a == '\0' && *b == '\0';
}

// map token to supported query field
static QueryField parse_field(const char *token) {
  if (strcaseequal(token, "NAME")) {
    return QUERY_FIELD_NAME;
  }
  if (strcaseequal(token, "PROGRAMME") || strcaseequal(token, "PROGRAM")) {
    return QUERY_FIELD_PROGRAMME;
  }
  if (strcaseequal(token, "MARK") || strcaseequal(token, "MARKS")) {
    return QUERY_FIELD_MARK;
  }
  return QUERY_FIELD_INVALID;
}

// flatten all records into one array for filtering
static size_t collect_records(StudentDatabase *db, StudentRecord ***out) {
  size_t total = 0;
  for (size_t t = 0; t < db->table_count; t++) {
    StudentTable *table = db->tables[t];
    if (table) {
      total += table->record_count;
    }
  }
  if (total == 0) {
    *out = NULL;
    return 0;
  }
  StudentRecord **records = malloc(total * sizeof(StudentRecord *));
  if (!records) {
    *out = NULL;
    return (size_t)-1;
  }
  size_t idx = 0;
  for (size_t t = 0; t < db->table_count; t++) {
    StudentTable *table = db->tables[t];
    if (!table) {
      continue;
    }
    for (size_t r = 0; r < table->record_count; r++) {
      records[idx++] = &table->records[r];
    }
  }
  *out = records;
  return total;
}

// substring check ignoring case
static int contains_case_insensitive(const char *haystack, const char *needle) {
  if (!haystack || !needle || *needle == '\0') {
    return 0;
  }
  size_t needle_len = strlen(needle);
  for (const char *cursor = haystack; *cursor; cursor++) {
    size_t i = 0;
    while (cursor[i] && tolower((unsigned char)cursor[i]) ==
                            tolower((unsigned char)needle[i])) {
      i++;
      if (i == needle_len) {
        return 1;
      }
    }
  }
  return 0;
}

// match a record against a GREP stage for a given field
static int grep_matches(const StudentRecord *record, QueryField field,
                        const char *pattern) {
  if (!pattern) {
    return 0;
  }
  if (field == QUERY_FIELD_NAME) {
    return contains_case_insensitive(record->name, pattern);
  }
  if (field == QUERY_FIELD_PROGRAMME) {
    return contains_case_insensitive(record->prog, pattern);
  }
  return 0;
}

static int mark_matches(const StudentRecord *record, char op, double value) {
  if (op == '<') {
    return record->mark < value;
  }
  if (op == '>') {
    return record->mark > value;
  }
  return record->mark == value;
}

static void strip_quotes(char *text) {
  size_t len = strlen(text);
  if (len >= 2 && text[0] == '"' && text[len - 1] == '"') {
    memmove(text, text + 1, len - 2);
    text[len - 2] = '\0';
  }
}

static int apply_text_filter(StudentRecord **records, unsigned char *keep,
                             size_t count, QueryField field,
                             const char *pattern) {
  if (!pattern || *pattern == '\0') {
    return 0;
  }
  for (size_t i = 0; i < count; i++) {
    if (keep[i] && !grep_matches(records[i], field, pattern)) {
      keep[i] = 0;
    }
  }
  return 1;
}

// apply MARK comparison to keep-mask for all records
static int apply_mark_filter(StudentRecord **records, unsigned char *keep,
                             size_t count, char op, double value) {
  for (size_t i = 0; i < count; i++) {
    if (keep[i] && !mark_matches(records[i], op, value)) {
      keep[i] = 0;
    }
  }
  return 1;
}

// parse and apply a GREP stage to filter by name/programme substring
static int parse_grep_stage(StudentRecord **records, unsigned char *keep,
                            size_t count, char *expr, int *field_used) {
  expr = trim(expr);
  if (*expr == '\0') {
    return 0;
  }
  char field_buf[32] = {0};
  size_t idx = 0;
  while (expr[idx] && !isspace((unsigned char)expr[idx]) && expr[idx] != '=' &&
         idx < sizeof(field_buf) - 1) {
    field_buf[idx] = expr[idx];
    idx++;
  }
  field_buf[idx] = '\0';
  expr += idx;
  expr = trim(expr);
  if (*expr == '=') {
    expr++;
    expr = trim(expr);
  }
  QueryField field = parse_field(field_buf);
  if (field == QUERY_FIELD_INVALID || field == QUERY_FIELD_MARK) {
    return 0;
  }
  if (field_used[field]) {
    return 0;
  }
  strip_quotes(expr);
  field_used[field] = 1;
  return apply_text_filter(records, keep, count, field, expr);
}

// parse and apply a MARK comparison stage
static int parse_mark_stage(StudentRecord **records, unsigned char *keep,
                            size_t count, char *expr, int *field_used) {
  expr = trim(expr);
  if (*expr == '\0') {
    return 0;
  }
  char op = *expr;
  if (op != '<' && op != '>' && op != '=') {
    return 0;
  }
  expr++;
  expr = trim(expr);
  if (*expr == '\0') {
    return 0;
  }
  char *endptr = NULL;
  double value = strtod(expr, &endptr);
  if (endptr == expr || *endptr != '\0') {
    return 0;
  }
  if (field_used[QUERY_FIELD_MARK]) {
    return 0;
  }
  field_used[QUERY_FIELD_MARK] = 1;
  return apply_mark_filter(records, keep, count, op, value);
}

// entry to run a pipeline string (already built) against the database
AdvQueryStatus adv_query_execute(StudentDatabase *db, const char *pipeline) {
  if (!db || !pipeline) {
    return ADV_QUERY_ERROR_INVALID_ARGUMENT;
  }
  if (db->table_count == 0) {
    return ADV_QUERY_ERROR_EMPTY_DATABASE;
  }

  StudentRecord **records = NULL;
  size_t total = collect_records(db, &records);
  if (total == (size_t)-1) {
    return ADV_QUERY_ERROR_MEMORY;
  }
  if (total == 0) {
    printf("ADVQUERY: No records matched the pipeline.\n");
    return ADV_QUERY_OK;
  }

  unsigned char *keep = malloc(total);
  if (!keep) {
    free(records);
    return ADV_QUERY_ERROR_MEMORY;
  }
  memset(keep, 1, total);

  char *working = dup_string(pipeline);
  if (!working) {
    free(records);
    free(keep);
    return ADV_QUERY_ERROR_MEMORY;
  }

  int field_used[3] = {0};
  int success = 1;
  int stages = 0;
  char *ctx = NULL;
  char *stage = strtok_r(working, "|", &ctx);

  while (stage && success) {
    char *trimmed = trim(stage);
    if (*trimmed == '\0') {
      success = 0;
      break;
    }
    char cmd[16] = {0};
    size_t idx = 0;
    while (trimmed[idx] && !isspace((unsigned char)trimmed[idx]) &&
           idx < sizeof(cmd) - 1) {
      cmd[idx] = trimmed[idx];
      idx++;
    }
    cmd[idx] = '\0';
    trimmed += idx;

    if (strcaseequal(cmd, "GREP")) {
      success = parse_grep_stage(records, keep, total, trimmed, field_used);
    } else if (strcaseequal(cmd, "MARK") || strcaseequal(cmd, "FILTER")) {
      success = parse_mark_stage(records, keep, total, trimmed, field_used);
    } else {
      success = 0;
    }

    if (success) {
      stages++;
      stage = strtok_r(NULL, "|", &ctx);
    }
  }

  if (!success || stages == 0) {
    free(records);
    free(keep);
    free(working);
    return ADV_QUERY_ERROR_PARSE;
  }

  size_t match_count = 0;
  for (size_t i = 0; i < total; i++) {
    if (keep[i]) {
      match_count++;
    }
  }

  if (match_count == 0) {
    printf("ADVQUERY: No records matched the pipeline.\n");
  } else {
    printf("ID\tName\tProgramme\tMark\n");
    for (size_t i = 0; i < total; i++) {
      if (keep[i]) {
        StudentRecord *r = records[i];
        printf("%d\t%s\t%s\t%.2f\n", r->id, r->name, r->prog, r->mark);
      }
    }
    printf("Total: %zu record(s)\n", match_count);
  }

  free(records);
  free(keep);
  free(working);
  return ADV_QUERY_OK;
}

const char *adv_query_status_string(AdvQueryStatus status) {
  switch (status) {
  case ADV_QUERY_OK:
    return "advanced query succeeded";
  case ADV_QUERY_ERROR_INVALID_ARGUMENT:
    return "invalid argument";
  case ADV_QUERY_ERROR_EMPTY_DATABASE:
    return "database not loaded";
  case ADV_QUERY_ERROR_PARSE:
    return "advanced query parse failed";
  case ADV_QUERY_ERROR_MEMORY:
    return "memory allocation failed";
  default:
    return "unknown advanced query error";
  }
}

/*
 * interactive wrapper used by menus: guides the user to build a pipeline
 * then executes it via adv_query_execute
 */
static void adv_query_trim_newline(char *text) {
  if (!text) {
    return;
  }
  size_t len = strcspn(text, "\r\n");
  text[len] = '\0';
}

static int adv_query_read_line(const char *prompt, char *buffer, size_t size) {
  printf("%s", prompt);
  if (!fgets(buffer, size, stdin)) {
    return 0;
  }
  adv_query_trim_newline(buffer);
  return 1;
}

static int adv_query_prompt_int(const char *prompt, int *out_value) {
  char buffer[256];
  if (!adv_query_read_line(prompt, buffer, sizeof(buffer))) {
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

// simple container for a single guided prompt selection
typedef struct {
  int field; // 1=Name, 2=Programme, 3=Mark
  char op;   // only used for Mark comparisons
  char value[256];
} AdvQuerySelection;

static int adv_query_prompt_field(void) {
  while (1) {
    printf("\nPick a field to filter:\n");
    printf(" 1) Name\n");
    printf(" 2) Programme\n");
    printf(" 3) Mark\n");
    printf(" 0) Cancel\n");
    int choice = 0;
    int rc = adv_query_prompt_int("Select option: ", &choice);
    if (rc == 0) {
      return 0;
    }
    if (rc == 1 && choice >= 0 && choice <= 3) {
      return choice;
    }
    printf("Invalid choice. Try again.\n");
  }
}

static int adv_query_yes_no(const char *prompt) {
  char buffer[256];
  while (1) {
    if (!adv_query_read_line(prompt, buffer, sizeof(buffer))) {
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

static void adv_query_sanitize_quotes(char *text) {
  for (char *cursor = text; *cursor; cursor++) {
    if (*cursor == '"') {
      *cursor = '\'';
    }
  }
}

static void adv_query_prompt_text(const char *label, char *output,
                                  size_t size) {
  char buffer[256];
  while (1) {
    char prompt[64];
    snprintf(prompt, sizeof(prompt), "Enter %s to search: ", label);
    if (!adv_query_read_line(prompt, buffer, sizeof(buffer))) {
      continue;
    }
    if (buffer[0] == '\0') {
      printf("Input cannot be empty.\n");
      continue;
    }
    adv_query_sanitize_quotes(buffer);
    strncpy(output, buffer, size - 1);
    output[size - 1] = '\0';
    break;
  }
}

static char adv_query_prompt_mark_op(void) {
  while (1) {
    printf("\nMark comparison\n");
    printf(" 1) Greater than\n");
    printf(" 2) Less than\n");
    printf(" 3) Equal to\n");
    int choice = 0;
    int rc = adv_query_prompt_int("Select option: ", &choice);
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

static void adv_query_prompt_mark_value(char *output, size_t size) {
  char buffer[256];
  while (1) {
    if (!adv_query_read_line("Enter mark value: ", buffer, sizeof(buffer))) {
      continue;
    }
    if (buffer[0] == '\0') {
      printf("Mark cannot be empty.\n");
      continue;
    }
    strncpy(output, buffer, size - 1);
    output[size - 1] = '\0';
    break;
  }
}

static const char *adv_query_field_token(int field_choice) {
  switch (field_choice) {
  case 1:
    return "NAME";
  case 2:
    return "PROGRAMME";
  case 3:
    return "MARK";
  default:
    return "";
  }
}

static const char *adv_query_field_label(int field_choice) {
  switch (field_choice) {
  case 1:
    return "Name";
  case 2:
    return "Programme";
  case 3:
    return "Mark";
  default:
    return "Unknown";
  }
}

static int adv_query_field_used(const AdvQuerySelection *selections,
                                size_t count, int field_choice) {
  for (size_t i = 0; i < count; i++) {
    if (selections[i].field == field_choice) {
      return 1;
    }
  }
  return 0;
}

static int adv_query_collect_fields(AdvQuerySelection *selections,
                                    size_t *selection_count) {
  size_t count = 0;
  while (count < 8) {
    int choice = adv_query_prompt_field();
    if (choice == 0) {
      if (count == 0) {
        printf("Cancelled advanced search.\n");
        *selection_count = 0;
        return 0;
      }
      printf("Use the Y/N prompt to finish.\n");
      continue;
    }
    if (adv_query_field_used(selections, count, choice)) {
      printf("You already selected %s. Pick another field.\n",
             adv_query_field_label(choice));
      continue;
    }
    selections[count].field = choice;
    selections[count].op = '=';
    selections[count].value[0] = '\0';
    count++;
    if (count >= 8) {
      printf("Reached maximum number of fields (8).\n");
      break;
    }
    if (!adv_query_yes_no("Add another field? (Y/N): ")) {
      break;
    }
  }
  *selection_count = count;
  return count > 0;
}

static void adv_query_collect_values(AdvQuerySelection *selections,
                                     size_t count) {
  for (size_t i = 0; i < count; i++) {
    if (selections[i].field == 3) {
      selections[i].op = adv_query_prompt_mark_op();
      adv_query_prompt_mark_value(selections[i].value,
                                  sizeof(selections[i].value));
    } else {
      adv_query_prompt_text(adv_query_field_label(selections[i].field),
                            selections[i].value, sizeof(selections[i].value));
    }
  }
}

static void adv_query_build_pipeline(const AdvQuerySelection *selections,
                                     size_t count, char *pipeline,
                                     size_t size) {
  pipeline[0] = '\0';
  for (size_t i = 0; i < count; i++) {
    char stage[512];
    if (selections[i].field == 3) {
      snprintf(stage, sizeof(stage), "MARK %c %s", selections[i].op,
               selections[i].value);
    } else {
      snprintf(stage, sizeof(stage), "GREP %s = \"%s\"",
               adv_query_field_token(selections[i].field), selections[i].value);
    }
    if (pipeline[0] != '\0') {
      strncat(pipeline, " | ", size - strlen(pipeline) - 1);
    }
    strncat(pipeline, stage, size - strlen(pipeline) - 1);
  }
}

// guided prompt entry point used by CMS/test harness to build and run a
// pipeline
AdvQueryStatus adv_query_run_prompt(StudentDatabase *db) {
  if (!db) {
    return ADV_QUERY_ERROR_INVALID_ARGUMENT;
  }
  if (!db->is_loaded || db->table_count == 0) {
    printf("CMS: Please OPEN the database before running advanced query.\n");
    return ADV_QUERY_ERROR_EMPTY_DATABASE;
  }

  AdvQuerySelection selections[8];
  size_t selection_count = 0;
  if (!adv_query_collect_fields(selections, &selection_count)) {
    return ADV_QUERY_OK;
  }

  adv_query_collect_values(selections, selection_count);

  char pipeline[256 * 8] = {0};
  adv_query_build_pipeline(selections, selection_count, pipeline,
                           sizeof(pipeline));

  return adv_query_execute(db, pipeline);
}
