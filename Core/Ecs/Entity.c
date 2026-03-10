#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DtAllocators.h"
#include "DtEcs.h"

static void entity_info_children_start(void* data);
static void* entity_info_children_current(void* data);
static bool entity_info_children_has_current(void* data);
static void entity_info_children_next(void* data);

static void entity_container_items_start(void* data);
static void* entity_container_items_current(void* data);
static bool entity_container_items_has_current(void* data);
static void entity_container_items_next(void* data);

static void entity_container_entities_start(void* data);
static void* entity_container_entities_current(void* data);
static bool entity_container_entities_has_current(void* data);
static void entity_container_entities_next(void* data);

static void default_entity_item_reset(void* data, size_t);
static void default_entity_item_copy(void* dst, const void* src, size_t);

DtEntityInfo dt_entity_info_new(DtEcsManager* manager, const DtEntity id, u16 component_count,
                                const DtEntity children_size) {
    component_count = component_count ? component_count : 10;

    return (DtEntityInfo) {
        .manager = manager,

        .id = id,

        .components = DT_CALLOC(component_count, sizeof(u16)),
        .component_size = component_count,
        .component_count = 0,

        .parent = DT_ENTITY_NULL,

        .children = NULL,
        .children_size = children_size,
        .base_children_size = children_size,
        .children_count = 0,
        .children_iterator =
            (DtIterator) {
                .start = entity_info_children_start,
                .current = entity_info_children_current,
                .has_current = entity_info_children_has_current,
                .next = entity_info_children_next,
            },

        .alive = true,
        .gen = 0,
    };
}

void dt_entity_info_reuse(DtEntityInfo* info) {
    if (info->gen > 0)
        return;

    info->gen++;
    info->alive = true;

    info->component_count = info->component_count;
    info->children_count = 0;
}

void dt_entity_info_set_parent(DtEntityInfo* info, DtEntityInfo* parent) {
    if (parent) {
        info->parent = parent->id;
        printf("[DEBUG]\t entity \"%d\" has become parent of entity \"%d\"\n", parent->id,
               info->id);
    } else {
        info->parent = DT_ENTITY_NULL;
        printf("[DEBUG]\t entity \"DT_ENTITY_NULL\" has become parent of entity \"%d\"\n",
               info->id);
    }
}

void dt_entity_info_add_child(DtEntityInfo* info, DtEntityInfo* child) {
    if (info->children == NULL) {
        info->children_size = info->children_size ? info->children_size * 2 : 10;
        info->children = DT_CALLOC(info->base_children_size, sizeof(DtEntityInfo*));
    }

    for (int i = 0; i < info->children_count + 1; i++) {
        if (i == info->children_size) {
            info->children_size = info->children_size ? info->children_size * 2 : 10;
            void* tmp = DT_REALLOC(info->children, info->children_size * sizeof(DtEntityInfo*));

            if (!tmp) {
                printf("[DEBUG]\t Memory allocation exception\n");
            }

            info->children = tmp;
        }

        if (i == info->children_count) {
            info->children[i] = child->id;
            info->children_count++;
            printf("[DEBUG]\t entity \"%d\" has become child of entity \"%d\"\n", child->id,
                   info->id);
            return;
        }

        if (info->children[i] == child->id)
            return;
    }
}

void dt_entity_info_remove_child(DtEntityInfo* info, DtEntityInfo* child) {
    for (int i = 0; i < info->children_count; i++) {
        if (info->children[i] != child->id)
            continue;

        info->children[i] = info->children[--info->children_count];
        printf("[DEBUG]\t entity \"%d\" ceased to be child of entity \"%d\"\n", child->id,
               info->id);
    }
}

void dt_entity_info_remove_all_children(DtEntityInfo* info) {
    for (int i = 0; i < info->children_count; i++) {
        dt_ecs_manager_set_parent(info->manager, info->children[i], DT_ENTITY_NULL);
    }

    info->children_count = 0;
}

void dt_entity_info_add_component(DtEntityInfo* info, const u16 id) {
    for (int i = 0; i < info->component_count + 1; i++) {
        if (info->components[i] == id && info->component_count != i)
            return;

        if (i == info->component_size) {
            info->component_size = info->component_size ? info->component_size * 2 : 10;
            void* tmp = DT_REALLOC(info->components, info->component_size * sizeof(int));

            if (!tmp) {
                printf("[DEBUG] entity info realloc exception\n");
            }

            info->components = tmp;
        }

        if (i == info->component_count) {
            info->components[i] = id;
            info->component_count++;
            return;
        }
    }
}

void dt_entity_info_remove_component(DtEntityInfo* info, u16 id) {
    for (int i = 0; i < info->component_count; i++) {
        if (info->components[i] != id)
            continue;

        info->components[i] = info->components[--info->component_count];
    }
}

