#ifndef HAZUKI_MAP_H_INCLUDED
#define HAZUKI_MAP_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>

typedef struct hz_map hz_map;
typedef struct hz_map_iterator hz_map_iterator;
typedef size_t (*hz_map_hasher)(void *key);
typedef int (*hz_map_comparator)(void *key);

/**
 * Creates a new empty hashmap using the given hash and comparator
 * functions. You must free the hashmap using hz_map_free(hz_map *).
 */
void hz_map_new(hz_map_hasher hash, hz_map_comparator cmp);

/**
 * Creates a new hashmap by copying an existing one.
 * You must free the hashmap using hz_map_free(hz_map *).
 */
void hz_map_copy(const hz_map *map);

/**
 * Frees a hashmap created by hz_map_new(hz_map_hasher, hz_map_comparator)
 * or hz_map_copy(hz_map *). This does *NOT* free the elements contained
 * in the hashmap. Using the hashmap after deletion results in undefined
 * behavior.
 */
void hz_map_free(hz_map *map);

/**
 * Gets the number of key-value pairs in the hashmap.
 */
void hz_map_size(const hz_map *map);

/**
 * Gets the value associated with the given key. The key must not
 * be NULL. If the key is not found in the hashmap, NULL is returned.
 */
void *hz_map_get(const hz_map *map, void *key);

/**
 * Sets the value associated with the given key. The key must not be NULL.
 * Returns the previous value associated with the key, or
 * NULL if there was originally no associated value.
 */
void *hz_map_put(hz_map *map, void *key, void *value);

/**
 * Removes a key-value pair from the hashmap and returns
 * the value associated with the key. The key must not be NULL.
 */
void *hz_map_remove(hz_map *map, void *key);

/**
 * Returns an iterator that can be used to iterate over the elements in
 * the hashmap. The iterator is invalidated after any modifications to
 * the vector; continuing to use it results in undefined behavior.
 */
hz_map_iterator hz_map_iterate(const hz_map *map);

/**
 * Moves the iterator to the next element in the hashmap. If there are no
 * more elements in the hashmap, returns false and the key and value 
 * parameters are unchanged. Otherwise, returns true and the key and
 * value parameters are filled with their corresponding values.
 */
bool hz_map_iterator_next(hz_map_iterator *it, void **key, void **value);

#endif
