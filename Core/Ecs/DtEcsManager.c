#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Collections/Collections.h"
#include "DtAllocators.h"
#include "DtEcs.h"
#include "RegisterHandler.h"

/**
 * @brief compair two pools by id
 */
static int cmp_pools(const void* id1, const void* id2);

/**
 * @brief return new filter
 *
 * @param manager manager that contains filters
 * @param mask mask from will be generated filter
 */
static DtEcsFilter* filter_new(DtEcsManager* manager, DtEcsMask mask);

/**
 * @brief add entity to filter
 */
static void filter_add_entity(DtEcsFilter* filter, DtEntity entity);

/**
 * @brief remove entity from filter
 */
static void filter_remove_entity(DtEcsFilter* filter, DtEntity entity);

/**
 * @brief resize filter when entities out of range
 */
static void filter_resize(DtEcsFilter* filter, size_t new_size);

/**
 * @brief free filter memory
 */
static void filter_free(DtEcsFilter*);

/**
 * @brief resize filters container if filter count out of range
 */
static void ecs_manager_resize_filters(DtEcsManager* manager);

/**
 * @brief resize pool container if pool count out of range
 */
static void ecs_manager_resize_pools(DtEcsManager* manager);
/**
 * @brief return filter from manager pool or create new
 */
static DtEcsFilter* get_filter(DtEcsManager* manager, DtEcsMask mask);

/**
 * @brief remove HierarchyDirty component from entities
 */
static void remove_hierarchy_dirty_tag(const DtEcsManager* manager);

/**
 * @brief return 1 if entity has all components pool and hasn't all exclude components
 * else return 0
 */
static int is_mask_compatible(const DtEcsManager* manager, DtEcsMask mask, DtEntity entity);

/**
 * @brief return 1 if entity hasn't all components pool and has all exclude components
 * else return 0
 */
static int is_mask_compatible_without(const DtEcsManager*, DtEcsMask, DtEntity, int);

DtEcsMask dt_mask_new(DtEcsManager* manager, const u16 inc_size, const u16 exc_size) {
    return (DtEcsMask) {
        .manager = manager,

        .include_pools = calloc(inc_size, sizeof(int)),
        .include_size = inc_size,
        .include_count = 0,

        .exclude_pools = calloc(inc_size, sizeof(int)),
        .exclude_size = exc_size,
        .exclude_count = 0,
    };
}

void dt_mask_inc(DtEcsMask* mask, const u16 ecs_manager_component_id) {
    if (mask->include_count == mask->include_size) {
        mask->include_size = mask->include_size ? mask->include_size * 2 : 4;
        void* tmp = DT_REALLOC(mask->include_pools, mask->include_size * sizeof(int));

        if (!tmp) {
            printf("mask realloc failed\n");
            exit(1);
        }

        mask->include_pools = tmp;
    }

    mask->include_pools[mask->include_count++] = ecs_manager_component_id;
}

void dt_mask_exc(DtEcsMask* mask, const u16 ecs_manager_component_id) {
    if (mask->exclude_count == mask->exclude_size) {
        mask->exclude_size = mask->exclude_size ? mask->exclude_size * 2 : 4;
        void* tmp = DT_REALLOC(mask->exclude_pools, mask->exclude_size * sizeof(int));

        if (!tmp) {
            printf("mask realloc failed\n");
            exit(1);
        }

        mask->exclude_pools = tmp;
    }

    mask->exclude_pools[mask->exclude_count++] = ecs_manager_component_id;
}

static int cmp_pools(const void* id1, const void* id2) {
    return *(const int*) id1 - *(const int*) id2;
}

DtEcsFilter* dt_mask_end(DtEcsMask mask) {
    qsort(mask.include_pools, mask.include_count, sizeof(u16), cmp_pools);
    qsort(mask.exclude_pools, mask.exclude_count, sizeof(u16), cmp_pools);

    mask.hash = 314519;
    for (int i = 0; i < mask.include_count; i++) {
        mask.hash += mask.include_pools[i];
        mask.hash ^= mask.manager->pools[mask.include_pools[i]]->hash;
    }

    for (int i = 0; i < mask.exclude_count; i++) {
        mask.hash -= mask.exclude_pools[i];
        mask.hash ^= mask.manager->pools[mask.exclude_pools[i]]->hash;
    }

    return get_filter(mask.manager, mask);
}

