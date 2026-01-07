/* CS429 CI Lab
 *
 * Students: Do Not Modify
 */

/** @file
 *  @cond DOXYGEN_IGNORE
 */

#define _POSIX_C_SOURCE 200809L
#include "hashtable/ht_trace_test.h"
#include "hashtable/hashtable.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 4096
#define MAX_TOKENS 16

static HashTable *g_ht = NULL;
static int g_passed = 0;
static int g_failed = 0;
static int g_line_num = 0;
static char g_line[MAX_LINE_LEN];

TaggedValue *tagged_value_int(int64_t val) {
  TaggedValue *tv = calloc(1, sizeof(TaggedValue));
  if (!tv)
    return NULL;
  tv->type = VAL_INT;
  tv->as_int = val;
  return tv;
}

TaggedValue *tagged_value_string(const char *val) {
  TaggedValue *tv = calloc(1, sizeof(TaggedValue));
  if (!tv)
    return NULL;
  tv->type = VAL_STRING;
  tv->as_string = strdup(val);
  if (!tv->as_string) {
    free(tv);
    return NULL;
  }
  return tv;
}

TaggedValue *tagged_value_bool(bool val) {
  TaggedValue *tv = calloc(1, sizeof(TaggedValue));
  if (!tv)
    return NULL;
  tv->type = VAL_BOOL;
  tv->as_bool = val;
  return tv;
}

void tagged_value_free(void *val) {
  if (!val)
    return;
  TaggedValue *tv = (TaggedValue *)val;
  if (tv->type == VAL_STRING) {
    free(tv->as_string);
  }
  free(tv);
}

static void pass(const char *msg) {
  printf("[PASS] %s\n", msg);
  g_passed++;
}

static void fail(const char *msg) {
  printf("[FAIL] line %d: %s\n", g_line_num, msg);
  g_failed++;
}

static int tokenize(char *line, char *tokens[], int max_tokens) {
  int count = 0;
  char *p = line;

  while (*p && count < max_tokens) {
    while (*p && isspace((unsigned char)*p))
      p++;
    if (!*p)
      break;

    if (*p == '"') {
      p++;
      tokens[count++] = p;
      while (*p && *p != '"')
        p++;
      if (*p == '"') {
        *p++ = '\0';
      }
    } else {
      tokens[count++] = p;
      while (*p && !isspace((unsigned char)*p))
        p++;
      if (*p) {
        *p++ = '\0';
      }
    }
  }
  return count;
}

static bool parse_value(const char *type_str, const char *val_str,
                        TaggedValue **out) {
  if (strcmp(type_str, "int") == 0) {
    int64_t v = strtoll(val_str, NULL, 10);
    *out = tagged_value_int(v);
    return *out != NULL;
  } else if (strcmp(type_str, "string") == 0) {
    *out = tagged_value_string(val_str);
    return *out != NULL;
  } else if (strcmp(type_str, "bool") == 0) {
    bool v = (strcmp(val_str, "true") == 0);
    *out = tagged_value_bool(v);
    return *out != NULL;
  } else if (strcmp(type_str, "null") == 0) {
    *out = NULL;
    return true;
  }
  return false;
}

static bool values_equal(TaggedValue *a, TaggedValue *b) {
  if (a == NULL && b == NULL)
    return true;
  if (a == NULL || b == NULL)
    return false;
  if (a->type != b->type)
    return false;

  switch (a->type) {
  case VAL_INT:
    return a->as_int == b->as_int;
  case VAL_STRING:
    return strcmp(a->as_string, b->as_string) == 0;
  case VAL_BOOL:
    return a->as_bool == b->as_bool;
  case VAL_NULL:
    return true;
  }
  return false;
}

static void value_to_string(TaggedValue *v, char *buf, size_t size) {
  if (!v) {
    snprintf(buf, size, "null");
    return;
  }
  switch (v->type) {
  case VAL_INT:
    snprintf(buf, size, "%ld", (long)v->as_int);
    break;
  case VAL_STRING:
    snprintf(buf, size, "\"%s\"", v->as_string);
    break;
  case VAL_BOOL:
    snprintf(buf, size, "%s", v->as_bool ? "true" : "false");
    break;
  case VAL_NULL:
    snprintf(buf, size, "null");
    break;
  }
}

static void cmd_init(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  if (g_ht) {
    fail("init: hashtable already initialized");
    return;
  }

  g_ht = calloc(1, sizeof(HashTable));
  if (!g_ht) {
    fail("init: failed to allocate hashtable");
    return;
  }

  if (!ht_init(g_ht, tagged_value_free)) {
    free(g_ht);
    g_ht = NULL;
    fail("init: ht_init failed");
    return;
  }

  pass("init");
}

