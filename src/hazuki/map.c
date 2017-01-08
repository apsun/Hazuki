#include "hazuki/map.h"
#include "hazuki/utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Initial capacity for the hashmap. Must be an integer > 0.
 */
#define INITIAL_CAPACITY 8

/**
 * Factor by which to scale the hashmap's bucket array when the load
 * factor is reached. Must be > 1.
 */
#define SCALING_FACTOR 1.5

/**
 * When the number of entries divided by the number of buckets reaches
 * this value, the hashmap is resized. Must be > 0.
 */
#define LOAD_FACTOR 0.75

typedef struct hz_map_entry
{
    struct hz_map_entry *next;
    size_t hash;
    char buffer[];
} hz_map_entry;

struct hz_map
{
    size_t key_size;
    size_t value_size;
    hz_map_hash_func hash_func;
    hz_map_cmp_func cmp_func;
    size_t size;
    size_t bucket_count;
    hz_map_entry **buckets;
    unsigned int mod_count;
};

struct hz_map_iterator
{
    const hz_map *map;
    size_t bucket_index;
    hz_map_entry *current_entry;
    unsigned int mod_count;
};

static void
hz_map_touch(hz_map *map)
{
    map->mod_count++;
}

static size_t
hz_map_hash_key(const hz_map *map, const void *key)
{
    // Key hash function. If desired, a secondary key hashing round
    // can be applied here to reduce the risk of collisions.
    return map->hash_func(key);
}

static size_t
hz_map_get_bucket_index(size_t hash, size_t bucket_count)
{
    // We compute the index as the hash modulo the number of buckets.
    // If the bucket count is always a power of 2, we can use
    // hash & (bucket_count - 1) instead which is a bit faster.
    return hash % bucket_count;
}

static hz_map_entry *
hz_map_entry_alloc(const hz_map *map)
{
    // sizeof(hz_map_entry) gives the size without the flexible array,
    // so we need to add the size of the key and value.
    size_t size = sizeof(hz_map_entry) + map->key_size + map->value_size;
    hz_map_entry *entry = hz_malloc(1, size);
    return entry;
}

static void *
hz_map_entry_key_offset(const hz_map *map, const hz_map_entry *entry)
{
    // Technically we don't need the map argument since the key
    // is at buffer[0], but we take it anyways for consistency
    // with the value offset function.
    (void)map;
    return (void *)(entry->buffer);
}

static void *
hz_map_entry_value_offset(const hz_map *map, const hz_map_entry *entry)
{
    return (void *)(entry->buffer + map->key_size);
}

static void
hz_map_entry_get_key(
    const hz_map *map,
    hz_map_entry *entry,
    void *out_key)
{
    void *key = hz_map_entry_key_offset(map, entry);
    hz_memcpy(out_key, key, 1, map->key_size);
}

static void
hz_map_entry_get_value(
    const hz_map *map,
    hz_map_entry *entry,
    void *out_value)
{
    void *value = hz_map_entry_value_offset(map, entry);
    hz_memcpy(out_value, value, 1, map->value_size);
}

static void
hz_map_entry_set_key(
    const hz_map *map,
    hz_map_entry *entry,
    const void *new_key)
{
    void *key = hz_map_entry_key_offset(map, entry);
    hz_memcpy(key, new_key, 1, map->key_size);
}

static void
hz_map_entry_set_value(
    const hz_map *map,
    hz_map_entry *entry,
    const void *new_value)
{
    void *value = hz_map_entry_value_offset(map, entry);
    hz_memcpy(value, new_value, 1, map->value_size);
}

static bool
hz_map_entry_matches(
    const hz_map *map,
    const hz_map_entry *entry,
    size_t hash,
    const void *key)
{
    // If the hashes don't match, we don't need to compare the keys.
    if (entry->hash != hash) {
        return false;
    }

    // If the hashes match, we still need to check that the keys are equal.
    void *entry_key = hz_map_entry_key_offset(map, entry);
    int cmp;
    if (map->cmp_func == NULL) {
        cmp = hz_memcmp(key, entry_key, 1, map->key_size);
    } else {
        cmp = map->cmp_func(key, entry_key);
    }
    return cmp == 0;
}