static DtEcsFilter* filter_new(DtEcsManager* manager, const DtEcsMask mask) {
    DtEcsFilter* new_filter = malloc(sizeof(DtEcsFilter));

    *new_filter = (DtEcsFilter) {
        .manager = manager,
        .entities =
            dt_entity_container_new(sizeof(DtEntity), manager->cfg_dense_size, manager->sparse_size,
                                    manager->cfg_recycle_size, NULL, NULL),
        .mask = mask,
    };

    new_filter->entities.entities_iterator.enumerable = &new_filter->entities;
    new_filter->entities.items_iterator.enumerable = &new_filter->entities;

    return new_filter;
}

static void filter_add_entity(DtEcsFilter* filter, DtEntity entity) {
    dt_entity_container_add(&filter->entities, entity, &entity);
}

static void filter_remove_entity(DtEcsFilter* filter, const DtEntity entity) {
    dt_entity_container_remove(&filter->entities, entity);
}

static void filter_resize(DtEcsFilter* filter, const size_t new_size) {
    dt_entity_container_resize(&filter->entities, new_size);
    filter->entities.entities_iterator.enumerable = &filter->entities;
    filter->entities.items_iterator.enumerable = &filter->entities;
}

static void filter_free(DtEcsFilter* filter) {
    dt_entity_container_free(&filter->entities);
    free(filter);
}

DtEcsManager* dt_ecs_manager_new(DtEcsManagerConfig cfg) {
    DtEcsManager* manager = DT_MALLOC(sizeof(DtEcsManager));

    if (!manager) {
        //TODO: add handle
        exit(1);
    }

    if (!cfg.sparse_size) cfg.sparse_size = 50;
    if (!cfg.dense_size) cfg.dense_size = 50;
    if (!cfg.recycle_size) cfg.recycle_size = 10;
    if (!cfg.children_size) cfg.children_size = 50;
    if (!cfg.pools_size) cfg.pools_size = 50;
    if (!cfg.include_mask_count) cfg.include_mask_count = 10;
    if (!cfg.exclude_mask_count) cfg.exclude_mask_count = 10;
    if (!cfg.filters_size) cfg.filters_size = 50;

    *manager = (DtEcsManager) {
        .sparse_entities = DT_CALLOC(cfg.sparse_size, sizeof(DtEntityInfo)),
        .sparse_size = cfg.sparse_size,
        .entities_ptr = 0,

        .cfg_dense_size = cfg.dense_size,
        .cfg_recycle_size = cfg.recycle_size,
        .children_size = cfg.children_size,
        .component_count = cfg.components_count,

        .recycled_entities = DT_CALLOC(cfg.recycle_size, sizeof(DtEntity)),
        .recycled_size = cfg.recycle_size,
        .recycled_ptr = 0,

        .pools = DT_VEC_NEW(DtEcsPool*, cfg.pools_size),
        .pools_table = DT_CALLOC(cfg.pools_size, sizeof(DtEcsPool*)),
        .pools_table_size = cfg.pools_size,

        .include_mask_count = cfg.include_mask_count,
        .exclude_mask_count = cfg.exclude_mask_count,

        .filters = DT_CALLOC(cfg.filters_size, sizeof(DtEcsFilter*)),
        .filters_size = cfg.filters_size,
        .filters_count = 0,

        .filter_by_include = DT_CALLOC(cfg.pools_size, sizeof(DT_VEC(DtEcsFilter*))),
        .filter_by_exclude = DT_CALLOC(cfg.pools_size, sizeof(DT_VEC(DtEcsFilter*))),
    };

    manager->hierarchy_dirty_pool = DT_ECS_MANAGER_GET_POOL(manager, HierarchyDirty);

    return manager;
}

