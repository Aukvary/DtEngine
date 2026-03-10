#ifndef DT_ECS_H
#define DT_ECS_H

/*=============================================================================
 *                               Включение заголовков
 *============================================================================*/

#include <DtNumericalTypes.h>
#include <stdbool.h>
#include "Collections/Collections.h"

/*=============================================================================
 *                              Базовые определения
 *============================================================================*/

typedef u16 DtEntity;
#define DT_ENTITY_NULL (0xFFFF)
#define DT_KILLED_ENTITY (0xFFFF)

/*=============================================================================
 *                        Определения типов и структур
 *============================================================================*/

/**
 * @brief Указатель на функцию для сброса элемента в EntityContainer
 * @param data Указатель на данные
 * @param size Размер данных
 */
typedef void (*DtResetItemHandler)(void* data, size_t size);

/**
 * @brief Указатель на функцию для копирования элемента в EntityContainer
 * @param dst Указатель на назначение
 * @param src Указатель на источник
 * @param size Размер данных
 */
typedef void (*DtCopyItemHandler)(void* dst, const void* src, size_t size);

/**
 * @brief Основной обработчик ECS
 */
typedef struct DtEcsManager DtEcsManager;
// TODO: comments
typedef struct DtEcsFilter DtEcsFilter;

/**
 * @brief Информация о сущности
 */
typedef struct {
    DtEcsManager* manager;

    DtEntity id;
    u16* components;
    u16 component_size;
    u16 component_count;

    DtEntity parent;
    DtEntity* children;
    u16 children_size;
    u16 base_children_size;
    u16 children_count;
    DtIterator children_iterator;
    u16 children_iterator_ptr;

    bool alive;
    u16 gen;
} DtEntityInfo;

typedef struct DtRawEntity {
    u16* components;
    u16 component_count;

    struct DtRawEntity* parent;
    struct DtRawEntity* children;
    u16 children_count;
} DtRawEntity;

/*=============================================================================
 *                             Макросы для сущностей
 *============================================================================*/

/**
 * @brief Информация о сущности с NULL id
 */
#define DT_ENTITY_INFO_NULL                                                                        \
    (DtEntityInfo) { .id = DT_ENTITY_NULL }

/*=============================================================================
 *                          Функции для работы с EntityInfo
 *============================================================================*/

DtEntityInfo dt_entity_info_new(DtEcsManager* manager, DtEntity id, u16 component_count,
                                DtEntity children_size);
void dt_entity_info_reuse(DtEntityInfo* info);
void dt_entity_info_set_parent(DtEntityInfo* info, DtEntityInfo* parent);
void dt_entity_info_add_child(DtEntityInfo* info, DtEntityInfo* child);
void dt_entity_info_remove_child(DtEntityInfo* info, DtEntityInfo* child);
void dt_entity_info_remove_all_children(DtEntityInfo* info);

void dt_entity_info_add_component(DtEntityInfo* info, u16 id);
void dt_entity_info_remove_component(DtEntityInfo* info, u16 id);

void dt_entity_info_reset(DtEntityInfo* info);
void dt_entity_info_clear(DtEntityInfo* info);
void dt_entity_info_copy(DtEntityInfo* dst, const DtEntityInfo* src);
void dt_entity_info_kill(DtEntityInfo* info);

/*=============================================================================
 *                         Контейнер сущностей (EntityContainer)
 *============================================================================*/

/**
 * @brief Контейнер для хранения данных сущностей
 * @note Используйте dense_items/dense_entities и count для итерации
 */
typedef struct {
    DtEntity* entities;
    void* dense_items;
    u32 item_size;
    DtEntity dense_size;
    DtEntity count;

    DtEntity* sparce_entities;
    DtEntity sparse_size;

    DtEntity* recycle_entities;
    DtEntity recycle_ptr;
    DtEntity recycle_size;

    DtIterator items_iterator;
    DtEntity items_iterator_ptr;

    DtIterator entities_iterator;
    DtEntity entities_iterator_ptr;

    DtResetItemHandler auto_reset;
    DtCopyItemHandler auto_copy;
} DtEntityContainer;

DtEntityContainer dt_entity_container_new(u32 item_size, DtEntity dense_size, DtEntity sparse_size,
                                          DtEntity recycle_size, DtResetItemHandler auto_reset,
                                          DtCopyItemHandler auto_copy);
