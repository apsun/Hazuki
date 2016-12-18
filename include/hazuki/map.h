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
 * equal, and non-zero otherwise. This function must always return
 * the same value for any particular pair of inputs.
 */
typedef int (*hz_map_cmp_func)(const void *a, const void *b);

/**
 * Creates a new empty hashmap with the given key and value sizes and
 * hash and comparator functions. You must free the returned hashmap
 * using hz_map_free().
 */
hz_map *
hz_map_new(size_t key_size, size_t value_size,
           hz_map_hash_func hash_func, hz_map_cmp_func cmp_func);

/**
 * Creates a new hashmap by copying an existing one.
 * You must free the returned hashmap using hz_map_free().
 */
hz_map *
hz_map_copy(const hz_map *map);

/**
 * Frees a hashmap created by hz_map_new() or hz_map_copy(). This does
 * *NOT* free the elements contained in the hashmap. Using the hashmap
 * after deletion results in undefined behavior.
 */
void
hz_map_free(hz_map *map);

/**
 * Gets the number of entries in the hashmap.
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
 * Gets the value associated with the given key. Returns true if the entry
 * exists in the map, and false otherwise. If the entry exists and out_value
 * is not NULL, the value is written to out_value.
 */
bool
hz_map_get(const hz_map *map, const void *key, void *out_value);

/**
 * Sets the value associated with the given key. Returns true if this replaces
 * an existing value, and false otherwise. If a value was replaced and
 * out_value is not NULL, the previous value is written to out_value.
 */
bool
hz_map_put(hz_map *map, const void *key, const void *value, void *out_value);

/**
 * Removes the entry associated with the given key. Returns true if
 * the entry exists in the map, and false otherwise. If the entry exists
 * and out_value is not NULL, the removed value is written to out_value.
 */
bool
hz_map_remove(hz_map *map, const void *key, void *out_value);

/**
 * Creates an iterator that can be used to iterate over the elements in
 * the hashmap. The iterator is invalidated after any modifications to
 * the hashmap; continuing to use it results in an error. You must free
 * the returned iterator using hz_map_iterator_free().
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
 * value parameters are filled with their corresponding values. You may
 * pass NULL for the key or value parameters to ignore their value.
 */
bool
hz_map_iterator_next(hz_map_iterator *it, void *key, void *value);

#endif
