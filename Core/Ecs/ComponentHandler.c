#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DtAllocators.h"
#include "RegisterHandler.h"
#include "scheduler/RuntimeScheduler.h"

static u64 size = 0;
static int id_counter = 0;
static const DtComponentData** component_data_by_id = NULL;
static const DtComponentData** component_data_by_name = NULL;

void dt_component_increment_count() { size++; }

static int dt_component_get_hash(const char* name) {
    int hash = 2147483647;
    while (*name) {
        hash ^= *name++;
        hash *= 314159;
    }
    return hash;
}

void dt_register_component(DtComponentData* data) {
    if (!size)
        size = 107ULL;
    if (!component_data_by_id) {
        component_data_by_id = DT_CALLOC(size, sizeof(DtComponentData*));
        component_data_by_name = DT_CALLOC(size, sizeof(DtComponentData*));
    }

    data->id = id_counter++;
    data->hash = dt_component_get_hash(data->name);
    u64 idx = data->hash % size;
    const u64 start = idx;
    while (component_data_by_name[idx] != NULL) {
        if (strcmp(data->name, component_data_by_name[idx]->name) == 0)
            return;

        idx = (idx + 1) % size;
        if (idx == start) {
            printf("[DEBUG]component count out of range");
            exit(1);
        }
    }

    component_data_by_name[idx] = data;
    component_data_by_id[data->id] = data;
    printf("[DEBUD]%s component was registered with id %d\n", data->name, data->id);
}

const DtComponentData* dt_component_get_data_by_id(const u16 id) {
    if (id >= size) {
        return NULL;
    }

    return component_data_by_id[id];
}

const DtComponentData* dt_component_get_data_by_name(const char* name) {
    u64 idx = dt_component_get_hash(name) % size;
    const u64 start = idx;

    while (component_data_by_name[idx] != NULL &&
           strcmp(component_data_by_name[idx]->name, name) != 0) {
        idx = (idx + 1) % size;
        if (idx == start) {
            DtEnvironment* env = dt_environment_instance();

            FOREACH(ModuleInfo*, info, &env->modules.iterator, {
                const DtComponentData* data = info->environment->get_component(name);
                if (data)
                    return data;
            });

            return NULL;
        }
    }


    if (component_data_by_name[idx] != NULL) {
        return component_data_by_name[idx];
    }

    DtEnvironment* env = dt_environment_instance();

    FOREACH(ModuleInfo*, info, &env->modules.iterator, {
        const DtComponentData* data = info->environment->get_component(name);
        if (data)
            return data;
    });

    return NULL;
}

i32 dt_component_get_field_index(const DtComponentData* data, const char* name) {
    for (u16 i = 0; i < data->field_count; i++) {
        if (strcmp(data->field_names[i], name) != 0)
            continue;
        return i;
    }
    return -1;
}

const DtComponentData** dt_component_get_all(u16* size) {
    *size = id_counter;
    return component_data_by_id;
}