void dt_entity_container_add(DtEntityContainer* container, DtEntity entity, const void* data);
int dt_entity_container_has(const DtEntityContainer* container, DtEntity entity);
void* dt_entity_container_get(const DtEntityContainer* container, DtEntity entity);
void dt_entity_container_reset(DtEntityContainer* container, DtEntity entity);
void dt_entity_container_copy(DtEntityContainer* container, DtEntity dst, DtEntity src);
void dt_entity_container_remove(DtEntityContainer* container, DtEntity entity);
void dt_entity_container_resize(DtEntityContainer* container, u16 size);
void dt_entity_container_free(DtEntityContainer* container);

/*=============================================================================
 *                         Конфигурация ECS менеджера
 *============================================================================*/

/**
 * @brief Конфигурация для инициализации ECS менеджера
 */
typedef struct DtEcsManagerConfig {
    u16 dense_size;
    u16 sparse_size;
    u16 recycle_size;
    u16 children_size;
    u16 components_count;
    u16 pools_size;
    u32 masks_size;
    u32 include_mask_count;
    u32 exclude_mask_count;
    u32 filters_size;
} DtEcsManagerConfig;

/*=============================================================================
 *                               Пул компонентов (EcsPool)
 *============================================================================*/

typedef enum { TAG_POOL, COMPONENT_POOL } PoolType;

/**
 * @brief Пул данных компонентов ECS
 */
typedef struct {
    const DtEcsManager* manager;

    u16 count;

    PoolType type;

    const char* name;
    u64 hash;
    u16 ecs_manager_id;

    void* data;

    void (*add)(void*, DtEntity, const void*);
    void* (*get)(const void*, DtEntity);
    bool (*has)(const void*, DtEntity);
    void (*reset)(void*, DtEntity);
    void (*copy)(void*, DtEntity, DtEntity);
    void (*remove)(void*, DtEntity);
    void (*resize)(void*, u16);
    void (*free)(void*);

    DtIterator iterator;
} DtEcsPool;

/**
 * @brief Пул компонентов с данными
 */
typedef struct {
    DtEcsPool pool;
    DtEntityContainer entities;
} DtComponentPool;


typedef u16 TagBucket;
#define BUCKET_SIZE (sizeof(TagBucket) * 8)
/**
 * @brief Пул компонентов без данных (теги)
 */
typedef struct {
    DtEcsPool pool;
    TagBucket* buckets;
    size_t size;
    size_t max_entities;

    TagBucket iterator_bucket;
    DtEntity iterator_entity;
    DtEntity iterator_ptr;
} DtTagPool;

/*=============================================================================
 *                          Макросы для создания пулов
 *============================================================================*/

/**
 * @brief Создает новый пул ECS
 * @param T Тип пула
 * @param manager ECS менеджер, содержащий пул
 * @return Новый пул ECS
 */