DtEntity dt_ecs_manager_new_entity(DtEcsManager* manager) {
    DtEntity entity;

    if (manager->recycled_ptr > 0) {
        entity = manager->recycled_entities[--manager->recycled_ptr];
        dt_entity_info_reuse(&manager->sparse_entities[entity]);
        printf("[DEBUG]\t recycled entity \"%d\" was created\n", entity);
    } else {
        entity = manager->entities_ptr++;
        if (entity == manager->sparse_size) {
            printf("[DEBUG]\t need to resize sparse array\n");
            manager->sparse_size = manager->sparse_size ? 2 * manager->sparse_size : 8;
            void* tmp =
                DT_REALLOC(manager->sparse_entities, manager->sparse_size * sizeof(DtEntityInfo));

            if (!tmp) {
                printf("entity add realloc failed\n");
                exit(1);
            }

            manager->sparse_entities = tmp;

            for (int i = 0; i < manager->pools_table_size; i++) {
                if (manager->pools_table[i])
                    dt_ecs_pool_resize(manager->pools_table[i], manager->sparse_size);
            }

            for (int i = 0; i < manager->filters_size; i++) {
                if (manager->filters[i])
                    filter_resize(manager->filters[i], manager->sparse_size);
            }

            for (DtEntity i = 0; i < manager->entities_ptr; i++) {
                manager->sparse_entities[i].children_iterator.enumerable =
                    &manager->sparse_entities[i];
            }
        }
        manager->sparse_entities[entity] =
            dt_entity_info_new(manager, entity, manager->component_count, manager->children_size);

        manager->sparse_entities[entity].children_iterator.enumerable =
            &manager->sparse_entities[entity];

        printf("[DEBUG]\t new entity \"%d\" was created\n", entity);
    }

    return entity;
}

DtEntityInfo dt_ecs_manager_get_entity(const DtEcsManager* manager, const DtEntity entity) {
    if (manager->entities_ptr <= entity || entity < 0)
        return DT_ENTITY_INFO_NULL;

    return manager->sparse_entities[entity];
}

DtEntityInfo dt_ecs_manager_get_parent(const DtEcsManager* manager, const DtEntity entity) {
    if (entity < 0 || entity > manager->entities_ptr)
        return DT_ENTITY_INFO_NULL;
    if (manager->sparse_entities[entity].parent == DT_ENTITY_NULL)
        return DT_ENTITY_INFO_NULL;
    return manager->sparse_entities[manager->sparse_entities[entity].parent];
}

void dt_ecs_manager_set_parent(const DtEcsManager* manager, const DtEntity child,
                               const DtEntity parent) {
    if (child > manager->entities_ptr || child == DT_ENTITY_NULL)
        return;
    if (parent > manager->entities_ptr && parent != DT_ENTITY_NULL)
        return;

    if (parent == child)
        return;

    if (manager->sparse_entities[child].parent != DT_ENTITY_NULL &&
        manager->sparse_entities[child].parent == parent)
        return;

    if (parent != DT_ENTITY_NULL && manager->sparse_entities[parent].parent == child) {
        dt_entity_info_set_parent(&manager->sparse_entities[parent], NULL);
        dt_entity_info_remove_child(&manager->sparse_entities[child],
                                    &manager->sparse_entities[parent]);
    }

    if (manager->sparse_entities[child].parent != DT_ENTITY_NULL) {
        dt_entity_info_remove_child(
            &manager->sparse_entities[manager->sparse_entities[child].parent],
            &manager->sparse_entities[child]);
    }

    DtEntityInfo* parent_ptr = parent == DT_ENTITY_NULL ? NULL : &manager->sparse_entities[parent];

    dt_entity_info_set_parent(&manager->sparse_entities[child], parent_ptr);
    if (parent_ptr)
        dt_entity_info_add_child(&manager->sparse_entities[parent],
                                 &manager->sparse_entities[child]);
}

void dt_ecs_manager_add_child(const DtEcsManager* manager, const DtEntity parent,
                              const DtEntity child) {
    if (child > manager->entities_ptr || child == DT_ENTITY_NULL)
        return;
    if (parent > manager->entities_ptr || parent == DT_ENTITY_NULL)
        return;

    if (parent == child)
        return;

    if (manager->sparse_entities[child].parent != DT_ENTITY_NULL &&
        manager->sparse_entities[child].parent == parent)
        return;

    if (manager->sparse_entities[parent].parent == child) {
        dt_entity_info_set_parent(&manager->sparse_entities[parent], NULL);
        dt_entity_info_remove_child(&manager->sparse_entities[child],
                                    &manager->sparse_entities[parent]);
    }

    if (manager->sparse_entities[child].parent != DT_ENTITY_NULL) {
        dt_entity_info_remove_child(
            &manager->sparse_entities[manager->sparse_entities[child].parent],
            &manager->sparse_entities[child]);
    }

    DtEntityInfo* parent_ptr = parent == DT_ENTITY_NULL ? NULL : &manager->sparse_entities[parent];

    dt_entity_info_set_parent(&manager->sparse_entities[child], parent_ptr);
    dt_entity_info_add_child(&manager->sparse_entities[parent], &manager->sparse_entities[child]);
}

