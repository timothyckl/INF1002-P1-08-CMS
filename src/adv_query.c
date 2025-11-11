#include "adv_query.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strtok_r(str, delim, saveptr) strtok_s(str, delim, saveptr)
#endif

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

typedef enum {
  QUERY_FIELD_ID = 0,
  QUERY_FIELD_NAME,
  QUERY_FIELD_PROGRAMME,
  QUERY_FIELD_MARK,
  QUERY_FIELD_INVALID
} QueryField;

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

static QueryField parse_field(const char *token) {
  if (strcaseequal(token, "ID")) {
    return QUERY_FIELD_ID;
  }
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

static int contains_case_insensitive(const char *haystack,
                                     const char *needle) {
  if (!haystack || !needle || *needle == '\0') {
    return 0;
  }
  size_t needle_len = strlen(needle);
  for (const char *cursor = haystack; *cursor; cursor++) {
    size_t i = 0;
    while (cursor[i] &&
           tolower((unsigned char)cursor[i]) ==
               tolower((unsigned char)needle[i])) {
      i++;
      if (i == needle_len) {
        return 1;
      }
    }
  }
  return 0;
}

static int grep_matches(const StudentRecord *record, QueryField field,
                        const char *pattern) {
  if (!pattern) {
    return 0;
  }
  if (field == QUERY_FIELD_ID) {
    char id_buf[32];
    snprintf(id_buf, sizeof(id_buf), "%d", record->id);
    return contains_case_insensitive(id_buf, pattern);
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

static int filter_grep(StudentRecord **records, unsigned char *keep,
                       size_t count, QueryField field, const char *pattern) {
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

static int filter_mark(StudentRecord **records, unsigned char *keep,
                       size_t count, char op, double value) {
  for (size_t i = 0; i < count; i++) {
    if (keep[i] && !mark_matches(records[i], op, value)) {
      keep[i] = 0;
    }
  }
  return 1;
}

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
  if (field == QUERY_FIELD_INVALID || field == QUERY_FIELD_MARK ||
      field_used[field]) {
    return 0;
  }
  strip_quotes(expr);
  field_used[field] = 1;
  return filter_grep(records, keep, count, field, expr);
}

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
  return filter_mark(records, keep, count, op, value);
}

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
  if (db->table_count > 0 && total == 0) {
    printf("ADVQUERY: No records matched the pipeline.\n");
    return ADV_QUERY_OK;
  }
  if (total == 0) {
    free(records);
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

  int field_used[4] = {0};
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
    } else if (strcaseequal(cmd, "MARK")) {
      success = parse_mark_stage(records, keep, total, trimmed, field_used);
    } else if (strcaseequal(cmd, "FILTER")) {
      trimmed = trim(trimmed);
      char field_buf[32] = {0};
      size_t fidx = 0;
      while (trimmed[fidx] && !isspace((unsigned char)trimmed[fidx]) &&
             trimmed[fidx] != '=' && fidx < sizeof(field_buf) - 1) {
        field_buf[fidx] = trimmed[fidx];
        fidx++;
      }
      field_buf[fidx] = '\0';
      trimmed += fidx;
      if (parse_field(field_buf) != QUERY_FIELD_MARK) {
        success = 0;
      } else {
        success = parse_mark_stage(records, keep, total, trimmed, field_used);
      }
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
        printf("%d\t%s\t%s\t%.1f\n", r->id, r->name, r->prog, r->mark);
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
