#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DtAllocators.h"
#include "DtEcs.h"
#include "RegisterHandler.h"

static bool has = true;

static void tag_pool_add(void*, DtEntity, const void*);
static void* tag_pool_get(const void*, DtEntity);
static bool tag_pool_has(const void* pool, DtEntity entity);
static void tag_pool_reset(void* pool, DtEntity entity);
static void tag_pool_copy(void* pool, DtEntity dst, DtEntity src);
static void tag_pool_remove(void*, DtEntity);
static void tag_pool_resize(void*, u16);
static void tag_pool_start(void*);
static void* tag_pool_current(void*);
static bool tag_pool_has_current(void*);
static void tag_pool_next(void*);
static void tag_pool_free(void*);

//TODO: split decl def
static int get_hash(const char* name) {
    int hash = 2147483647;
    while (*name) {
        hash ^= *name++;
        hash *= 314159;
    }
    return hash;
}

DtEcsPool* dt_tag_pool_new(const DtEcsManager* manager, const char* name) {
    DtTagPool* pool = DT_MALLOC(sizeof(DtTagPool));
    if (!pool)
        return NULL;

    const size_t num_buckets = (manager->sparse_size + BUCKET_SIZE - 1) / BUCKET_SIZE;

    *pool = (DtTagPool) {
        .pool =
            (DtEcsPool) {
                .manager = manager,
                .count = 0,
                .name = name,
                .hash = dt_component_get_data_by_name(name)->hash,
                .data = pool,

                .type = TAG_POOL,

                .add = tag_pool_add,
                .get = tag_pool_get,
                .has = tag_pool_has,
                .reset = tag_pool_reset,
                .copy = tag_pool_copy,
                .remove = tag_pool_remove,
                .resize = tag_pool_resize,
                .free = tag_pool_free,
                .iterator =
                    (DtIterator) {
                        .start = tag_pool_start,
                        .current = tag_pool_current,
                        .has_current = tag_pool_has_current,
                        .next = tag_pool_next,
                        .enumerable = pool,
                    },
            },
        .buckets = calloc(num_buckets, sizeof(TagBucket)),
        .size = num_buckets,
        .max_entities = num_buckets * BUCKET_SIZE,
    };

    return &pool->pool;
}

static void tag_pool_add(void* pool, DtEntity entity, const void* data) {
    DtTagPool* tag_pool = pool;

    const size_t bucket_idx = entity / BUCKET_SIZE;
    const size_t bit_offset = entity % BUCKET_SIZE;

    tag_pool->buckets[bucket_idx] |= 1 << bit_offset;
    tag_pool->pool.count++;
}

static void* tag_pool_get(const void* data, DtEntity entity) {
    return tag_pool_has(data, entity) ? &has : NULL;
}

static bool tag_pool_has(const void* pool, const DtEntity entity) {
    const DtTagPool* tag_pool = pool;

    if (entity >= tag_pool->max_entities) {
        return 0;
    }

    size_t bucket_idx = entity / BUCKET_SIZE;
    size_t bit_offset = entity % BUCKET_SIZE;

    return (tag_pool->buckets[bucket_idx] & 1 << bit_offset) != 0;
}

static void tag_pool_reset(void* pool, DtEntity entity) {}

static void tag_pool_copy(void* pool, const DtEntity dst, DtEntity src) {
    if (tag_pool_has(pool, src))
        tag_pool_add(pool, dst, NULL);
    else
        tag_pool_remove(pool, dst);
}

static void tag_pool_remove(void* pool, const DtEntity entity) {
    const DtTagPool* tag_pool = pool;

    if (entity >= tag_pool->max_entities) {
        return;
    }

    const size_t bucket_idx = entity / BUCKET_SIZE;
    const size_t bit_offset = entity % BUCKET_SIZE;

    tag_pool->buckets[bucket_idx] &= ~(1 << bit_offset);
}

static void tag_pool_resize(void* pool, const u16 new_max_entities) {
    DtTagPool* tag_pool = pool;

    const size_t new_capacity = (new_max_entities + BUCKET_SIZE - 1) / BUCKET_SIZE;
    const size_t new_size = new_capacity * sizeof(TagBucket);

    TagBucket* new_buckets = realloc(tag_pool->buckets, new_size);
    if (!new_buckets) {
        printf("[DEBUG] Failed to resize tag pool to %d entities\n", new_max_entities);
        return;
    }

    if (new_capacity > tag_pool->size) {
        const u16 old_size = tag_pool->size * sizeof(TagBucket);
        memset(new_buckets + tag_pool->size, 0, new_size - old_size);
    }

    tag_pool->buckets = new_buckets;
    tag_pool->size = new_capacity;
    tag_pool->max_entities = new_capacity * BUCKET_SIZE;
}

static void tag_pool_start(void* data) {
    DtTagPool* tag_pool = data;

    tag_pool->iterator_bucket = tag_pool->buckets[0];
    tag_pool->iterator_entity = 0;
    tag_pool->iterator_ptr = 0;
}

static void* tag_pool_current(void* data) {
    DtTagPool* tag_pool = data;
    tag_pool->iterator_entity =
        __builtin_ctz(tag_pool->iterator_bucket) + sizeof(TagBucket) * tag_pool->iterator_ptr;


    return &tag_pool->iterator_entity;
}

static bool tag_pool_has_current(void* data) {
    DtTagPool* tag_pool = data;

    return tag_pool->iterator_ptr < tag_pool->size;
}

static void tag_pool_next(void* data) {
    DtTagPool* tag_pool = data;

    tag_pool->iterator_bucket &= tag_pool->iterator_bucket - 1;

    while (tag_pool->iterator_bucket == 0) {
        tag_pool->iterator_ptr++;
        if (tag_pool->iterator_ptr == tag_pool->size)
            return;
        tag_pool->iterator_bucket = tag_pool->buckets[tag_pool->iterator_ptr];
    }
}

void tag_pool_free(void* pool) {
    DT_FREE(((DtTagPool*) pool)->buckets);
    DT_FREE(pool);
}
