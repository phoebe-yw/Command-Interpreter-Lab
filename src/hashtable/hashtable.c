/* CS429 CI Lab
 */

#include "hashtable/hashtable.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static unsigned long ht_hash_function(const char *s);

bool ht_init(HashTable *ht, HTValueFreeFn free_value) {
  return false;
}

void ht_free(HashTable *ht) {}

/**
 * @brief Returns a hash of the specified key.
 *
 * @param s The string to hash.
 * @return The hash of the string.
 */
static unsigned long ht_hash_function(const char *s) {
  unsigned long h = 5381;
  int c;
  while ((c = (unsigned char)*s++)) {
    h = ((h << 5) + h) + c; /* h * 33 + c */
  }

  return h;
}

bool ht_put(HashTable *ht, const char *key, void *value) { return false; }

void *ht_get(HashTable *ht, const char *key) { return NULL; }

bool ht_delete(HashTable *ht, const char *key) { return false; }

size_t ht_size(HashTable *ht) { return 0; }
