#ifndef COMPONENTS_HANDLER_H
#define COMPONENTS_HANDLER_H

#include <DtNumericalTypes.h>
#include <stdio.h>
#include "DtEcs.h"
#include "ExecuteOrder.h"

/**
 * @brief metadata of component and components field
 */
typedef struct {
    char* attribute_name;
    void* data;
} DtAttributeData;

/**
 * @brief data of serialized component
 */
typedef struct DtComponentData {
    char* name;
    u16 id;
    u64 hash;

    u16 field_count;
    char** field_names;
    char** field_types;
    u16* field_offsets;
    DtAttributeData** filed_attributes;
    u16* filed_attributes_count;

    u64 component_size;

    DtAttributeData* attributes;
    u8 attribute_count;
} DtComponentData;

#define DT_FIELD_DECL(type, name, component_name, ...) type name;
#define DT_FIELD_COUNT(type, name, component_name, ...) +1
#define DT_FIELD_NAME(type, name, component_name, ...) #name,
#define DT_FIELD_OFFSET(type, name, component_name, ...) offsetof(component_name, name),
#define DT_FIELD_TYPE(type, name, component_name, ...) #type,
#define DT_FIELD_SIZE(type, name, component_name, ...) sizeof(type),
#define DT_FIELD_ATTRIBUTES_GENERATE(type, name, component_name, ...)                              \
    static DtAttributeData component_name##_##name##_attrs[] = {__VA_ARGS__};

#define DT_FIELD_ATTRIBUTES(type, name, component_name, ...) component_name##_##name##_attrs,
#define DT_FIELD_ATTRIBUTES_COUNT(type, name, component_name, ...)                                 \
    (sizeof((DtAttributeData[]) {__VA_ARGS__}) / sizeof(DtAttributeData)),

// TODO: comments
#define DT_DEFINE_COMPONENT(component_name, fields)                                                \
    typedef struct {                                                                               \
        fields(DT_FIELD_DECL, component_name)                                                      \
    } component_name;

// TODO: comments
#define DT_REGISTER_TAG(component_name, ...)                                                       \
    static DtAttributeData component_name##_attrs[] = {__VA_ARGS__};                               \
                                                                                                   \
    static DtComponentData local_##component_name##_data;                                          \
    static                                                                                         \
        __attribute__((constructor(DT_ORDER_REGISTER_COUNT))) void component_name##_add_count() {  \
        dt_component_increment_count();                                                            \
    }                                                                                              \
                                                                                                   \
    static __attribute__((                                                                         \
        constructor(DT_ORDER_REGISTER))) void component_name##_register_component(void) {          \
        local_##component_name##_data = (DtComponentData) {                                        \
            .name = #component_name,                                                               \
            .attributes = component_name##_attrs,                                                  \
            .attribute_count =                                                                     \
                (sizeof((DtAttributeData[]) {__VA_ARGS__}) / sizeof(DtAttributeData)),             \
            .field_count = 0,                                                                      \
            .field_names = NULL,                                                                   \
            .field_offsets = NULL,                                                                 \
            .component_size = 0,                                                                   \
        };                                                                                         \
        dt_register_component(&local_##component_name##_data);                                     \
    }                                                                                              \
    DtComponentData component_name##_data() { return local_##component_name##_data; }

// TODO: comments
#define DT_REGISTER_COMPONENT(component_name, fields, ...)                                         \
    static DtAttributeData component_name##_attrs[] = {__VA_ARGS__};                               \
                                                                                                   \
    static u16 component_name##_attrs_count =                                                      \
        (sizeof((DtAttributeData[]) {__VA_ARGS__}) / sizeof(DtAttributeData));                     \
                                                                                                   \
    static char* component_name##_field_names[] = {fields(DT_FIELD_NAME, component_name)};         \
    static u16 component_name##_field_offsets[] = {fields(DT_FIELD_OFFSET, component_name)};       \
    static char* component_name##_field_typess[] = {fields(DT_FIELD_TYPE, component_name)};        \
                                                                                                   \
    fields(DT_FIELD_ATTRIBUTES_GENERATE, component_name);                                          \
                                                                                                   \
    static DtAttributeData* component_name##_field_attrs[] = {                                     \
        fields(DT_FIELD_ATTRIBUTES, component_name)};                                              \
    static u16 component_name##_field_attrs_count[] = {                                            \
        fields(DT_FIELD_ATTRIBUTES_COUNT, component_name)};                                        \
                                                                                                   \
    static DtComponentData local_##component_name##_data;                                          \
                                                                                                   \
    static                                                                                         \
        __attribute__((constructor(DT_ORDER_REGISTER_COUNT))) void component_name##_add_count() {  \
        dt_component_increment_count();                                                            \
    }                                                                                              \
                                                                                                   \
    static __attribute__((                                                                         \
        constructor(DT_ORDER_REGISTER))) void component_name##_register_component(void) {          \
        local_##component_name##_data = (DtComponentData) {                                        \
            .name = #component_name,                                                               \
            .attributes = component_name##_attrs,                                                  \
            .attribute_count = component_name##_attrs_count,                                       \
            .field_count = 0 fields(DT_FIELD_COUNT, component_name),                               \
            .field_names = component_name##_field_names,                                           \
            .field_offsets = component_name##_field_offsets,                                       \
            .filed_attributes = component_name##_field_attrs,                                      \
            .filed_attributes_count = component_name##_field_attrs_count,                          \
            .field_types = component_name##_field_typess,                                          \
            .component_size = sizeof(component_name),                                              \
        };                                                                                         \
        dt_register_component(&local_##component_name##_data);                                     \
    }                                                                                              \
    DtComponentData component_name##_data() { return local_##component_name##_data; }

