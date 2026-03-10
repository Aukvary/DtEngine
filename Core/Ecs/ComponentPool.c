#include <stdio.h>
#include <stdlib.h>

#include "DtEcs.h"
#include "RegisterHandler.h"

static void component_pool_add(void* pool, DtEntity entity, const void* data);
static void* component_pool_get_item(const void* pool, DtEntity entity);
static bool component_pool_has(const void* pool, DtEntity entity);
static void component_pool_reset(void* pool, DtEntity entity);
static void component_pool_copy(void* pool, DtEntity dst, DtEntity src);
static void component_pool_remove(void* pool, DtEntity entity);
static void component_pool_resize(void* pool, u16 new_size);
static void component_pool_free(void* pool);

//TODO: split decl def
static int get_hash(const char* name) {
    int hash = 2147483647;
    while (*name) {
        hash ^= *name++;
        hash *= 314159;
    }
    return hash;
}

DtEcsPool* dt_component_pool_new(const DtEcsManager* manager, const char* name,
                                 const u16 item_size, const DtResetItemHandler reset_handler,
                                 const DtCopyItemHandler copy_handler) {
    DtComponentPool* pool = malloc(sizeof(DtComponentPool));
    const DtComponentData* component_data = dt_component_get_data_by_name(name);

    *pool = (DtComponentPool) {
        .pool =
            (DtEcsPool) {
                .manager = manager,
                .name = name,
                .hash = component_data->hash,
                .count = 0,

                .data = pool,

                .type = COMPONENT_POOL,

                .add = component_pool_add,
                .get = component_pool_get_item,
                .has = component_pool_has,
                .reset = component_pool_reset,
                .copy = component_pool_copy,
                .remove = component_pool_remove,
                .resize = component_pool_resize,
                .free = component_pool_free,
            },

        .entities = dt_entity_container_new(item_size, manager->cfg_dense_size,
                                            manager->sparse_size, manager->recycled_size,
                                            reset_handler, copy_handler),
    };

    pool->pool.iterator = pool->entities.entities_iterator;
    pool->pool.iterator.enumerable = &pool->entities;

    return &pool->pool;
}

static void component_pool_add(void* pool, DtEntity entity, const void* data) {
    DtComponentPool* component_pool = pool;
    dt_entity_container_add(&component_pool->entities, entity, data);
}

static void* component_pool_get_item(const void* pool, DtEntity entity) {
    const DtComponentPool* component_pool = pool;
    return dt_entity_container_get(&component_pool->entities, entity);
}

static bool component_pool_has(const void* pool, const DtEntity entity) {
    const DtComponentPool* component_pool = pool;
    return dt_entity_container_has(&component_pool->entities, entity);
}

static void component_pool_reset(void* pool, const DtEntity entity) {
    DtComponentPool* component_pool = pool;

    dt_entity_container_reset(&component_pool->entities, entity);
}

static void component_pool_copy(void* pool, const DtEntity dst, const DtEntity src) {
    DtComponentPool* component_pool = pool;
    dt_entity_container_copy(&component_pool->entities, dst, src);
}

static void component_pool_remove(void* pool, const DtEntity entity) {
    DtComponentPool* component_pool = pool;
    dt_entity_container_remove(&component_pool->entities, entity);
}

static void component_pool_resize(void* pool, const u16 new_size) {
    DtComponentPool* component_pool = pool;
    dt_entity_container_resize(&component_pool->entities, new_size);
    component_pool->pool.iterator = component_pool->entities.entities_iterator;
}

static void component_pool_free(void* pool) {
    dt_entity_container_free(&((DtComponentPool*) pool)->entities);
    free(pool);
}
