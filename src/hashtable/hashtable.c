/* CS429 CI Lab
 */

#include "hashtable/hashtable.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static unsigned long ht_hash_function(const char* s);

bool ht_init(HashTable* ht, HTValueFreeFn free_value) {
    if (!ht) {
        return false;
    }
    ht->free_value = free_value;
    ht->size = 0;       // no entries yet
    ht->capacity = 16;  // 16 buckets to start with
    ht->buckets = calloc(ht->capacity, sizeof(HTEntry*));
    if (!ht->buckets) {
        return false;
    }
    return true;
}

void ht_free(HashTable* ht) {
    if (!ht) {
        return;
    }
    for (size_t i = 0; i < ht->capacity; i++) {
        HTEntry* current = ht->buckets[i];
        while (current != NULL) {
            HTEntry* next = current->next;
            if (ht->free_value) {
                ht->free_value(current->value);
            }
            free(current->key);
            free(current);
            current = next;
        }
    }
    free(ht->buckets);
    ht->buckets = NULL;
    ht->size = 0;
    ht->capacity = 0;
}

/**
 * @brief Returns a hash of the specified key.
 *
 * @param s The string to hash.
 * @return The hash of the string.
 */
static unsigned long ht_hash_function(const char* s) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*s++)) {
        h = ((h << 5) + h) + c; /* h * 33 + c */
    }

    return h;
}

// helper function to duplicate a const string
static char* ht_strdup(const char* s) {
    size_t n = strlen(s) + 1;
    char* p = malloc(n);
    if (!p) {
        return NULL;
    }
    memcpy(p, s, n);
    return p;
}

// put key-value pair into hashtable
bool ht_put(HashTable* ht, const char* key, void* value) {
    if (!ht || !key) {
        return false;
    }
    // bucket index = hash(key) mod capacity
    size_t index = ht_hash_function(key) % ht->capacity;
    HTEntry* current = ht->buckets[index];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (ht->free_value) {
                ht->free_value(current->value);
            }
            // key exists, update
            current->value = value;
            return true;
        }
        current = current->next;
    }
    // key doesn't exist, create new entry
    HTEntry* new_entry = malloc(sizeof(HTEntry));
    if (!new_entry) {
        return false;
    }
    new_entry->key = ht_strdup(key);
    if (!new_entry->key) {
        free(new_entry);
        return false;
    }
    new_entry->value = value;              // set the value
    new_entry->next = ht->buckets[index];  // insert at head of bucket list
    ht->buckets[index] = new_entry;        // update bucket head
    ht->size++;
    return true;
}

// get the value by key in the hashtable
void* ht_get(HashTable* ht, const char* key) {
    if (!ht || !key) {
        return NULL;
    }
    size_t index = ht_hash_function(key) % ht->capacity;
    HTEntry* current = ht->buckets[index];
    while (current != NULL) {
        if (strcmp(key, current->key) == 0) {
            return current->value;
        } else {
            current = current->next;
        }
    }
    return NULL;
}

// delete the key-value pair from hashtable
bool ht_delete(HashTable* ht, const char* key) {
    if (!ht || !key) {
        return false;
    }
    size_t index = ht_hash_function(key) % ht->capacity;
    HTEntry* current = ht->buckets[index];
    HTEntry* prev = NULL;
    while (current != NULL) {
        if (strcmp(key, current->key) == 0) {
            // remove
            if (prev == NULL) {
                ht->buckets[index] = current->next;  // delete head
            } else {
                prev->next = current->next;
            }
            if (ht->free_value) {
                ht->free_value(current->value);
            }
            free(current->key);
            free(current);
            ht->size--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

size_t ht_size(HashTable* ht) {
    if (!ht) {
        return 0;
    }
    return ht->size;
}
