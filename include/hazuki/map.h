#ifndef HAZUKI_MAP_H_INCLUDED
#define HAZUKI_MAP_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

typedef struct hz_map hz_map;
typedef struct hz_map_iterator hz_map_iterator;

/**
 * Key hash function for hz_map. This function must satisfy the
 * condition that if cmp(a, b) == 0, then hash(a) == hash(b). This
 * function must always return the same value for any particular key.
 */
typedef size_t (*hz_map_hash_func)(const void *key);

/**
 * Comparator function for hz_map. Returns zero if the inputs are
 * equal, and non-zero otherwise. If a == b, this function must
 * return 0. This function must always return the same value for
 * any particular pair of inputs.
 */
typedef int (*hz_map_cmp_func)(const void *a, const void *b);

/**
 * Creates a new empty hashmap using the given hash and comparator
 * functions. You must free the hashmap using hz_map_free(hz_map *).
 */
hz_map *
hz_map_new(hz_map_hash_func hash, hz_map_cmp_func cmp);

/**
 * Creates a new hashmap by copying an existing one.
 * You must free the hashmap using hz_map_free(hz_map *).
 */
hz_map *
hz_map_copy(const hz_map *map);

/**
 * Frees a hashmap created by hz_map_new(hz_map_hash_func, hz_map_cmp_func)
 * or hz_map_copy(hz_map *). This does *NOT* free the elements contained
 * in the hashmap. Using the hashmap after deletion results in undefined
 * behavior.
 */
void
hz_map_free(hz_map *map);

/**
 * Gets the number of key-value pairs in the hashmap.
 */
size_t
hz_map_size(const hz_map *map);

/**
 * Removes all elements from the hashmap. This does *NOT* free the
 * elements contained in the hashmap.
 */
void
hz_map_clear(hz_map *map);

/**
 * Returns true if the key exists in the hashmap; false otherwise.
 * The key may be NULL.
 */
bool
hz_map_contains(const hz_map *map, const void *key);

/**
 * Gets the value associated with the given key. The key may be NULL.
 * If the key is not found in the hashmap, NULL is returned.
 */
void *
hz_map_get(const hz_map *map, const void *key);

/**
 * Sets the value associated with the given key. The key and/or value may be
 * NULL. Returns the previous value associated with the key, or NULL if there
 * was originally no associated value. Deallocating the key or value while
 * it is in the hashmap results in undefined behavior.
 */
void *
hz_map_put(hz_map *map, void *key, void *value);

/**
 * Removes a key-value pair from the hashmap and returns
 * the value associated with the key. The key may be NULL.
 */
void *
hz_map_remove(hz_map *map, const void *key);

/**
 * Returns an iterator that can be used to iterate over the elements in
 * the hashmap. The iterator is invalidated after any modifications to
 * the hashmap; continuing to use it results in an error. You must free
 * the iterator using hz_map_iterator_free(hz_map_iterator *).
 */
hz_map_iterator *
hz_map_iterator_new(const hz_map *map);

/**
 * Frees an iterator allocated by hz_map_iterator_new(hz_map *).
 * Using the iterator after deletion results in undefined behavior.
 */
void
hz_map_iterator_free(hz_map_iterator *it);

/**
 * Moves the iterator to the next element in the hashmap. If there are no
 * more elements in the hashmap, returns false and the key and value
 * parameters are unchanged. Otherwise, returns true and the key and
 * value parameters are filled with their corresponding values.
 */
bool
hz_map_iterator_next(hz_map_iterator *it, void **key, void **value);

#endif