static void cmd_put(int argc, char *argv[]) {
  if (argc < 4) {
    fail("put: expected 'put <key> <type> <value>'");
    return;
  }
  if (!g_ht) {
    fail("put: hashtable not initialized");
    return;
  }

  const char *key = argv[1];
  const char *type_str = argv[2];
  const char *val_str = argv[3];
  TaggedValue *tv = NULL;

  if (!parse_value(type_str, val_str, &tv)) {
    fail("put: failed to parse value");
    return;
  }

  if (!ht_put(g_ht, key, tv)) {
    tagged_value_free(tv);
    fail("put: ht_put failed");
    return;
  }

  char msg[256];
  char val_buf[128];
  value_to_string(tv, val_buf, sizeof(val_buf));
  snprintf(msg, sizeof(msg), "put %s %s %s", key, type_str, val_buf);
  pass(msg);
}

static void cmd_get(int argc, char *argv[]) {
  if (argc < 4) {
    fail("get: expected 'get <key> <type> <expected>'");
    return;
  }
  if (!g_ht) {
    fail("get: hashtable not initialized");
    return;
  }

  const char *key = argv[1];
  const char *type_str = argv[2];
  const char *expected_str = argv[3];
  TaggedValue *expected = NULL;

  if (!parse_value(type_str, expected_str, &expected)) {
    fail("get: failed to parse expected value");
    return;
  }

  TaggedValue *actual = (TaggedValue *)ht_get(g_ht, key);

  if (values_equal(actual, expected)) {
    char msg[256];
    char val_buf[128];
    value_to_string(expected, val_buf, sizeof(val_buf));
    snprintf(msg, sizeof(msg), "get %s %s %s", key, type_str, val_buf);
    pass(msg);
  } else {
    char msg[512];
    char exp_buf[128], act_buf[128];
    value_to_string(expected, exp_buf, sizeof(exp_buf));
    value_to_string(actual, act_buf, sizeof(act_buf));
    snprintf(msg, sizeof(msg), "get %s: expected %s, got %s", key, exp_buf,
             act_buf);
    fail(msg);
  }

  tagged_value_free(expected);
}

static void cmd_del(int argc, char *argv[]) {
  if (argc < 2) {
    fail("del: expected 'del <key>'");
    return;
  }
  if (!g_ht) {
    fail("del: hashtable not initialized");
    return;
  }

  const char *key = argv[1];
  ht_delete(g_ht, key);

  char msg[256];
  snprintf(msg, sizeof(msg), "del %s", key);
  pass(msg);
}

static void cmd_size(int argc, char *argv[]) {
  if (argc < 2) {
    fail("size: expected 'size <expected>'");
    return;
  }
  if (!g_ht) {
    fail("size: hashtable not initialized");
    return;
  }

  size_t expected = (size_t)atoi(argv[1]);
  size_t actual = ht_size(g_ht);

  if (actual == expected) {
    char msg[64];
    snprintf(msg, sizeof(msg), "size %zu", expected);
    pass(msg);
  } else {
    char msg[128];
    snprintf(msg, sizeof(msg), "size: expected %zu, got %zu", expected, actual);
    fail(msg);
  }
}

static void cmd_free_ht(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  if (!g_ht) {
    fail("free: hashtable not initialized");
    return;
  }

  ht_free(g_ht);
  free(g_ht);
  g_ht = NULL;

  pass("free");
}

static void process_line(char *line) {
  char *tokens[MAX_TOKENS];
  int count = tokenize(line, tokens, MAX_TOKENS);

  if (count == 0)
    return;

  const char *cmd = tokens[0];

  if (strcmp(cmd, "init") == 0) {
    cmd_init(count, tokens);
  } else if (strcmp(cmd, "put") == 0) {
    cmd_put(count, tokens);
  } else if (strcmp(cmd, "get") == 0) {
    cmd_get(count, tokens);
  } else if (strcmp(cmd, "del") == 0) {
    cmd_del(count, tokens);
  } else if (strcmp(cmd, "size") == 0) {
    cmd_size(count, tokens);
  } else if (strcmp(cmd, "free") == 0) {
    cmd_free_ht(count, tokens);
  } else {
    char msg[256];
    snprintf(msg, sizeof(msg), "unknown command: %s", cmd);
    fail(msg);
  }
}

int run_trace_file(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Error: cannot open file '%s'\n", path);
    return 1;
  }

  g_passed = 0;
  g_failed = 0;
  g_line_num = 0;

  while (fgets(g_line, sizeof(g_line), f)) {
    g_line_num++;
    size_t len = strlen(g_line);
    if (len > 0 && g_line[len - 1] == '\n') {
      g_line[len - 1] = '\0';
    }
    if (g_line[0] == '\0')
      continue;

    process_line(g_line);
  }

  fclose(f);

  if (g_ht) {
    ht_free(g_ht);
    free(g_ht);
    g_ht = NULL;
  }

  printf("\nRESULT: %d/%d tests passed\n", g_passed, g_passed + g_failed);

  return (g_failed > 0) ? 1 : 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <trace_file>\n", argv[0]);
    return 1;
  }

  return run_trace_file(argv[1]);
}

/** @endcond */