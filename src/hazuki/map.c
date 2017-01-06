#include "hazuki/map.h"
#include "hazuki/utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define INITIAL_CAPACITY 8
#define SCALING_FACTOR 2
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
    return map->hash_func(key);
}

static size_t
hz_map_get_bucket_index(size_t hash, size_t bucket_count)
{
    return hash % bucket_count;
}

static hz_map_entry *
hz_map_entry_alloc(const hz_map *map)
{
    size_t size = sizeof(hz_map_entry) + map->key_size + map->value_size;
    hz_map_entry *entry = hz_malloc(1, size);
    return entry;
}

static void *
hz_map_entry_key_offset(const hz_map *map, const hz_map_entry *entry)
{
    (void)map;
    return (void *)(entry->buffer);
}

static void *
hz_map_entry_value_offset(const hz_map *map, const hz_map_entry *entry)
{
    return (void *)(entry->buffer + map->key_size);
}

static void
hz_map_entry_get_key(const hz_map *map, hz_map_entry *entry,
                     void *out_key)
{
    void *key = hz_map_entry_key_offset(map, entry);
    hz_memcpy(out_key, key, 1, map->key_size);
}

static void
hz_map_entry_get_value(const hz_map *map, hz_map_entry *entry,
                       void *out_value)
{
    void *value = hz_map_entry_value_offset(map, entry);
    hz_memcpy(out_value, value, 1, map->value_size);
}

static void
hz_map_entry_set_key(const hz_map *map, hz_map_entry *entry,
                     const void *new_key)
{
    void *key = hz_map_entry_key_offset(map, entry);
    hz_memcpy(key, new_key, 1, map->key_size);
}

static void
hz_map_entry_set_value(const hz_map *map, hz_map_entry *entry,
                       const void *new_value)
{
    void *value = hz_map_entry_value_offset(map, entry);
    hz_memcpy(value, new_value, 1, map->value_size);
}

static bool
hz_map_entry_matches(const hz_map *map, const hz_map_entry *entry,
                     size_t hash, const void *key)
{
    if (entry->hash != hash) {
        return false;
    }
    if (map->cmp_func(hz_map_entry_key_offset(map, entry), key) != 0) {
        return false;
    }
    return true;
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
hz_map_entry_new(const hz_map *map, size_t hash,
                 const void *key, const void *value)
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
hz_map_add_entry(hz_map *map, size_t hash, const void *key, const void *value)
{
    size_t index = 0;
    if (map->bucket_count != 0) {
        index = hz_map_get_bucket_index(hash, map->bucket_count);
    }
    if (hz_map_should_resize(map, index)) {
        hz_map_resize(map);
        index = hz_map_get_bucket_index(hash, map->bucket_count);
    }
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
hz_map_new(size_t key_size, size_t value_size,
           hz_map_hash_func hash_func, hz_map_cmp_func cmp_func)
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
        if (out_value != NULL) {
            hz_map_entry_get_value(map, entry, out_value);
        }
        hz_map_entry_set_value(map, entry, value);
        return true;
    } else {
        hz_map_add_entry(map, hash, key, value);
        return false;
    }
}

bool
hz_map_remove(hz_map *map, const void *key, void *out_value)
{
    hz_check_null(map);
    hz_check_null(key);
    if (map->size == 0) {
        return false;
    }
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
    return false;
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
        hz_map_entry_get_key(it->map, it->current_entry, key);
    }
    if (value != NULL) {
        hz_map_entry_get_value(it->map, it->current_entry, value);
    }
    it->current_entry = it->current_entry->next;
    return true;
}