void dt_ecs_manager_remove_child(const DtEcsManager* manager, const DtEntity parent,
                                 const DtEntity child) {
    if (child > manager->entities_ptr || child == DT_ENTITY_NULL)
        return;
    if (parent > manager->entities_ptr || parent == DT_ENTITY_NULL)
        return;

    if (parent == child)
        return;

    if (manager->sparse_entities[child].parent != DT_ENTITY_NULL &&
        manager->sparse_entities[child].parent != parent)
        return;

    dt_entity_info_set_parent(&manager->sparse_entities[child], NULL);

    dt_entity_info_remove_child(&manager->sparse_entities[parent],
                                &manager->sparse_entities[child]);
}

DtEntity* dt_ecs_manager_get_children(const DtEcsManager* manager, const DtEntity entity,
                                      u16* count) {
    *count = manager->sparse_entities[entity].children_count;

    return manager->sparse_entities[entity].children;
}

void dt_ecs_manager_kill_entity(DtEcsManager* manager, const DtEntity entity) {
    if (manager->entities_ptr <= entity || entity < 0)
        return;
    if (manager->sparse_entities[entity].gen < 0)
        return;

    if (manager->recycled_ptr == manager->recycled_size) {
        printf("[DEBUG]\t need to resize recycle array\n");

        manager->recycled_size = manager->recycled_size ? 2 * manager->recycled_size : 8;
        void* tmp =
            DT_REALLOC(manager->recycled_entities, manager->recycled_size * sizeof(DtEntity));

        if (!tmp) {
            printf("[DEBUG]entity kill realloc failed\n");
            exit(1);
        }

        manager->recycled_entities = tmp;
    }

    manager->recycled_entities[manager->recycled_ptr++] = entity;

    dt_entity_info_kill(&manager->sparse_entities[entity]);
    dt_entity_info_clear(&manager->sparse_entities[entity]);

    printf("[DEBUG]\t entity \"%d\" was killed\n", entity);
}

size_t dt_ecs_manager_get_entity_components_count(const DtEcsManager* manager,
                                                  const DtEntity entity) {
    if (manager->entities_ptr < entity || entity < 0)
        return 0;
    if (manager->sparse_entities[entity].gen < 0)
        return 0;

    return manager->sparse_entities[entity].component_count;
}

uint16_t dt_ecs_manager_get_entity_gen(const DtEcsManager* manager, const DtEntity entity) {
    if (manager->entities_ptr < entity || entity < 0)
        return 0;
    if (manager->sparse_entities[entity].gen < 0)
        return 0;

    return manager->sparse_entities[entity].gen;
}

void dt_ecs_manager_copy_entity(const DtEcsManager* manager, const DtEntity dst,
                                const DtEntity src) {
    if (manager->entities_ptr < dst || dst < 0)
        return;
    if (manager->sparse_entities[dst].gen < 0)
        return;

    if (manager->entities_ptr < src || src < 0)
        return;
    if (manager->sparse_entities[src].gen < 0)
        return;

    dt_entity_info_copy(&manager->sparse_entities[dst], &manager->sparse_entities[src]);
}
void dt_ecs_manager_reset_entity(const DtEcsManager* manager, const DtEntity entity) {
    if (manager->entities_ptr < entity || entity < 0)
        return;
    if (manager->sparse_entities[entity].gen < 0)
        return;

    dt_entity_info_reset(&manager->sparse_entities[entity]);
}

void dt_ecs_manager_clear_entity(const DtEcsManager* manager, const DtEntity entity) {
    if (manager->entities_ptr < entity || entity < 0)
        return;
    if (manager->sparse_entities[entity].gen < 0)
        return;

    dt_entity_info_clear(&manager->sparse_entities[entity]);
}

