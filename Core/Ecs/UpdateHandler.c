#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DtAllocators.h"
#include "RegisterHandler.h"
#include "scheduler/RuntimeScheduler.h"

static u64 size = 0;
static int id_counter = 0;
static const DtUpdateData** update_data_by_id = NULL;
static const DtUpdateData** update_data_by_name = NULL;

void dt_update_increment_count() { size++; }

static int dt_update_get_hash(const char* name) {
    int hash = 2147483647;
    while (*name) {
        hash ^= *name++;
        hash *= 314159;
    }
    return hash;
}

void dt_update_register(DtUpdateData* data) {
    if (!size)
        size = 107ULL;
    if (!update_data_by_id) {
        update_data_by_id = DT_CALLOC(size, sizeof(DtUpdateData*));
        update_data_by_name = DT_CALLOC(size, sizeof(DtUpdateData*));
    }

    data->id = id_counter++;
    update_data_by_id[data->id] = data;

    u64 idx = dt_update_get_hash(data->name) % size;
    const u64 start = idx;
    while (update_data_by_name[idx] != NULL) {
        if (strcmp(data->name, update_data_by_name[idx]->name) == 0)
            return;

        idx = (idx + 1) % size;
        if (idx == start) {
            printf("[DEBUG]update count out of range");
            exit(1);
        }
    }

    update_data_by_name[idx] = data;
    printf("[DEBUG]%s update system was registered with id %d\n", data->name, data->id);
}

const DtUpdateData* dt_update_get_data_by_id(const u16 id) {
    if (id >= size) {
        return NULL;
    }

    return update_data_by_id[id];
}

const DtUpdateData* dt_update_get_data_by_name(const char* name) {
    u64 idx = dt_update_get_hash(name) % size;
    const u64 start = idx;

    while (update_data_by_name[idx] != NULL && strcmp(update_data_by_name[idx]->name, name) != 0) {
        idx = (idx + 1) % size;
        if (idx == start) {
            DtEnvironment* env = dt_environment_instance();
            FOREACH(ModuleInfo*, info, &env->modules.iterator, {
                const DtUpdateData* data = info->environment->get_update(name);
                if (data)
                    return data;
            });

            return NULL;
        }
    }

    if (update_data_by_name[idx] != NULL) {
        return update_data_by_name[idx];
    }

    DtEnvironment* env = dt_environment_instance();

    FOREACH(ModuleInfo*, info, &env->modules.iterator, {
        const DtUpdateData* data = info->environment->get_update(name);
        if (data)
            return data;
    });

    return NULL;
}

const DtUpdateData** dt_update_get_all(u16* size_out) {
    *size_out = id_counter;
    return update_data_by_id;
}