void dt_entity_info_reset(DtEntityInfo* info) {
    if (info->gen < 0)
        return;

    for (int i = 0; i < info->component_count; i++) {
        DtEcsPool* pool = info->manager->pools[info->components[i]];
        dt_ecs_pool_reset(pool, info->id);
    }

    printf("[DEBUG]\t entity \"%d\" has reseted\n", info->id);
}

void dt_entity_info_clear(DtEntityInfo* info) {
    if (info->gen < 0)
        return;

    for (int i = info->component_count - 1; i > -1; i--) {
        DtEcsPool* pool = info->manager->pools[info->components[i]];
        dt_ecs_pool_remove(pool, info->id);
    }

    info->component_count = 0;

    printf("[DEBUG]\t entity \"%d\" has cleared\n", info->id);
}

void dt_entity_info_copy(DtEntityInfo* dst, const DtEntityInfo* src) {
    if (dst->gen < 0)
        return;
    if (src->gen < 0)
        return;

    if (dst->component_size < src->component_size) {
        dst->component_size = src->component_size;

        void* tmp = realloc(dst->components, dst->component_size * sizeof(int));

        if (!tmp) {
            printf("[DEBUG] entity info realloc exception]\n");
        }

        dst->components = tmp;
    }

    memcpy(dst->components, src->components, dst->component_size * sizeof(int));

    for (int i = 0; i < src->component_count; i++) {
        DtEcsPool* pool = dst->manager->pools[dst->components[i]];
        dt_ecs_pool_copy(pool, dst->id, src->id);
    }
}

void dt_entity_info_kill(DtEntityInfo* info) {
    if (info->gen < 0)
        return;

    dt_entity_info_remove_all_children(info);
    dt_ecs_manager_remove_child(info->manager, info->parent, info->id);
    info->alive = false;
}

static void entity_info_children_start(void* data) {
    ((DtEntityInfo*) data)->children_iterator_ptr = -1;
}

static void* entity_info_children_current(void* data) {
    DtEntityInfo* info = data;
    return info->children + info->children_iterator_ptr;
}

static bool entity_info_children_has_current(void* data) {
    DtEntityInfo* info = data;
    return info->children_iterator_ptr < info->children_count;
}

static void entity_info_children_next(void* data) {
    DtEntityInfo* info = data;

    info->children_iterator_ptr++;
}

DtEntityContainer dt_entity_container_new(const u32 item_size, const DtEntity dense_size,
                                          const DtEntity sparse_size, const DtEntity recycle_size,
                                          const DtResetItemHandler auto_reset,
                                          const DtCopyItemHandler auto_copy) {
    DtEntityContainer ec = {
        .entities = DT_CALLOC(dense_size, sizeof(DtEntity)),
        .dense_items = DT_CALLOC(dense_size, item_size),
        .item_size = item_size,
        .dense_size = dense_size,
        .count = 0,

        .sparce_entities = DT_CALLOC(sparse_size, sizeof(DtEntity)),
        .sparse_size = sparse_size,

        .recycle_entities = DT_CALLOC(recycle_size, sizeof(DtEntity)),
        .recycle_ptr = 0,
        .recycle_size = recycle_size,

        .auto_reset = auto_reset ? auto_reset : default_entity_item_reset,
        .auto_copy = auto_copy ? auto_copy : default_entity_item_copy,

        .items_iterator =
            (DtIterator) {
                .start = entity_container_items_start,
                .current = entity_container_items_current,
                .has_current = entity_container_items_has_current,
                .next = entity_container_items_next,
            },
        .entities_iterator =
            (DtIterator) {
                .start = entity_container_entities_start,
                .current = entity_container_entities_current,
                .has_current = entity_container_entities_has_current,
                .next = entity_container_entities_next,
            },
    };

    for (size_t i = 0; i < sparse_size; i++) {
        ec.sparce_entities[i] = DT_ENTITY_NULL;
    }


    return ec;
}

void dt_entity_container_add(DtEntityContainer* container, const DtEntity entity,
                             const void* data) {
    if (entity > container->sparse_size)
        return;
    if (dt_entity_container_has(container, entity))
        return;

    DtEntity e;

    if (container->recycle_ptr) {
        e = container->recycle_entities[container->recycle_ptr--];
    } else {
        if (container->count == container->dense_size) {
            container->dense_size = container->dense_size ? container->dense_size * 2 : 10;
            void* tmp =
                DT_REALLOC(container->dense_items, container->dense_size * container->item_size);

            if (!tmp) {
                printf("[DEBUG] entity container realloc exception\n");
                exit(1);
            }

            container->dense_items = tmp;

            tmp = DT_REALLOC(container->entities, container->dense_size * sizeof(DtEntity));

            if (!tmp) {
                printf("[DEBUG] entity container realloc exception\n");
                exit(1);
            }

            container->entities = tmp;
        }

        e = container->count;
    }

    void* target = (u8*) container->dense_items + e * container->item_size;

    if (data) {
        container->auto_copy(target, data, container->item_size);
    } else {
        container->auto_reset(target, container->item_size);
    }

    container->entities[e] = entity;
    container->sparce_entities[entity] = e;

    container->count++;
}

