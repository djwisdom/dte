#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "intmap.h"
#include "arith.h"
#include "bit.h"
#include "debug.h"
#include "errorcode.h"

const char tombstone[16] = "TOMBSTONE";

enum {
    MIN_CAPACITY = 8,
};

static size_t hash_key(uint32_t k)
{
    k = ((k >> 16) ^ k) * 0x45d9f3b;
    k = ((k >> 16) ^ k) * 0x45d9f3b;
    k = (k >> 16) ^ k;
    return k;
}

WARN_UNUSED_RESULT
static SystemErrno intmap_resize(IntMap *map, size_t capacity)
{
    BUG_ON(capacity < MIN_CAPACITY);
    BUG_ON(capacity <= map->count);
    BUG_ON(!IS_POWER_OF_2(capacity));

    const size_t entrysize = sizeof(map->entries[0]);
    if (unlikely(calloc_args_have_ub_overflow(capacity, entrysize))) {
        return EOVERFLOW;
    }

    IntMapEntry *newtab = calloc(capacity, entrysize); // NOLINT(*-unsafe-functions)
    if (unlikely(!newtab)) {
        return ENOMEM;
    }

    IntMapEntry *oldtab = map->entries;
    size_t oldlen = map->mask + 1;
    map->entries = newtab;
    map->mask = capacity - 1;
    map->tombstones = 0;

    if (!oldtab) {
        return 0;
    }

    // Copy the entries to the new table
    for (const IntMapEntry *e = oldtab, *end = e + oldlen; e < end; e++) {
        if (!e->value || e->value == tombstone) {
            continue;
        }
        IntMapEntry *newe;
        for (size_t i = hash_key(e->key), j = 1; ; i += j++) {
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
static SystemErrno intmap_do_init(IntMap *map, size_t capacity)
{
    // Accommodate the 75% load factor in the table size, to allow
    // filling to the requested size without needing to resize()
    capacity += capacity / 3;

    if (unlikely(capacity < MIN_CAPACITY)) {
        capacity = MIN_CAPACITY;
    }

    capacity = next_pow2(capacity);
    if (unlikely(capacity == 0)) {
        return EOVERFLOW;
    }

    *map = (IntMap)INTMAP_INIT;
    return intmap_resize(map, capacity);
}

void intmap_init(IntMap *map, size_t capacity)
{
    SystemErrno err = intmap_do_init(map, capacity);
    if (unlikely(err)) {
        fatal_error(__func__, err);
    }
}

IntMapEntry *intmap_find(const IntMap *map, uint32_t key)
{
    if (unlikely(!map->entries)) {
        return NULL;
    }

    size_t hash = hash_key(key);
    IntMapEntry *e;
    for (size_t i = hash, j = 1; ; i += j++) {
        e = map->entries + (i & map->mask);
        if (!e->value) {
            return NULL;
        }
        if (e->value == tombstone) {
            continue;
        }
        if (e->key == key) {
            return e;
        }
    }

    BUG("unexpected loop break");
}

void *intmap_remove(IntMap *map, uint32_t key)
{
    IntMapEntry *e = intmap_find(map, key);
    if (!e) {
        return NULL;
    }

    void *value = e->value;
    e->key = 0;
    e->value = (char*)tombstone;
    map->count--;
    map->tombstones++;
    return value;
}

WARN_UNUSED_RESULT
static SystemErrno intmap_do_insert(IntMap *map, uint32_t key, void *value, void **old_value)
{
    SystemErrno err = 0;
    if (unlikely(!map->entries)) {
        err = intmap_do_init(map, 0);
        if (unlikely(err)) {
            return err;
        }
    }

    size_t hash = hash_key(key);
    bool replacing_tombstone_or_existing_value = false;
    IntMapEntry *e;
    for (size_t i = hash, j = 1; ; i += j++) {
        e = map->entries + (i & map->mask);
        if (!e->value) {
            break;
        }
        if (e->value == tombstone) {
            replacing_tombstone_or_existing_value = true;
            BUG_ON(map->tombstones == 0);
            map->tombstones--;
        }
        if (unlikely(e->key == key)) {
            replacing_tombstone_or_existing_value = true;
            BUG_ON(!e->value);
            *old_value = e->value;
            key = e->key;
            map->count--;
            break;
        }
    }

    const size_t max_load = map->mask - (map->mask / 4);
    e->key = key;
    e->value = value;
    map->count++;

    if (unlikely(map->count + map->tombstones > max_load)) {
        BUG_ON(replacing_tombstone_or_existing_value);
        size_t new_size = map->mask + 1;
        if (map->count > map->tombstones || new_size <= 256) {
            // Only increase the size of the table when the number of
            // real entries is higher than the number of tombstones
            new_size <<= 1;
            if (unlikely(new_size == 0)) {
                err = EOVERFLOW;
                goto error;
            }
        }
        err = intmap_resize(map, new_size);
        if (unlikely(err)) {
            goto error;
        }
    }

    return 0;

error:
    map->count--;
    e->key = 0;
    e->value = NULL;
    return err;
}

void *intmap_insert_or_replace(IntMap *map, uint32_t key, void *value)
{
    void *replaced_value = NULL;
    SystemErrno err = intmap_do_insert(map, key, value, &replaced_value);
    if (unlikely(err)) {
        fatal_error(__func__, err);
    }
    return replaced_value;
}

// Remove all entries without freeing the table
static void intmap_clear(IntMap *map, FreeFunction free_value)
{
    if (unlikely(!map->entries)) {
        return;
    }

    if (free_value) {
        size_t count = 0;
        for (IntMapIter it = intmap_iter(map); intmap_next(&it); count++) {
            do_free_value(free_value, it.entry->value);
        }
        BUG_ON(count != map->count);
    }

    size_t len = map->mask + 1;
    map->count = 0;
    memset(map->entries, 0, len * sizeof(*map->entries));
}

void intmap_free(IntMap *map, FreeFunction free_value)
{
    intmap_clear(map, free_value);
    free(map->entries);
    *map = (IntMap)INTMAP_INIT;
}