void dt_component_increment_count();

/**
 * @brief register component in global pool
 *
 * @param data pointer to generated data variable
 *
 * @note this func must be called by __attribute__((constructor)) func else may be
 * exceptions
 */
void dt_register_component(DtComponentData* data);

/**
 * @brief get component name by id
 *
 * @param id id of component
 *
 * @note if global pool hasn't this id return NULL
 */
const DtComponentData* dt_component_get_data_by_id(u16 id);

/**
 * @brief get component id by name
 *
 * @param name name of component
 *
 * @note if global pool hasn't this name return -1
 */
const DtComponentData* dt_component_get_data_by_name(const char* name);

i32 dt_component_get_field_index(const DtComponentData* data, const char* name);

// TODO: comments
const DtComponentData** dt_component_get_all(u16* size);

typedef struct {
    char* name;
    u16 id;

    UpdateSystem* (*new)(void);

    i16 priority;
} DtUpdateData;

// TODO: comments
#define DT_REGISTER_UPDATE(system_name, new_func)                                                  \
    static DtUpdateData local_##system_name##_data;                                                \
                                                                                                   \
    static __attribute__((constructor(DT_ORDER_REGISTER_COUNT))) void system_name##_add_count() {  \
        dt_update_increment_count();                                                               \
    }                                                                                              \
    static __attribute__((                                                                         \
        constructor(DT_ORDER_REGISTER))) void dt_##system_name##_register_update(void) {           \
        local_##system_name##_data = (DtUpdateData) {                                              \
            .name = #system_name,                                                                  \
            .new = new_func,                                                                       \
        };                                                                                         \
        dt_update_register(&local_##system_name##_data);                                           \
    }                                                                                              \
    DtUpdateData system_name##_data() { return local_##system_name##_data; }

// TODO: comments
void dt_update_register(DtUpdateData* data);
// TODO: comments
const DtUpdateData* dt_update_get_data_by_name(const char* name);
// TODO: comments
const DtUpdateData* dt_update_get_data_by_id(u16 id);
// TODO: comments
const DtUpdateData** dt_update_get_all(u16* size);
// TODO:comments
void dt_update_increment_count();

// TODO: comments
typedef struct {
    char* name;
    u16 id;

    DrawSystem* (*new)(void);

    i16 priority;
} DtDrawData;

// TODO: comments
#define DT_REGISTER_DRAW(system_name, new_func)                                                    \
    static DtDrawData local_##system_name##_data;                                                  \
    static __attribute__((constructor(DT_ORDER_REGISTER_COUNT))) void system_name##_add_count() {  \
        dt_draw_increment_count();                                                                 \
    }                                                                                              \
    static __attribute__((constructor(DT_ORDER_REGISTER))) void dt_##system_name##_register_draw(  \
        void) {                                                                                    \
        local_##system_name##_data = (DtDrawData) {                                                \
            .name = #system_name,                                                                  \
            .new = new_func,                                                                       \
        };                                                                                         \
        dt_draw_register(&local_##system_name##_data);                                             \
        dt_draw_increment_count();                                                                 \
    }                                                                                              \
    DtDrawData system_name##_data() { return local_##system_name##_data; }

// TODO: comments
void dt_draw_register(DtDrawData* data);
// TODO: comments
const DtDrawData* dt_draw_get_data_by_name(const char* name);
// TODO: comments
const DtDrawData* dt_draw_get_data_by_id(u16 id);
// TODO: comments
const DtDrawData** dt_draw_get_all(u16* size);
// TODO: comments
void dt_draw_increment_count();

#endif /*COMPONENTS_HANDLER_H*/