void dt_ecs_manager_entity_add_component(DtEcsManager* manager, const DtEntity entity,
                                                 const char* name, const void* data) {
    if (manager->entities_ptr < entity || entity < 0)
        return;
    if (manager->sparse_entities[entity].gen < 0)
        return;

    DtEcsPool* pool = dt_ecs_manager_get_pool(manager, name);

    dt_ecs_pool_add(pool, entity, data);
}

void dt_ecs_manager_entity_remove_component(DtEcsManager* manager, DtEntity entity,
                                                    const char* name) {
    if (manager->entities_ptr < entity || entity < 0)
        return;
    if (manager->sparse_entities[entity].gen < 0)
        return;

    DtEcsPool* pool = dt_ecs_manager_get_pool(manager, name);
    dt_ecs_pool_remove(pool, entity);
}

void dt_ecs_manager_add_pool(DtEcsManager* manager, DtEcsPool* pool) {
    const u64 hash = pool->hash;
    size_t idx = hash % manager->pools_table_size;
    const size_t start = idx;
    while (manager->pools_table[idx] != NULL) {
        if (hash == manager->pools_table[idx]->hash)
            return;

        idx = (idx + 1) % manager->pools_table_size;
        if (idx == start) {
            ecs_manager_resize_pools(manager);
            dt_ecs_manager_add_pool(manager, pool);
            return;
        }
    }

    DT_VEC_ADD(manager->pools, pool);
    manager->pools_table[idx] = pool;
    pool->ecs_manager_id = dt_vec_count(manager->pools) - 1;
}

DtEcsPool* dt_ecs_manager_get_pool(DtEcsManager* manager, const char* name) {
    const DtComponentData* data = dt_component_get_data_by_name(name);
    const u64 hash = data->hash;
    size_t idx = hash % manager->pools_table_size;
    const size_t start = idx;

    while (manager->pools_table[idx] != NULL &&
           hash != manager->pools_table[idx]->hash) {
        idx = (idx + 1) % manager->pools_table_size;
        if (idx == start) {
            DtEcsPool* pool = dt_ecs_pool_new_by_name(manager, name);
            dt_ecs_manager_add_pool(manager, pool);
            return pool;
        }
    }

    if (manager->pools_table[idx] == NULL) {
        DtEcsPool* pool = dt_ecs_pool_new_by_name(manager, name);
        manager->pools_table[idx] = pool;
        DT_VEC_ADD(manager->pools, pool);
        pool->ecs_manager_id = dt_vec_count(manager->pools) - 1;
        return pool;
    }

    return manager->pools_table[idx];
}

static void ecs_manager_resize_pools(DtEcsManager* manager) {
    const size_t old_size = manager->pools_table_size;
    manager->pools_table_size *= 2;

    DtEcsPool** old_pools = DT_STACK_ALLOC(old_size * sizeof(DtEcsPool*));
    memcpy(old_pools, manager->pools_table, old_size * sizeof(DtEcsPool*));

    void* tmp_pools =
        DT_REALLOC(manager->pools_table, sizeof(DtEcsPool*) * manager->pools_table_size);

    if (!tmp_pools) {
        printf("[DEBUG]\t Failed to allocate memory for new pools\n");
        exit(1);
    }

    manager->pools_table = tmp_pools;
    memset(manager->pools_table, 0, manager->pools_table_size * sizeof(DtEcsPool*));

    void* tmp_inc_filter =
        DT_REALLOC(manager->filter_by_include, manager->pools_table_size * sizeof(DtEcsFilter*));

    if (!tmp_inc_filter) {
        printf("[DEBUG]\t Failed to allocate memory for filters by include\n");
        exit(1);
    }

    manager->filter_by_include = tmp_inc_filter;
    memset(manager->filter_by_include + old_size, 0,
           (manager->pools_table_size - old_size) * sizeof(DtEcsFilter*));

    void* tmp_exc_filters = DT_REALLOC(manager->filter_by_exclude,
                                       manager->pools_table_size * sizeof(DT_VEC(DtEcsFilter*)));

    if (!tmp_exc_filters) {
        printf("[DEBUG]\t Failed to allocate memory for filters by exclude\n");
        exit(1);
    }

    manager->filter_by_exclude = tmp_exc_filters;
    memset(manager->filter_by_exclude + old_size, 0,
           (manager->pools_table_size - old_size) * sizeof(DT_VEC(DtEcsFilter*)));

    for (int i = 0; i < old_size; i++) {
        u16 idx = old_pools[i]->hash % manager->pools_table_size;

        while (manager->pools_table[idx] != NULL) {
            idx = (1 + idx) % manager->pools_table_size;
        }

        manager->pools_table[idx] = old_pools[i];
    }
}