#define DT_ECS_POOL_NEW(T, manager) ecs_pool_new(manager, #T, sizeof(T))
#define DT_COMPONENT_POOL_NEW(T, manager, reset, copy)                                             \
    dt_component_pool_new(manager, #T, sizeof(T), reset, copy)

/*=============================================================================
 *                         Функции для работы с пулами
 *============================================================================*/

DtEcsPool* dt_ecs_pool_new(const DtEcsManager* manager, const char* name, u16 size);
DtEcsPool* dt_ecs_pool_new_by_name(const DtEcsManager* manager, const char* name);
DtEcsPool* dt_component_pool_new(const DtEcsManager* manager, const char* name, u16 size,
                                 DtResetItemHandler reset_handler, DtCopyItemHandler copy_handler);
DtEcsPool* dt_tag_pool_new(const DtEcsManager* manager, const char* name);

void dt_ecs_pool_add(DtEcsPool* pool, DtEntity entity, const void* data);
void* dt_ecs_pool_get(const DtEcsPool* pool, DtEntity entity);
int dt_ecs_pool_has(const DtEcsPool* pool, DtEntity entity);
void dt_ecs_pool_reset(DtEcsPool* pool, DtEntity entity);
void dt_ecs_pool_copy(DtEcsPool* pool, DtEntity dst, DtEntity src);
void dt_ecs_pool_remove(DtEcsPool* pool, DtEntity entity);
void dt_ecs_pool_resize(DtEcsPool* pool, u64 size);
void dt_ecs_pool_free(DtEcsPool* pool);

/*=============================================================================
 *                              Маски ECS (EcsMask)
 *============================================================================*/

/**
 * @brief Маска с данными о фильтре ECS
 */
typedef struct {
    DtEcsManager* manager;

    u16* include_pools;
    u16 include_size;
    u16 include_count;

    u16* exclude_pools;
    u16 exclude_size;
    u16 exclude_count;
    u64 hash;
} DtEcsMask;

DtEcsMask dt_mask_new(DtEcsManager* manager, u16 inc_size, u16 exc_size);
void dt_mask_inc(DtEcsMask* mask, u16 ecs_manager_component_id);
void dt_mask_exc(DtEcsMask* mask, u16 ecs_manager_component_id);
DtEcsFilter* dt_mask_end(DtEcsMask mask);

/*=============================================================================
 *                              Фильтр ECS (DtEcsFilter)
 *============================================================================*/

/**
 * @brief Архетип-контейнер для сущностей с идентичными компонентами
 */
struct DtEcsFilter {
    DtEcsManager* manager;
    DtEcsMask mask;
    DtEntityContainer entities;
};

/*=============================================================================
 *                              ECS Менеджер (DtEcsManager)
 *============================================================================*/

struct DtEcsManager {
    DtEntityInfo* sparse_entities;
    DtEntity sparse_size;
    DtEntity entities_ptr;

    DtEntity cfg_dense_size;
    DtEntity cfg_recycle_size;
    u16 component_count;
    DtEntity children_size;

    DtEntity* recycled_entities;
    DtEntity recycled_size;
    DtEntity recycled_ptr;

    DT_VEC(DtEcsPool*) pools;
    DtEcsPool** pools_table;
    size_t pools_table_size;

    size_t include_mask_count;
    size_t exclude_mask_count;

    DtEcsFilter** filters;
    size_t filters_size;
    size_t filters_count;

    DT_VEC(DtEcsFilter*) * filter_by_include;
    DT_VEC(DtEcsFilter*) * filter_by_exclude;

    DtEcsPool* hierarchy_dirty_pool;
};

/*=============================================================================
 *                        Макросы для работы с менеджером
 *============================================================================*/

/**
 * @brief Возвращает пул типа T из менеджера
 * @param manager Менеджер, в котором ищем пул
 * @param T Тип компонента
 * @note Если менеджер не имеет пула типа T, создается новый пул
 */
#define DT_ECS_MANAGER_GET_POOL(manager, T) ({ dt_ecs_manager_get_pool((manager), #T); })

#define DT_ECS_MANAGER_ADD_TO_POOL(manager, T, entity, data)                                       \
    ({ dt_ecs_manager_entity_add_component(manager, entity, #T, data); })

#define DT_ECS_MANAGER_REMOVE_FROM_POOL(manager, T, entity)                                        \
    ({ dt_ecs_manager_entity_remove_component(manager, entity, #T); })

/**
 * @brief Возвращает маску с типом T по умолчанию
 * @param manager Менеджер, содержащий маску
 * @param T Тип маски по умолчанию
 */
#define DT_GET_MASK(manager, T)                                                                    \
    dt_mask_new(manager, manager->include_mask_count, manager->exclude_mask_count,                 \
                (DT_ECS_MANAGER_GET_POOL(manager, T))->info)

/**
 * @brief Включает тип T в маску
 * @param mask Маска, в которую включаем T
 * @param T Тип, который включаем
 */
#define DT_MASK_INC(mask, T)                                                                       \
    dt_mask_inc(&(mask), (DT_ECS_MANAGER_GET_POOL(manager, T))->ecs_manager_id)

/**
 * @brief Исключает тип T из маски
 * @param mask Маска, из которой исключаем T
 * @param T Тип, который исключаем
 */
#define DT_MASK_EXC(mask, T)                                                                       \
    dt_mask_exc(&(mask), (DT_ECS_MANAGER_GET_POOL(manager, T))->ecs_manager_id)

/*=============================================================================
 *                     Функции для работы с ECS менеджером
 *============================================================================*/

DtEcsManager* dt_ecs_manager_new(DtEcsManagerConfig cfg);
DtEntity dt_ecs_manager_new_entity(DtEcsManager* manager);
DtEntity dt_ecs_manager_new_entity_from(DtEcsManager* manager,
                                        DtEntityInfo info); // TODO: implement
DtEntity dt_ecs_manager_new_entity_from_prefab(DtEcsManager* manager,
                                               DtRawEntity entity); // TODO: implement
DtEntityInfo dt_ecs_manager_get_entity(const DtEcsManager* manager, DtEntity entity);
DtEntityInfo dt_ecs_manager_get_parent(const DtEcsManager* manager, DtEntity entity);
void dt_ecs_manager_set_parent(const DtEcsManager* manager, DtEntity child, DtEntity parent);
void dt_ecs_manager_add_child(const DtEcsManager* manager, DtEntity parent, DtEntity child);
void dt_ecs_manager_remove_child(const DtEcsManager* manager, DtEntity parent, DtEntity child);
DtEntity* dt_ecs_manager_get_children(const DtEcsManager* manager, DtEntity entity, u16* count);
size_t dt_ecs_manager_get_entity_components_count(const DtEcsManager* manager, DtEntity entity);
uint16_t dt_ecs_manager_get_entity_gen(const DtEcsManager* manager, DtEntity entity);
void dt_ecs_manager_copy_entity(const DtEcsManager* manager, DtEntity dst, DtEntity src);
void dt_ecs_manager_reset_entity(const DtEcsManager* manager, DtEntity entity);
void dt_ecs_manager_clear_entity(const DtEcsManager* manager, DtEntity entity);
void dt_ecs_manager_entity_add_component(DtEcsManager* manager, DtEntity entity, const char* name,
                                         const void* data);
void dt_ecs_manager_entity_remove_component(DtEcsManager* manager, DtEntity entity,
                                            const char* name);
void dt_ecs_manager_kill_entity(DtEcsManager* manager, DtEntity entity);
void dt_ecs_manager_add_pool(DtEcsManager* manager, DtEcsPool* pool);
DtEcsPool* dt_ecs_manager_get_pool(DtEcsManager* manager, const char* name);
void dt_on_entity_change(const DtEcsManager* manager, DtEntity entity, u16 ecs_manager_component_id,
                         bool added);
void dt_ecs_manager_free(DtEcsManager* manager);
void dt_remove_tool_components(const DtEcsManager* manager);

/*=============================================================================
 *                         Определения для систем
 *============================================================================*/


/*=============================================================================
 *                              Система ECS (EcsSystem)
 *============================================================================*/

typedef struct {
    float delta_time;
    float fixed_delta_time;
} DtUpdateContext;

typedef void (*Action)(void*);
typedef void (*CtxAction)(void*, DtUpdateContext*);
typedef void (*Init)(DtEcsManager*, void*);

/**
 * @brief Контейнер для функций системы
 * NULL
 */
typedef struct {
    void* data;

    Init init;
    CtxAction update;
    Action destroy;

    i16 priority;
} UpdateSystem;

/*=============================================================================
 *                         Обработчик систем (SystemHandler)
 *============================================================================*/

/**
 * @brief Контейнер, в котором системы сгруппированы по функциям
 */
typedef struct {
    DtEcsManager* manager;
    DT_VEC(UpdateSystem*) systems;
} UpdateHandler;

UpdateHandler* dt_update_handler_new(DtEcsManager* manager, u16 updater_count);
void dt_update_handler_add(UpdateHandler* handler, UpdateSystem* system);
void dt_update_handler_init(const UpdateHandler* handler);
void dt_update_handler_update(const UpdateHandler* handler, DtUpdateContext* ctx);
void dt_update_handler_destroy(const UpdateHandler* handler);
void dt_update_handler_free(UpdateHandler* handler);

/*=============================================================================
 *                         Система отрисовки (DrawSystem)
 *============================================================================*/

typedef struct {
    void* data;
    Init init;
    Action draw;
    Action destroy;
    i16 priority;
} DrawSystem;

/*=============================================================================
 *                        Обработчик отрисовки (DrawHandler)
 *============================================================================*/

typedef struct {
    DtEcsManager* manager;
    DT_VEC(DrawSystem*) systems;
} DrawHandler;

DrawHandler* dt_draw_handler_new(DtEcsManager* manager, u16 drawers_count);
void dt_draw_handler_add(DrawHandler* handler, const DrawSystem* system);
void dt_draw_handler_init(const DrawHandler* handler);
void dt_draw_handler_draw(const DrawHandler* handler);
void dt_draw_handler_destroy(const DrawHandler* handler);
void dt_draw_handler_free(DrawHandler* handler);

#endif /* DT_ECS_H */
