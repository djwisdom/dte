#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"
#include "arith.h"
#include "bit.h"
#include "errorcode.h"
#include "hash.h"
#include "xstring.h"

enum {
    MIN_CAPACITY = 8,
    TOMBSTONE = 0xdead,
};

WARN_UNUSED_RESULT
static SystemErrno hashmap_resize(HashMap *map, size_t capacity)
{
    BUG_ON(capacity < MIN_CAPACITY);
    BUG_ON(capacity <= map->count);
    BUG_ON(!IS_POWER_OF_2(capacity));

    const size_t entrysize = sizeof(map->entries[0]);
    if (unlikely(calloc_args_have_ub_overflow(capacity, entrysize))) {
        return EOVERFLOW;
    }

    HashMapEntry *newtab = calloc(capacity, entrysize); // NOLINT(*-unsafe-functions)
    if (unlikely(!newtab)) {
        return ENOMEM;
    }

    HashMapEntry *oldtab = map->entries;
    size_t oldlen = map->mask + 1;
    map->entries = newtab;
    map->mask = capacity - 1;
    map->tombstones = 0;

    if (!oldtab) {
        return 0;
    }

    // Copy the entries to the new table
    for (const HashMapEntry *e = oldtab, *end = e + oldlen; e < end; e++) {
        if (!e->key) {
            continue;
        }
        HashMapEntry *newe;
        for (size_t i = e->hash, j = 1; ; i += j++) {
            newe = newtab + (i & map->mask);
            if (!newe->key) {
                break;
            }
        }
        *newe = *e;
    }

    free(oldtab);
    return 0;
}

WARN_UNUSED_RESULT
static SystemErrno hashmap_do_init(HashMap *map, size_t capacity)
{
    // Accommodate the 75% load factor in the table size, to allow
    // filling to the requested size without needing to resize()
    capacity += capacity / 3;

    if (unlikely(capacity < MIN_CAPACITY)) {
        capacity = MIN_CAPACITY;
    }

    // Round up the size to the next power of 2, to allow using simple
    // bitwise ops (instead of modulo) to wrap the hash value and also
    // to allow quadratic probing with triangular numbers (`i += j++`
    // in the `for` loops below)
    capacity = next_pow2(capacity);
    if (unlikely(capacity == 0)) {
        return EOVERFLOW;
    }

    return hashmap_resize(map, capacity);
}

void hashmap_init(HashMap *map, size_t capacity, HashMapFlags flags)
{
    *map = (HashMap){.flags = flags};
    SystemErrno err = hashmap_do_init(map, capacity);
    if (unlikely(err)) {
        fatal_error(__func__, err);
    }
}

HashMapEntry *hashmap_find(const HashMap *map, const char *key)
{
    if (unlikely(!map->entries)) {
        return NULL;
    }

    size_t hash = fnv_1a_hash(key, strlen(key));
    HashMapEntry *e;
    for (size_t i = hash, j = 1; ; i += j++) {
        e = map->entries + (i & map->mask);
        if (!e->key) {
            if (e->hash == TOMBSTONE) {
                continue;
            }
            return NULL;
        }
        if (e->hash == hash && streq(e->key, key)) {
            return e;
        }
    }

    BUG("unexpected loop break");
}

static void hashmap_free_key(const HashMap *map, char *key)
{
    if (!(map->flags & HMAP_BORROWED_KEYS)) {
        free(key);
    }
}

void *hashmap_remove(HashMap *map, const char *key)
{
    HashMapEntry *e = hashmap_find(map, key);
    if (!e) {
        return NULL;
    }

    hashmap_free_key(map, e->key);
    e->key = NULL;
    e->hash = TOMBSTONE;
    map->count--;
    map->tombstones++;
    return e->value;
}

WARN_UNUSED_RESULT
static SystemErrno hashmap_do_insert(HashMap *map, char *key, void *value, void **old_value)
{
    SystemErrno err = 0;
    if (unlikely(!map->entries)) {
        err = hashmap_do_init(map, 0);
        if (unlikely(err)) {
            return err;
        }
    }

    size_t hash = fnv_1a_hash(key, strlen(key));
    bool replacing_tombstone_or_existing_value = false;
    HashMapEntry *e;
    for (size_t i = hash, j = 1; ; i += j++) {
        e = map->entries + (i & map->mask);
        if (!e->key) {
            if (e->hash == TOMBSTONE) {
                replacing_tombstone_or_existing_value = true;
                BUG_ON(map->tombstones == 0);
                map->tombstones--;
            }
            break;
        }
        if (unlikely(e->hash == hash && streq(e->key, key))) {
            replacing_tombstone_or_existing_value = true;
            // When a caller passes NULL as the "old_value" return param,
            // it implies there can be no existing entry with the same key
            // as the one to be inserted.
            BUG_ON(!old_value);
            BUG_ON(!e->value);
            *old_value = e->value;
            hashmap_free_key(map, key);
            key = e->key;
            map->count--;
            break;
        }
    }

    const size_t max_load = map->mask - (map->mask / 4);
    e->key = key;
    e->value = value;
    e->hash = hash;
    map->count++;

    if (unlikely(map->count + map->tombstones > max_load)) {
        BUG_ON(replacing_tombstone_or_existing_value);
        size_t new_size = map->mask + 1;
        if (map->count > map->tombstones || new_size <= 256) {
            // Only increase the size of the table when the number of
            // real entries is higher than the number of tombstones.
            // If the number of real entries is lower, the table is
            // most likely being filled with tombstones from repeated
            // insert/remove churn; so we just rehash at the same size
            // to clean out the tombstones.
            new_size <<= 1;
            if (unlikely(new_size == 0)) {
                err = EOVERFLOW;
                goto error;
            }
        }
        err = hashmap_resize(map, new_size);
        if (unlikely(err)) {
            goto error;
        }
    }

    return 0;

error:
    map->count--;
    e->key = NULL;
    e->hash = 0;
    return err;
}

void *hashmap_insert(HashMap *map, char *key, void *value)
{
    SystemErrno err = hashmap_do_insert(map, key, value, NULL);
    if (unlikely(err)) {
        fatal_error(__func__, err);
    }
    return value;
}

void *hashmap_insert_or_replace(HashMap *map, char *key, void *value)
{
    void *replaced_value = NULL;
    SystemErrno err = hashmap_do_insert(map, key, value, &replaced_value);
    if (unlikely(err)) {
        fatal_error(__func__, err);
    }
    return replaced_value;
}

// Remove all entries without freeing the table
void hashmap_clear(HashMap *map, FreeFunction free_value)
{
    if (unlikely(!map->entries)) {
        return;
    }

    size_t count = 0;
    for (HashMapIter it = hashmap_iter(map); hashmap_next(&it); count++) {
        hashmap_free_key(map, it.entry->key);
        if (free_value) {
            do_free_value(free_value, it.entry->value);
        }
    }

    BUG_ON(count != map->count);
    size_t len = map->mask + 1;
    map->count = 0;
    memset(map->entries, 0, len * sizeof(*map->entries));
}

void hashmap_free(HashMap *map, FreeFunction free_value)
{
    hashmap_clear(map, free_value);
    free(map->entries);
    *map = (HashMap){.flags = map->flags};
}