static DtEcsFilter* get_filter(DtEcsManager* manager, const DtEcsMask mask) {
    if (manager->filters_size == 0) {
        manager->filters_size = 10;
        manager->filters = DT_CALLOC(sizeof(DtEcsFilter*), manager->filters_size);
    }

    const uint64_t hash = mask.hash;
    DtEcsFilter* filter = NULL;
    size_t idx = hash % manager->filters_size;
    const size_t start = idx;

    bool was_resized = false;

    while (manager->filters[idx] != NULL) {
        if (manager->filters[idx]->mask.hash == hash) {
            filter = manager->filters[idx];
        }

        idx = (idx + 1) % manager->filters_size;
        if (idx == start) {
            ecs_manager_resize_filters(manager);
            was_resized = true;
        }
    }

    if (filter)
        return filter;

    filter = filter_new(manager, mask);

    if (was_resized) {
        idx = hash % manager->filters_size;
        while (manager->filters[idx] != NULL) {
            idx = (idx + 1) % manager->filters_size;
        }
    }

    manager->filters[idx] = filter;

    for (int i = 0; i < mask.include_count; i++) {
        DT_VEC(DtEcsFilter*) list = manager->filter_by_include[mask.include_pools[i]];

        if (!list) {
            list = DT_VEC_NEW(DtEcsFilter*, mask.include_count);
            manager->filter_by_include[mask.include_pools[i]] = list;
        }

        DT_VEC_ADD(list, filter);
    }

    for (int i = 0; i < mask.exclude_count; i++) {
        DT_VEC(DtEcsFilter*) list = manager->filter_by_exclude[mask.exclude_pools[i]];

        if (!list) {
            list = DT_VEC_NEW(DtEcsFilter*, mask.exclude_count);
            manager->filter_by_exclude[mask.exclude_pools[i]] = list;
        }

        DT_VEC_ADD(list, filter);
    }

    for (DtEntity i = 0; i < manager->entities_ptr; i++) {
        if (manager->sparse_entities[i].gen > -1 && is_mask_compatible(manager, mask, i)) {
            filter_add_entity(filter, i);
        }
    }

    return filter;
}

static void ecs_manager_resize_filters(DtEcsManager* manager) {
    u16 old_size = manager->filters_size;
    manager->filters_size = manager->filters_size ? manager->filters_size * 2 : 10;
    DtEcsFilter** old_filters = DT_STACK_ALLOC(sizeof(DtEcsFilter*) * old_size);
    memcpy(old_filters, manager->filters, sizeof(DtEcsFilter*) * old_size);

    for (int i = 0; i < old_size; i++) {
        u16 idx = old_filters[i]->mask.hash % manager->filters_size;

        while (manager->filters[idx] != NULL) {
            idx = (idx + 1) % manager->filters_size;
        }

        manager->filters[idx] = old_filters[i];

        manager->filters[idx]->entities.entities_iterator.enumerable =
            &manager->filters[idx]->entities;
        manager->filters[idx]->entities.items_iterator.enumerable =
            &manager->filters[idx]->entities;
    }
}

