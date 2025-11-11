#include "adv_query.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strtok_r(str, delim, saveptr) strtok_s(str, delim, saveptr)
#endif

#define MAX_FIELD_NAME 32
#define MAX_VALUE_LEN 128

typedef enum {
  ADV_FIELD_ID = 0,
  ADV_FIELD_NAME,
  ADV_FIELD_PROGRAMME,
  ADV_FIELD_MARK,
  ADV_FIELD_COUNT
} AdvField;

typedef enum {
  ADV_OP_EQ,
  ADV_OP_NEQ,
  ADV_OP_GT,
  ADV_OP_GTE,
  ADV_OP_LT,
  ADV_OP_LTE
} AdvCompareOp;

typedef struct {
  StudentRecord **items;
  size_t count;
  size_t capacity;
} RecordBuffer;

typedef struct {
  AdvField field;
  int descending;
} SortContext;

static SortContext g_sort_ctx;

static char *adv_strdup(const char *src) {
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

static char *trim(char *str) {
  if (!str) {
    return str;
  }
  while (isspace((unsigned char)*str)) {
    str++;
  }
  if (*str == '\0') {
    return str;
  }
  char *end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) {
    *end = '\0';
    end--;
  }
  return str;
}

static int strcasecmp_local(const char *a, const char *b) {
  while (*a && *b) {
    int da = tolower((unsigned char)*a);
    int db = tolower((unsigned char)*b);
    if (da != db) {
      return da - db;
    }
    a++;
    b++;
  }
  return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static int strncaseequal(const char *a, const char *b, size_t n) {
  for (size_t i = 0; i < n; i++) {
    unsigned char ca = (unsigned char)a[i];
    unsigned char cb = (unsigned char)b[i];
    int da = tolower(ca);
    int db = tolower(cb);
    if (da != db) {
      return 0;
    }
    if (ca == '\0' || cb == '\0') {
      break;
    }
  }
  return 1;
}

static int strcaseequal(const char *a, const char *b) {
  size_t len_a = strlen(a);
  size_t len_b = strlen(b);
  if (len_a != len_b) {
    return 0;
  }
  return strncaseequal(a, b, len_a);
}

static AdvField parse_field(const char *token) {
  if (strcaseequal(token, "ID")) {
    return ADV_FIELD_ID;
  }
  if (strcaseequal(token, "NAME")) {
    return ADV_FIELD_NAME;
  }
  if (strcaseequal(token, "PROGRAMME") || strcaseequal(token, "PROGRAM")) {
    return ADV_FIELD_PROGRAMME;
  }
  if (strcaseequal(token, "MARK") || strcaseequal(token, "MARKS")) {
    return ADV_FIELD_MARK;
  }
  return ADV_FIELD_COUNT;
}

static void strip_quotes(char *value) {
  if (!value) {
    return;
  }
  size_t len = strlen(value);
  if (len >= 2 && value[0] == '"' && value[len - 1] == '"') {
    value[len - 1] = '\0';
    memmove(value, value + 1, len - 1);
  }
}

static AdvCompareOp parse_operator(const char *expr, size_t *op_len) {
  if (strncmp(expr, ">=", 2) == 0) {
    *op_len = 2;
    return ADV_OP_GTE;
  }
  if (strncmp(expr, "<=", 2) == 0) {
    *op_len = 2;
    return ADV_OP_LTE;
  }
  if (strncmp(expr, "!=", 2) == 0) {
    *op_len = 2;
    return ADV_OP_NEQ;
  }
  if (*expr == '>') {
    *op_len = 1;
    return ADV_OP_GT;
  }
  if (*expr == '<') {
    *op_len = 1;
    return ADV_OP_LT;
  }
  if (*expr == '=') {
    *op_len = 1;
    return ADV_OP_EQ;
  }
  *op_len = 0;
  return ADV_OP_EQ;
}

static int compare_numeric(double lhs, double rhs, AdvCompareOp op) {
  switch (op) {
  case ADV_OP_EQ:
    return lhs == rhs;
  case ADV_OP_NEQ:
    return lhs != rhs;
  case ADV_OP_GT:
    return lhs > rhs;
  case ADV_OP_GTE:
    return lhs >= rhs;
  case ADV_OP_LT:
    return lhs < rhs;
  case ADV_OP_LTE:
    return lhs <= rhs;
  default:
    return 0;
  }
}

static int compare_string(const char *lhs, const char *rhs, AdvCompareOp op) {
  int eq = strcaseequal(lhs, rhs);
  if (op == ADV_OP_EQ) {
    return eq;
  }
  if (op == ADV_OP_NEQ) {
    return !eq;
  }
  return 0;
}

static int contains_case_insensitive(const char *haystack, const char *needle) {
  size_t needle_len = strlen(needle);
  if (needle_len == 0) {
    return 0;
  }
  for (const char *p = haystack; *p; p++) {
    size_t i = 0;
    while (p[i] && i < needle_len) {
      char a = tolower((unsigned char)p[i]);
      char b = tolower((unsigned char)needle[i]);
      if (a != b) {
        break;
      }
      i++;
    }
    if (i == needle_len) {
      return 1;
    }
  }
  return 0;
}

static int id_contains_pattern(int value, const char *pattern_str) {
  if (!pattern_str || *pattern_str == '\0') {
    return 0;
  }
  for (const char *c = pattern_str; *c; c++) {
    if (!isdigit((unsigned char)*c)) {
      return 0;
    }
  }
  int pattern = 0;
  int digits = 0;
  for (const char *c = pattern_str; *c; c++) {
    pattern = pattern * 10 + (*c - '0');
    digits++;
  }
  int factor = 1;
  for (int i = 0; i < digits; i++) {
    factor *= 10;
  }
  if (pattern == 0) {
    if (value == 0) {
      return 1;
    }
    int temp = value;
    while (temp > 0) {
      if (temp % 10 == 0) {
        return 1;
      }
      temp /= 10;
    }
    return 0;
  }
  int temp = value;
  if (temp == pattern) {
    return 1;
  }
  while (temp > 0) {
    if (temp % factor == pattern) {
      return 1;
    }
    temp /= 10;
  }
  return 0;
}

static void buffer_free(RecordBuffer *buffer) {
  if (!buffer) {
    return;
  }
  free(buffer->items);
  buffer->items = NULL;
  buffer->count = 0;
  buffer->capacity = 0;
}

static AdvQueryStatus buffer_init_from_db(RecordBuffer *buffer,
                                          StudentDatabase *db) {
  size_t total = 0;
  for (size_t t = 0; t < db->table_count; t++) {
    StudentTable *table = db->tables[t];
    if (table) {
      total += table->record_count;
    }
  }
  if (total == 0) {
    buffer->items = NULL;
    buffer->count = 0;
    buffer->capacity = 0;
    return ADV_QUERY_OK;
  }
  buffer->items = malloc(total * sizeof(StudentRecord *));
  if (!buffer->items) {
    buffer->count = 0;
    buffer->capacity = 0;
    return ADV_QUERY_ERROR_MEMORY;
  }
  buffer->count = 0;
  buffer->capacity = total;
  for (size_t t = 0; t < db->table_count; t++) {
    StudentTable *table = db->tables[t];
    if (!table) {
      continue;
    }
    for (size_t r = 0; r < table->record_count; r++) {
      buffer->items[buffer->count++] = &table->records[r];
    }
  }
  return ADV_QUERY_OK;
}

static void buffer_apply_numeric_filter(RecordBuffer *buffer, AdvField field,
                                        AdvCompareOp op, double value) {
  size_t write = 0;
  for (size_t i = 0; i < buffer->count; i++) {
    StudentRecord *record = buffer->items[i];
    double lhs = 0.0;
    if (field == ADV_FIELD_ID) {
      lhs = (double)record->id;
    } else {
      lhs = (double)record->mark;
    }
    if (compare_numeric(lhs, value, op)) {
      buffer->items[write++] = record;
    }
  }
  buffer->count = write;
}

static void buffer_apply_string_filter(RecordBuffer *buffer, AdvField field,
                                       AdvCompareOp op, const char *value) {
  size_t write = 0;
  for (size_t i = 0; i < buffer->count; i++) {
    StudentRecord *record = buffer->items[i];
    const char *lhs =
        (field == ADV_FIELD_NAME) ? record->name : record->prog;
    if (compare_string(lhs, value, op)) {
      buffer->items[write++] = record;
    }
  }
  buffer->count = write;
}

static void buffer_apply_grep(RecordBuffer *buffer, AdvField field,
                              const char *pattern) {
  size_t write = 0;
  for (size_t i = 0; i < buffer->count; i++) {
    StudentRecord *record = buffer->items[i];
    int keep = 0;
    if (field == ADV_FIELD_ID) {
      keep = id_contains_pattern(record->id, pattern);
    } else if (field == ADV_FIELD_NAME) {
      keep = contains_case_insensitive(record->name, pattern);
    } else if (field == ADV_FIELD_PROGRAMME) {
      keep = contains_case_insensitive(record->prog, pattern);
    }
    if (keep) {
      buffer->items[write++] = record;
    }
  }
  buffer->count = write;
}

static void buffer_apply_limit(RecordBuffer *buffer, size_t limit) {
  if (buffer->count > limit) {
    buffer->count = limit;
  }
}

static int record_compare(const void *lhs, const void *rhs) {
  StudentRecord *const *a = lhs;
  StudentRecord *const *b = rhs;
  StudentRecord *ra = *a;
  StudentRecord *rb = *b;
  int result = 0;
  switch (g_sort_ctx.field) {
  case ADV_FIELD_ID:
    if (ra->id < rb->id) {
      result = -1;
    } else if (ra->id > rb->id) {
      result = 1;
    }
    break;
  case ADV_FIELD_MARK:
    if (ra->mark < rb->mark) {
      result = -1;
    } else if (ra->mark > rb->mark) {
      result = 1;
    }
    break;
  case ADV_FIELD_NAME:
    result = strcasecmp_local(ra->name, rb->name);
    break;
  case ADV_FIELD_PROGRAMME:
    result = strcasecmp_local(ra->prog, rb->prog);
    break;
  default:
    result = 0;
    break;
  }
  if (g_sort_ctx.descending) {
    result = -result;
  }
  return result;
}

static void buffer_apply_sort(RecordBuffer *buffer, AdvField field,
                              int descending) {
  if (buffer->count <= 1) {
    return;
  }
  g_sort_ctx.field = field;
  g_sort_ctx.descending = descending;
  qsort(buffer->items, buffer->count, sizeof(StudentRecord *), record_compare);
}

static void print_table_header(const int *project_flags, int project_active) {
  int first = 1;
  for (int field = 0; field < ADV_FIELD_COUNT; field++) {
    if (!project_active || project_flags[field]) {
      if (!first) {
        printf("\t");
      }
      switch (field) {
      case ADV_FIELD_ID:
        printf("ID");
        break;
      case ADV_FIELD_NAME:
        printf("Name");
        break;
      case ADV_FIELD_PROGRAMME:
        printf("Programme");
        break;
      case ADV_FIELD_MARK:
        printf("Mark");
        break;
      default:
        break;
      }
      first = 0;
    }
  }
  printf("\n");
}

static void print_table_row(StudentRecord *record, const int *project_flags,
                            int project_active) {
  int first = 1;
  for (int field = 0; field < ADV_FIELD_COUNT; field++) {
    if (!project_active || project_flags[field]) {
      if (!first) {
        printf("\t");
      }
      switch (field) {
      case ADV_FIELD_ID:
        printf("%d", record->id);
        break;
      case ADV_FIELD_NAME:
        printf("%s", record->name);
        break;
      case ADV_FIELD_PROGRAMME:
        printf("%s", record->prog);
        break;
      case ADV_FIELD_MARK:
        printf("%.1f", record->mark);
        break;
      default:
        break;
      }
      first = 0;
    }
  }
  printf("\n");
}

static AdvQueryStatus parse_filter_stage(char *expr, RecordBuffer *buffer) {
  expr = trim(expr);
  if (*expr == '\0') {
    return ADV_QUERY_ERROR_PARSE;
  }

  char field_token[MAX_FIELD_NAME] = {0};
  size_t idx = 0;
  while (expr[idx] && !isspace((unsigned char)expr[idx]) && expr[idx] != '=' &&
         expr[idx] != '!' && expr[idx] != '>' && expr[idx] != '<') {
    if (idx < sizeof(field_token) - 1) {
      field_token[idx] = expr[idx];
    }
    idx++;
  }
  field_token[idx < sizeof(field_token) ? idx : sizeof(field_token) - 1] = '\0';

  AdvField field = parse_field(field_token);
  if (field == ADV_FIELD_COUNT) {
    return ADV_QUERY_ERROR_PARSE;
  }

  expr += idx;
  expr = trim(expr);
  size_t op_len = 0;
  AdvCompareOp op = parse_operator(expr, &op_len);
  if (op_len == 0) {
    return ADV_QUERY_ERROR_PARSE;
  }
  expr += op_len;
  expr = trim(expr);

  char value_buf[MAX_VALUE_LEN];
  strncpy(value_buf, expr, sizeof(value_buf) - 1);
  value_buf[sizeof(value_buf) - 1] = '\0';
  strip_quotes(value_buf);

  if (field == ADV_FIELD_ID || field == ADV_FIELD_MARK) {
    char *endptr = NULL;
    double parsed = strtod(value_buf, &endptr);
    if (endptr == value_buf || *endptr != '\0') {
      return ADV_QUERY_ERROR_PARSE;
    }
    buffer_apply_numeric_filter(buffer, field, op, parsed);
  } else {
    if (op != ADV_OP_EQ && op != ADV_OP_NEQ) {
      return ADV_QUERY_ERROR_PARSE;
    }
    buffer_apply_string_filter(buffer, field, op, value_buf);
  }

  return ADV_QUERY_OK;
}

static AdvQueryStatus parse_grep_stage(char *expr, RecordBuffer *buffer) {
  expr = trim(expr);
  if (*expr == '\0') {
    return ADV_QUERY_ERROR_PARSE;
  }
  char *equals = strchr(expr, '=');
  if (!equals) {
    return ADV_QUERY_ERROR_PARSE;
  }
  *equals = '\0';
  char *field_str = trim(expr);
  char *pattern = trim(equals + 1);
  strip_quotes(pattern);

  AdvField field = parse_field(field_str);
  if (field == ADV_FIELD_COUNT) {
    return ADV_QUERY_ERROR_PARSE;
  }

  if (field == ADV_FIELD_MARK) {
    return ADV_QUERY_ERROR_PARSE;
  }

  buffer_apply_grep(buffer, field, pattern);
  return ADV_QUERY_OK;
}

static AdvQueryStatus parse_sort_stage(char *expr, RecordBuffer *buffer) {
  expr = trim(expr);
  if (*expr == '\0') {
    return ADV_QUERY_ERROR_PARSE;
  }
  char field_token[MAX_FIELD_NAME] = {0};
  size_t idx = 0;
  while (expr[idx] && !isspace((unsigned char)expr[idx])) {
    if (idx < sizeof(field_token) - 1) {
      field_token[idx] = expr[idx];
    }
    idx++;
  }
  field_token[idx < sizeof(field_token) ? idx : sizeof(field_token) - 1] = '\0';

  AdvField field = parse_field(field_token);
  if (field == ADV_FIELD_COUNT) {
    return ADV_QUERY_ERROR_PARSE;
  }
  expr += idx;
  expr = trim(expr);

  int descending = 0;
  if (*expr != '\0') {
    if (strcaseequal(expr, "DESC")) {
      descending = 1;
    } else if (!strcaseequal(expr, "ASC")) {
      return ADV_QUERY_ERROR_PARSE;
    }
  }
  buffer_apply_sort(buffer, field, descending);
  return ADV_QUERY_OK;
}

static AdvQueryStatus parse_limit_stage(char *expr, RecordBuffer *buffer) {
  expr = trim(expr);
  if (*expr == '\0') {
    return ADV_QUERY_ERROR_PARSE;
  }
  char *endptr = NULL;
  long value = strtol(expr, &endptr, 10);
  if (endptr == expr || *endptr != '\0' || value < 0) {
    return ADV_QUERY_ERROR_PARSE;
  }
  buffer_apply_limit(buffer, (size_t)value);
  return ADV_QUERY_OK;
}

static AdvQueryStatus parse_project_stage(char *expr, int *project_flags,
                                          int *project_active) {
  expr = trim(expr);
  if (*expr == '\0') {
    return ADV_QUERY_ERROR_PARSE;
  }
  for (int i = 0; i < ADV_FIELD_COUNT; i++) {
    project_flags[i] = 0;
  }
  int matched = 0;
  char *cursor = expr;
  while (*cursor) {
    char *comma = strchr(cursor, ',');
    if (comma) {
      *comma = '\0';
    }
    char *field_name = trim(cursor);
    AdvField field = parse_field(field_name);
    if (field == ADV_FIELD_COUNT) {
      return ADV_QUERY_ERROR_PARSE;
    }
    project_flags[field] = 1;
    matched = 1;
    if (!comma) {
      break;
    }
    cursor = comma + 1;
  }
  if (!matched) {
    return ADV_QUERY_ERROR_PARSE;
  }
  *project_active = 1;
  return ADV_QUERY_OK;
}

AdvQueryStatus adv_query_execute(StudentDatabase *db, const char *pipeline) {
  if (!db || !pipeline) {
    return ADV_QUERY_ERROR_INVALID_ARGUMENT;
  }
  if (db->table_count == 0) {
    return ADV_QUERY_ERROR_EMPTY_DATABASE;
  }

  RecordBuffer buffer = {0};
  AdvQueryStatus status = buffer_init_from_db(&buffer, db);
  if (status != ADV_QUERY_OK) {
    return status;
  }

  int project_flags[ADV_FIELD_COUNT] = {0};
  int project_active = 0;

  char *working = adv_strdup(pipeline);
  if (!working) {
    buffer_free(&buffer);
    return ADV_QUERY_ERROR_MEMORY;
  }

  char *stage_ctx = NULL;
  char *stage = strtok_r(working, "|", &stage_ctx);
  while (stage) {
    char *trimmed = trim(stage);
    if (*trimmed != '\0') {
      if (strncaseequal(trimmed, "FILTER", 6) &&
          (trimmed[6] == '\0' || isspace((unsigned char)trimmed[6]))) {
        char *expr = trimmed + 6;
        status = parse_filter_stage(expr, &buffer);
      } else if (strncaseequal(trimmed, "GREP", 4) &&
                 (trimmed[4] == '\0' || isspace((unsigned char)trimmed[4]))) {
        char *expr = trimmed + 4;
        status = parse_grep_stage(expr, &buffer);
      } else if (strncaseequal(trimmed, "SORT", 4) &&
                 (trimmed[4] == '\0' || isspace((unsigned char)trimmed[4]))) {
        char *expr = trimmed + 4;
        status = parse_sort_stage(expr, &buffer);
      } else if (strncaseequal(trimmed, "LIMIT", 5) &&
                 (trimmed[5] == '\0' || isspace((unsigned char)trimmed[5]))) {
        char *expr = trimmed + 5;
        status = parse_limit_stage(expr, &buffer);
      } else if (strncaseequal(trimmed, "PROJECT", 7) &&
                 (trimmed[7] == '\0' || isspace((unsigned char)trimmed[7]))) {
        char *expr = trimmed + 7;
        status = parse_project_stage(expr, project_flags, &project_active);
      } else if (*trimmed != '\0') {
        status = ADV_QUERY_ERROR_PARSE;
      }

      if (status != ADV_QUERY_OK) {
        break;
      }
    }
    stage = strtok_r(NULL, "|", &stage_ctx);
  }

  if (status == ADV_QUERY_OK) {
    if (buffer.count == 0) {
      printf("ADVQUERY: No records matched the pipeline.\n");
    } else {
      print_table_header(project_flags, project_active);
      for (size_t i = 0; i < buffer.count; i++) {
        print_table_row(buffer.items[i], project_flags, project_active);
      }
      printf("Total: %zu record(s)\n", buffer.count);
    }
  }

  free(working);
  buffer_free(&buffer);
  return status;
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