static hz_map_entry *
hz_map_find_entry(const hz_map *map, size_t hash, const void *key)
{
    if (map->bucket_count == 0) {
        return NULL;
    }
    size_t index = hz_map_get_bucket_index(hash, map->bucket_count);
    hz_map_entry *entry = map->buckets[index];
    while (entry != NULL) {
        if (hz_map_entry_matches(map, entry, hash, key)) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

static hz_map_entry *
hz_map_entry_new(
    const hz_map *map,
    size_t hash,
    const void *key,
    const void *value)
{
    hz_map_entry *entry = hz_map_entry_alloc(map);
    entry->next = NULL;
    entry->hash = hash;
    hz_map_entry_set_key(map, entry, key);
    hz_map_entry_set_value(map, entry, value);
    return entry;
}

static hz_map_entry *
hz_map_entry_copy(const hz_map *map, const hz_map_entry *entry)
{
    // "Pop" each value from the old list, clone it, and "push"
    // it at the head of the new list. This has the side effect
    // of reversing the entry order.
    hz_map_entry *prev = NULL;
    while (entry != NULL) {
        hz_map_entry *curr = hz_map_entry_alloc(map);
        curr->next = prev;
        curr->hash = entry->hash;
        void *src_key = hz_map_entry_key_offset(map, entry);
        void *src_value = hz_map_entry_value_offset(map, entry);
        hz_map_entry_set_key(map, curr, src_key);
        hz_map_entry_set_value(map, curr, src_value);
        prev = curr;
        entry = entry->next;
    }
    return prev;
}

static void
hz_map_entry_free(hz_map_entry *entry)
{
    hz_free(entry);
}

static size_t
hz_map_next_bucket_count(size_t current_count)
{
    if (current_count > (size_t)(SIZE_MAX / SCALING_FACTOR)) {
        return SIZE_MAX;
    } else if (current_count == 0) {
        return INITIAL_CAPACITY;
    } else {
        return (size_t)(current_count * SCALING_FACTOR);
    }
}

static void
hz_map_resize(hz_map *map)
{
    size_t old_size = map->bucket_count;

    // Allocate and initialize new bucket array
    size_t new_size = hz_map_next_bucket_count(old_size);
    hz_map_entry **new_buckets = hz_malloc(new_size, sizeof(hz_map_entry *));
    for (size_t i = 0; i < new_size; ++i) {
        new_buckets[i] = NULL;
    }

    // Move entries to the new array
    hz_map_entry **old_buckets = map->buckets;
    for (size_t i = 0; i < old_size; ++i) {
        hz_map_entry *e = old_buckets[i];
        while (e != NULL) {
            size_t dest_index = hz_map_get_bucket_index(e->hash, new_size);
            hz_map_entry *old_head = new_buckets[dest_index];
            new_buckets[dest_index] = e;
            e = e->next;
            new_buckets[dest_index]->next = old_head;
        }
    }
    map->bucket_count = new_size;
    map->buckets = new_buckets;

    // Free old bucket array
    hz_free(old_buckets);
}

static bool
hz_map_should_resize(const hz_map *map, size_t index)
{
    if (map->bucket_count == 0) {
        // Always need to resize an empty map
        return true;
    } else if (map->buckets[index] == NULL) {
        // If we don't have a collision, don't resize even
        // if we are over the load factor
        return false;
    } else if (map->bucket_count == SIZE_MAX) {
        // Can't resize a full map (though this case will
        // probably never be hit since we would reach the
        // maximum memory far before this)
        return false;
    } else {
        // Otherwise, resize if we are over the load factor
        return map->size >= (size_t)(map->bucket_count * LOAD_FACTOR);
    }
}

static void
hz_map_add_entry(hz_map *map, size_t hash, const void *key, const void *value)
{
    // First find which bucket this entry belongs to,
    // since we only resize the map if we've reached the
    // load factor AND we get a collision.
    size_t index = 0;
    if (map->bucket_count != 0) {
        index = hz_map_get_bucket_index(hash, map->bucket_count);
    }

    // Resize the map if necessary. If a resize is performed,
    // we need to recalculate the bucket index.
    if (hz_map_should_resize(map, index)) {
        hz_map_resize(map);
        index = hz_map_get_bucket_index(hash, map->bucket_count);
    }

    // Allocate entry and push it at the head of the bucket
    hz_map_entry *new_entry = hz_map_entry_new(map, hash, key, value);
    hz_map_entry *old_head = map->buckets[index];
    new_entry->next = old_head;
    map->buckets[index] = new_entry;
    map->size++;
}

static void
hz_map_free_buckets(hz_map *map)
{
    for (size_t i = 0; i < map->bucket_count; ++i) {
        hz_map_entry *entry = map->buckets[i];
        while (entry != NULL) {
            hz_map_entry *next = entry->next;
            hz_map_entry_free(entry);
            entry = next;
        }
    }
    hz_free(map->buckets);
}

hz_map *
hz_map_new(
    size_t key_size,
    size_t value_size,
    hz_map_hash_func hash_func,
    hz_map_cmp_func cmp_func)
{
    hz_check_null(hash_func);
    hz_check_null(cmp_func);
    hz_map *map = hz_malloc(1, sizeof(hz_map));
    map->key_size = key_size;
    map->value_size = value_size;
    map->hash_func = hash_func;
    map->cmp_func = cmp_func;
    map->size = 0;
    map->bucket_count = 0;
    map->buckets = NULL;
    map->mod_count = 0;
    return map;
}

hz_map *
hz_map_copy(const hz_map *map)
{
    hz_check_null(map);
    hz_map *new_map = hz_malloc(1, sizeof(hz_map));
    new_map->key_size = map->key_size;
    new_map->value_size = map->value_size;
    new_map->hash_func = map->hash_func;
    new_map->cmp_func = map->cmp_func;
    new_map->size = map->size;
    new_map->bucket_count = map->bucket_count;
    new_map->buckets = hz_malloc(map->bucket_count, sizeof(hz_map_entry *));
    for (size_t i = 0; i < map->bucket_count; ++i) {
        new_map->buckets[i] = hz_map_entry_copy(map, map->buckets[i]);
    }
    new_map->mod_count = map->mod_count;
    return new_map;
}

void
hz_map_free(hz_map *map)
{
    if (map != NULL) {
        hz_map_free_buckets(map);
        hz_free(map);
    }
}

size_t
hz_map_size(const hz_map *map)
{
    hz_check_null(map);
    return map->size;
}

void
hz_map_clear(hz_map *map)
{
    hz_check_null(map);
    hz_map_touch(map);
    hz_map_free_buckets(map);
    map->size = 0;
    map->bucket_count = 0;
    map->buckets = NULL;
}

bool
hz_map_get(const hz_map *map, const void *key, void *out_value)
{
    hz_check_null(map);
    hz_check_null(key);
    size_t hash = hz_map_hash_key(map, key);
    hz_map_entry *entry = hz_map_find_entry(map, hash, key);
    if (entry != NULL) {
        if (out_value != NULL) {
            hz_map_entry_get_value(map, entry, out_value);
        }
        return true;
    } else {
        return false;
    }
}

bool
hz_map_put(hz_map *map, const void *key, const void *value, void *out_value)
{
    hz_check_null(map);
    hz_check_null(key);
    hz_check_null(value);
    hz_map_touch(map);
    size_t hash = hz_map_hash_key(map, key);
    hz_map_entry *entry = hz_map_find_entry(map, hash, key);
    if (entry != NULL) {
        // If we already had a matching entry for the given key,
        // just replace the entry's value
        if (out_value != NULL) {
            hz_map_entry_get_value(map, entry, out_value);
        }
        hz_map_entry_set_value(map, entry, value);
        return true;
    } else {
        // No matching entry for the given key, insert a new one
        hz_map_add_entry(map, hash, key, value);
        return false;
    }
}

bool
hz_map_remove(hz_map *map, const void *key, void *out_value)
{
    hz_check_null(map);
    hz_check_null(key);

    // No buckets to check, fail fast
    if (map->size == 0) {
        return false;
    }

    // Scan corresponding bucket for the entry
    size_t hash = hz_map_hash_key(map, key);
    size_t index = hz_map_get_bucket_index(hash, map->bucket_count);
    hz_map_entry **entry = &map->buckets[index];
    while (*entry != NULL) {
        hz_map_entry *curr = *entry;
        if (hz_map_entry_matches(map, curr, hash, key)) {
            if (out_value != NULL) {
                hz_map_entry_get_value(map, curr, out_value);
            }
            *entry = curr->next;
            hz_map_entry_free(curr);
            hz_map_touch(map);
            map->size--;
            return true;
        }
        entry = &curr->next;
    }

    // Didn't find an entry for the given key
    return false;
}

bool
hz_map_equals(const hz_map *a, const hz_map *b, hz_map_cmp_func cmp_func)
{
    if (a == b) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }
    if (a->key_size != b->key_size) {
        hz_abort("Maps have different key types");
    }
    if (a->value_size != b->value_size) {
        hz_abort("Maps have different value types");
    }
    if (a->size != b->size) {
        return false;
    }

    // Since the maps have the same size, they are equal if and only if
    // each key in A also exists in B and maps to the same value.
    for (size_t i = 0; i < a->bucket_count; ++i) {
        hz_map_entry *a_entry = a->buckets[i];
        while (a_entry != NULL) {
            void *key = hz_map_entry_key_offset(a, a_entry);

            // Find corresponding entry in B
            size_t b_hash = hz_map_hash_key(b, key);
            hz_map_entry *b_entry = hz_map_find_entry(b, b_hash, key);
            if (b_entry == NULL) {
                return false;
            }

            // If a custom comparator function was provided, use
            // that to determine value equality. Otherwise, use memcmp().
            void *a_value = hz_map_entry_value_offset(a, a_entry);
            void *b_value = hz_map_entry_value_offset(b, b_entry);
            int cmp;
            if (cmp_func == NULL) {
                cmp = hz_memcmp(a_value, b_value, 1, a->value_size);
            } else {
                cmp = cmp_func(a_value, b_value);
            }
            if (cmp != 0) {
                return false;
            }

            a_entry = a_entry->next;
        }
    }
    return true;
}

hz_map_iterator *
hz_map_iterator_new(const hz_map *map)
{
    hz_check_null(map);
    hz_map_iterator *it = hz_malloc(1, sizeof(hz_map_iterator));
    it->map = map;
    it->mod_count = map->mod_count;
    it->bucket_index = 0;
    it->current_entry = NULL;
    return it;
}

void
hz_map_iterator_free(hz_map_iterator *it)
{
    hz_free(it);
}

bool
hz_map_iterator_next(hz_map_iterator *it, void *key, void *value)
{
    hz_check_null(it);

    // Make sure we haven't modified the map between iterations,
    // since the index and entry may no longer be valid
    if (it->mod_count != it->map->mod_count) {
        hz_abort("Map contents modified during iteration");
    }

    // If we've iterated over everything in the current bucket,
    // move to the next bucket
    while (it->current_entry == NULL) {
        // If no more buckets, we've finished iterating the map
        if (it->bucket_index == it->map->bucket_count) {
            return false;
        }
        it->current_entry = it->map->buckets[it->bucket_index++];
    }

    // Write key and value as necessary
    if (key != NULL) {
        hz_map_entry_get_key(it->map, it->current_entry, key);
    }
    if (value != NULL) {
        hz_map_entry_get_value(it->map, it->current_entry, value);
    }

    // Move to the next entry in the current bucket
    it->current_entry = it->current_entry->next;
    return true;
}
