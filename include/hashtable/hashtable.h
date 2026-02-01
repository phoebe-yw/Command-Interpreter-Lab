/* CS429 CI Lab
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Callback function type for freeing values stored in the hashtable.
 *
 * @param value Pointer to the value to be freed.
 */
typedef void (*HTValueFreeFn)(void *value);

/**
 * @brief Represents an entry in the hashtable.
 */
typedef struct ht_entry {
    char *key;
    void *value;
    struct ht_entry *next;
} HTEntry;

/**
 * @brief Represents a generic hashtable.
 */
typedef struct {
    /** Optional callback to free values (NULL if hashtable doesn't own values). */
    HTValueFreeFn free_value;
    // You will probably want to add more fields here.
    size_t size; // number of entries
    size_t capacity; // number of buckets
    HTEntry **buckets; // array of bucket pointers
} HashTable;

/**
 * @brief Initializes a hashtable.
 *
 * @param ht Pointer to the HashTable to initialize.
 * @param free_value Optional callback to free values when the hashtable is freed.
 *                   Pass NULL if the hashtable doesn't own the values.
 * @return true if the hashtable was successfully initialized, false otherwise.
 */
bool ht_init(HashTable *ht, HTValueFreeFn free_value);

/**
 * @brief Deletes an entry from the hashtable.
 *
 * @param ht Pointer to the hashtable.
 * @param key The key to delete.
 * @return true if the key was found and deleted, false if not found.
 */
bool ht_delete(HashTable *ht, const char *key);

/**
 * @brief Frees the resources associated with a hashtable.
 *
 * Releases all memory allocated for the hashtable, including entries and keys.
 * 
 * @note Will call the free_value callback for each value if provided. Otherwise,
 * values will not be freed.
 *
 * @param ht Pointer to the HashTable to free.
 */
void ht_free(HashTable *ht);

/**
 * @brief Inserts a key-value pair into the hashtable.
 *
 * Adds a new key-value pair to the hashtable. If a key already exists,
 * its value will be replaced with the new value.
 *
 * @param ht Pointer to the hashtable.
 * @param key The key string.
 * @param value Pointer to the value to store.
 * @return true if the pair was successfully added, false otherwise.
 */
bool ht_put(HashTable *ht, const char *key, void *value);

/**
 * @brief Retrieves a value from the hashtable.
 *
 * Searches the hashtable for the given key and returns the associated value.
 *
 * @param ht Pointer to the hashtable.
 * @param key The key to search for.
 * @return A pointer to the value if the key exists, or NULL if not found.
 */
void *ht_get(HashTable *ht, const char *key);

/**
 * @brief Returns the number of entries in the hashtable.
 *
 * @param ht Pointer to the hashtable.
 * @return The number of key-value pairs stored in the hashtable.
 */
size_t ht_size(HashTable *ht);

#endif
