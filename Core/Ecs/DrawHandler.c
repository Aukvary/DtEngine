#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DtAllocators.h"
#include "RegisterHandler.h"
#include "scheduler/RuntimeScheduler.h"

static u64 size = 0;
static int id_counter = 0;
static const DtDrawData** draw_data_by_id = NULL;
static const DtDrawData** draw_data_by_name = NULL;

void dt_draw_increment_count() { size++; }

static int dt_draw_get_hash(const char* name) {
    int hash = 2147483647;
    while (*name) {
        hash ^= *name++;
        hash *= 314159;
    }
    return hash;
}

void dt_draw_register(DtDrawData* data) {
    if (!size)
        size = 107ULL;
    if (!draw_data_by_id) {
        draw_data_by_id = DT_CALLOC(size, sizeof(DtDrawData*));
        draw_data_by_name = DT_CALLOC(size, sizeof(DtDrawData*));
    }

    data->id = id_counter++;
    draw_data_by_id[data->id] = data;

    u64 idx = dt_draw_get_hash(data->name) % size;
    const u64 start = idx;
    while (draw_data_by_name[idx] != NULL) {
        if (strcmp(data->name, draw_data_by_name[idx]->name) == 0)
            return;

        idx = (idx + 1) % size;
        if (idx == start) {
            printf("[DEBUG]draw count out of range");
            exit(1);
        }
    }

    draw_data_by_name[idx] = data;
    printf("[DEBUG]%s draw system was registered with id %d\n", data->name, data->id);
}

const DtDrawData* dt_draw_get_data_by_id(const u16 id) {
    if (id >= size) {
        return NULL;
    }

    return draw_data_by_id[id];
}

const DtDrawData* dt_draw_get_data_by_name(const char* name) {
    u64 idx = dt_draw_get_hash(name) % size;
    const u64 start = idx;

    while (draw_data_by_name[idx] != NULL && strcmp(draw_data_by_name[idx]->name, name) != 0) {
        idx = (idx + 1) % size;
        if (idx == start) {
            FOREACH(ModuleInfo*, info, &env->modules.iterator, {
                const DtDrawData* data = info->environment->get_draw(name);
                if (data)
                    return data;
            });

            return NULL;
        }
    }

    if (draw_data_by_name[idx] != NULL) {
        return draw_data_by_name[idx];
    }

    DtEnvironment* env = dt_environment_instance();

    FOREACH(ModuleInfo*, info, &env->modules.iterator, {
        const DtDrawData* data = info->environment->get_draw(name);
        if (data)
            return data;
    });

    return NULL;
}

const DtDrawData** dt_draw_get_all(u16* size_out) {
    *size_out = id_counter;
    return draw_data_by_id;
}