void dt_on_entity_change(const DtEcsManager* manager, const DtEntity entity,
                         const u16 ecs_manager_component_id, const bool added) {
    DT_VEC(DtEcsFilter*)
    include_list = manager->filter_by_include[ecs_manager_component_id];
    DT_VEC(DtEcsFilter*)
    exclude_list = manager->filter_by_exclude[ecs_manager_component_id];

    if (added) {
        dt_entity_info_add_component(&manager->sparse_entities[entity], ecs_manager_component_id);
    } else {
        dt_entity_info_remove_component(&manager->sparse_entities[entity],
                                        ecs_manager_component_id);
    }

    if (added) {
        if (include_list != NULL) {
            FOREACH(DtEcsFilter*, filter, DT_VEC_ITERATOR(include_list), {
                if (is_mask_compatible(manager, filter->mask, entity)) {
                    filter_add_entity(filter, entity);
                }
            });
        }
        if (exclude_list != NULL) {
            FOREACH(DtEcsFilter*, filter, DT_VEC_ITERATOR(exclude_list), {
                if (is_mask_compatible_without(manager, filter->mask, entity,
                                               ecs_manager_component_id)) {
                    filter_remove_entity(filter, entity);
                }
            });
        }
    } else {
        if (include_list != NULL) {
            FOREACH(DtEcsFilter*, filter, DT_VEC_ITERATOR(include_list), {
                if (is_mask_compatible(manager, filter->mask, entity)) {
                    filter_remove_entity(filter, entity);
                }
            });
        }
        if (exclude_list != NULL) {
            FOREACH(DtEcsFilter*, filter, DT_VEC_ITERATOR(exclude_list), {
                if (is_mask_compatible_without(manager, filter->mask, entity,
                                               ecs_manager_component_id)) {
                    filter_add_entity(filter, entity);
                }
            });
        }
    }
}

static int is_mask_compatible(const DtEcsManager* manager, const DtEcsMask mask,
                              const DtEntity entity) {
    for (int i = 0; i < mask.include_count; i++) {
        if (!dt_ecs_pool_has(manager->pools[mask.include_pools[i]], entity)) {
            return 0;
        }
    }
    for (int i = 0; i < mask.exclude_count; i++) {
        if (dt_ecs_pool_has(manager->pools[mask.exclude_pools[i]], entity)) {
            return 0;
        }
    }
    return 1;
}

static int is_mask_compatible_without(const DtEcsManager* manager, const DtEcsMask filterMask,
                                      const DtEntity entity, const int ecs_manager_component_id) {
    for (int i = 0; i < filterMask.include_count; i++) {
        const size_t typeId = filterMask.include_pools[i];
        if (typeId == ecs_manager_component_id ||
            !dt_ecs_pool_has(manager->pools[typeId], entity)) {
            return 0;
        }
    }

    for (int i = 0; i < filterMask.exclude_count; i++) {
        const size_t typeId = filterMask.exclude_pools[i];
        if (typeId != ecs_manager_component_id &&
            !dt_ecs_pool_has(manager->pools[typeId], entity)) {
            return 0;
        }
    }
    return 1;
}

void dt_ecs_manager_free(DtEcsManager* manager) {
    free(manager->sparse_entities);
    free(manager->recycled_entities);

    for (int i = 0; i < dt_vec_count(manager->pools); i++) {
        if (manager->filter_by_include[i])
            dt_vec_free(manager->filter_by_include[i]);

        if (manager->filter_by_exclude[i])
            dt_vec_free(manager->filter_by_exclude[i]);
    }

    free(manager->filter_by_include);
    free(manager->filter_by_exclude);

    FOREACH(DtEcsPool*, pool, DT_VEC_ITERATOR(manager->pools), {
        dt_ecs_pool_free(pool);
    });

    dt_vec_free(manager->pools);
    free(manager->pools_table);

    for (int i = 0; i < manager->filters_size; i++) {
        if (manager->filters[i] == NULL)
            continue;

        if (manager->filters[i]->mask.include_pools)
            free(manager->filters[i]->mask.include_pools);

        if (manager->filters[i]->mask.exclude_pools)
            free(manager->filters[i]->mask.exclude_pools);

        filter_free(manager->filters[i]);
    }

    free(manager);
}

void dt_remove_tool_components(const DtEcsManager* manager) { remove_hierarchy_dirty_tag(manager); }

static void remove_hierarchy_dirty_tag(const DtEcsManager* manager) {
    for (DtEntity i = 0; i < manager->entities_ptr; i++) {
        if (manager->sparse_entities[i].gen < 0)
            continue;

        dt_ecs_pool_remove(manager->hierarchy_dirty_pool, i);
    }
}