void dt_entity_container_remove(DtEntityContainer* container, const DtEntity entity) {
    if (entity > container->sparse_size)
        return;
    if (!dt_entity_container_has(container, entity))
        return;


    void* entity_data =
        container->dense_items + container->sparce_entities[entity] * container->item_size;

    if (container->count > 1) {
        const void* last_entity =
            (u8*) container->dense_items +
            container->sparce_entities[container->count - 1] * container->item_size;

        container->auto_copy(entity_data, last_entity, container->item_size);

        const DtEntity last_id = container->entities[container->count - 1];
        container->sparce_entities[last_id] = container->sparce_entities[entity];
        container->entities[container->sparce_entities[entity]] = last_id;
    }

    container->sparce_entities[entity] = DT_ENTITY_NULL;
    container->count--;

    if (container->recycle_ptr >= container->recycle_size) {
        container->recycle_size = container->recycle_size ? 2 * container->recycle_size : 8;
        void* tmp =
            DT_REALLOC(container->recycle_entities, container->recycle_size * sizeof(DtEntity));

        if (!tmp) {
            printf("[DEBUG] entity container realloc exception]\n");
            exit(1);
        }

        container->recycle_entities = tmp;
    }

    container->recycle_entities[container->recycle_ptr++] = entity;
}

inline int dt_entity_container_has(const DtEntityContainer* container, const DtEntity entity) {
    return entity < container->sparse_size && container->sparce_entities[entity] != DT_ENTITY_NULL;
}

void* dt_entity_container_get(const DtEntityContainer* container, DtEntity entity) {
    if (entity > container->sparse_size)
        return NULL;

    if (container->sparce_entities[entity] == DT_ENTITY_NULL) {
        return NULL;
    }

    return (char*) container->dense_items +
           container->sparce_entities[entity] * container->item_size;
}

void dt_entity_container_reset(DtEntityContainer* container, const DtEntity entity) {
    if (!dt_entity_container_has(container, entity))
        return;

    container->auto_reset((u8*) container->dense_items +
                              container->sparce_entities[entity] * container->item_size,
                          container->item_size);
}

void dt_entity_container_copy(DtEntityContainer* container, const DtEntity dst,
                              const DtEntity src) {
    if (!dt_entity_container_has(container, src))
        return;

    if (dt_entity_container_has(container, dst))
        container->auto_copy(
            (u8*) container->dense_items + container->sparce_entities[dst] * container->item_size,
            (u8*) container->dense_items + container->sparce_entities[src] * container->item_size,
            container->item_size);
    else
        dt_entity_container_add(container, dst,
                                (u8*) container->dense_items +
                                    container->sparce_entities[src] * container->item_size);
}

void dt_entity_container_resize(DtEntityContainer* container, u16 new_size) {
    void* tmp = realloc(container->sparce_entities, new_size);

    if (!tmp) {
        printf("[DEBUG]memory allocation exception");
        exit(1);
    }

    container->sparce_entities = tmp;
}

static void default_entity_item_reset(void* data, const size_t size) { memset(data, 0, size); }

void dt_entity_container_free(DtEntityContainer* container) {
    free(container->dense_items);
    free(container->entities);
    free(container->sparce_entities);
    free(container->recycle_entities);
}

static void default_entity_item_copy(void* dst, const void* src, const size_t size) {
    memcpy(dst, src, size);
}

static void entity_container_items_start(void* data) {
    ((DtEntityContainer*) data)->items_iterator_ptr = 0;
}

static void* entity_container_items_current(void* data) {
    const DtEntityContainer* container = data;
    return container->dense_items + container->items_iterator_ptr * container->item_size;
}

static bool entity_container_items_has_current(void* data) {
    DtEntityContainer* container = data;
    return container->count > container->items_iterator_ptr; // lil popA
}

static void entity_container_items_next(void* data) {
    DtEntityContainer* container = data;
    container->items_iterator_ptr++;
}

static void entity_container_entities_start(void* data) {
    ((DtEntityContainer*) data)->entities_iterator_ptr = 0;
}

static void* entity_container_entities_current(void* data) {
    const DtEntityContainer* container = data;
    return &container->entities[container->items_iterator_ptr];
}

static bool entity_container_entities_has_current(void* data) {
    DtEntityContainer* container = data;
    return container->count > container->entities_iterator_ptr;
}

static void entity_container_entities_next(void* data) {
    DtEntityContainer* container = data;
    container->entities_iterator_ptr++;
}
