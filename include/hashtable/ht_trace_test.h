/* CS429 CI Lab
 *
 * Students: Do Not Modify
 */

/** @file
 *  @cond DOXYGEN_IGNORE
 */

#ifndef HT_TRACE_TEST_H
#define HT_TRACE_TEST_H

#include <stdbool.h>
#include <stdint.h>

typedef enum { VAL_INT, VAL_STRING, VAL_BOOL, VAL_NULL } ValueType;

typedef struct {
  ValueType type;
  union {
    int64_t as_int;
    char *as_string;
    bool as_bool;
  };
} TaggedValue;

TaggedValue *tagged_value_int(int64_t val);
TaggedValue *tagged_value_string(const char *val);
TaggedValue *tagged_value_bool(bool val);
void tagged_value_free(void *val);

int run_trace_file(const char *path);

#endif

/** @endcond */