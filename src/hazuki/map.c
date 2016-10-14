#include "hazuki/map.h"
#include "hazuki/utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 8
#define SCALING_FACTOR 2
#define LOAD_FACTOR 0.75

typedef struct hz_map_entry
{
    size_t hash;
    void *key;
    void *value;
    struct hz_map_entry *next;
} hz_map_entry;

struct hz_map
{
    size_t size;
    size_t bucket_count;
    hz_map_entry **buckets;
    hz_map_hash_func hash_func;
    hz_map_cmp_func cmp_func;
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
hz_map_check_null(const hz_map *map)
{
    if (map == NULL) {
        hz_abort("Map is null");
    }
}

static void
hz_map_iterator_check_null(const hz_map_iterator *it)
{
    if (it == NULL) {
        hz_abort("Map iterator is null");
    }
}

static void
hz_map_touch(hz_map *map)
{
    map->mod_count++;
}

static size_t
hz_map_hash_key(const hz_map *map, const void *key)
{
    return map->hash_func(key);
}

static size_t
hz_map_get_bucket_index(size_t hash, size_t bucket_count)
{
    return hash % bucket_count;
}

static bool
hz_map_entry_matches(const hz_map *map, hz_map_entry *entry,
                     size_t hash, const void *key)
{
    return (entry->hash == hash) &&
           (entry->key == key || map->cmp_func(entry->key, key) == 0);
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
hz_map_entry_new(size_t hash, void *key, void *value)
{
    hz_map_entry *entry = hz_malloc(1, sizeof(hz_map_entry));
    entry->hash = hash;
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static hz_map_entry *
hz_map_entry_copy(hz_map_entry *entry)
{
    hz_map_entry *prev = NULL;
    while (entry != NULL) {
        hz_map_entry *curr = hz_malloc(1, sizeof(hz_map_entry));
        curr->hash = entry->hash;
        curr->key = entry->key;
        curr->value = entry->value;
        curr->next = prev;
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
        return current_count * SCALING_FACTOR;
    }
}

static void
hz_map_resize(hz_map *map)
{
    size_t old_size = map->bucket_count;
    size_t new_size = hz_map_next_bucket_count(old_size);
    hz_map_entry **old_buckets = map->buckets;
    hz_map_entry **new_buckets = hz_malloc(new_size, sizeof(hz_map_entry *));
    for (size_t i = 0; i < new_size; ++i) {
        new_buckets[i] = NULL;
    }
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
    hz_free(old_buckets);
}

static bool
hz_map_should_resize(const hz_map *map, size_t index)
{
    if (map->bucket_count == 0) {
        return true;
    } else if (map->buckets[index] == NULL) {
        return false;
    } else if (map->bucket_count == SIZE_MAX) {
        return false;
    } else {
        return map->size >= (size_t)(map->bucket_count * LOAD_FACTOR);
    }
}

static void
hz_map_add_entry(hz_map *map, size_t hash, void *key, void *value)
{
    size_t index = 0;
    if (map->bucket_count != 0) {
        index = hz_map_get_bucket_index(hash, map->bucket_count);
    }
    if (hz_map_should_resize(map, index)) {
        hz_map_resize(map);
        index = hz_map_get_bucket_index(hash, map->bucket_count);
    }
    hz_map_entry *new_entry = hz_map_entry_new(hash, key, value);
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
hz_map_new(hz_map_hash_func hash, hz_map_cmp_func cmp)
{
    if (hash == NULL) {
        hz_abort("Key hash function is null");
    } else if (cmp == NULL) {
        hz_abort("Key comparator function is null");
    }
    hz_map *map = hz_malloc(1, sizeof(hz_map));
    map->size = 0;
    map->hash_func = hash;
    map->cmp_func = cmp;
    map->bucket_count = 0;
    map->buckets = NULL;
    map->mod_count = 0;
    return map;
}

hz_map *
hz_map_copy(const hz_map *map)
{
    hz_map_check_null(map);
    hz_map *new_map = hz_malloc(1, sizeof(hz_map));
    new_map->size = map->size;
    new_map->hash_func = map->hash_func;
    new_map->cmp_func = map->cmp_func;
    new_map->bucket_count = map->bucket_count;
    new_map->buckets = hz_malloc(map->bucket_count, sizeof(hz_map_entry *));
    for (size_t i = 0; i < map->bucket_count; ++i) {
        new_map->buckets[i] = hz_map_entry_copy(map->buckets[i]);
    }
    return new_map;
}

void
hz_map_free(hz_map *map)
{
    hz_map_check_null(map);
    hz_map_free_buckets(map);
    hz_free(map);
}

size_t
hz_map_size(const hz_map *map)
{
    hz_map_check_null(map);
    return map->size;
}

void
hz_map_clear(hz_map *map)
{
    hz_map_check_null(map);
    hz_map_touch(map);
    hz_map_free_buckets(map);
    map->size = 0;
    map->bucket_count = 0;
    map->buckets = NULL;
}

bool
hz_map_contains(const hz_map *map, const void *key)
{
    hz_map_check_null(map);
    size_t hash = hz_map_hash_key(map, key);
    return hz_map_find_entry(map, hash, key) != NULL;
}

void *
hz_map_get(const hz_map *map, const void *key)
{
    hz_map_check_null(map);
    size_t hash = hz_map_hash_key(map, key);
    hz_map_entry *entry = hz_map_find_entry(map, hash, key);
    if (entry != NULL) {
        return entry->value;
    } else {
        return NULL;
    }
}

void *
hz_map_put(hz_map *map, void *key, void *value)
{
    hz_map_check_null(map);
    hz_map_touch(map);
    size_t hash = hz_map_hash_key(map, key);
    hz_map_entry *entry = hz_map_find_entry(map, hash, key);
    if (entry != NULL) {
        void *old_value = entry->value;
        entry->value = value;
        return old_value;
    } else {
        hz_map_add_entry(map, hash, key, value);
        return NULL;
    }
}

void *
hz_map_remove(hz_map *map, const void *key)
{
    hz_map_check_null(map);
    if (map->size == 0) {
        return NULL;
    }
    size_t hash = hz_map_hash_key(map, key);
    size_t index = hz_map_get_bucket_index(hash, map->bucket_count);
    hz_map_entry *prev = NULL;
    hz_map_entry *curr = map->buckets[index];
    while (curr != NULL) {
        if (hz_map_entry_matches(map, curr, hash, key)) {
            if (prev == NULL) {
                map->buckets[index] = curr->next;
            } else {
                prev->next = curr->next;
            }
            void *value = curr->value;
            hz_map_entry_free(curr);
            hz_map_touch(map);
            map->size--;
            return value;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

hz_map_iterator *
hz_map_iterator_new(const hz_map *map)
{
    hz_map_check_null(map);
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
    hz_map_iterator_check_null(it);
    hz_free(it);
}

bool
hz_map_iterator_next(hz_map_iterator *it, void **key, void **value)
{
    hz_map_iterator_check_null(it);
    if (it->mod_count != it->map->mod_count) {
        hz_abort("Map contents modified during iteration");
    }
    while (it->current_entry == NULL) {
        if (it->bucket_index == it->map->bucket_count) {
            return false;
        }
        it->current_entry = it->map->buckets[it->bucket_index++];
    }
    if (key != NULL) {
        *key = it->current_entry->key;
    }
    if (value != NULL) {
        *value = it->current_entry->value;
    }
    it->current_entry = it->current_entry->next;
    return true;
}
