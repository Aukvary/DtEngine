#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Collections/Collections.h"
#include "DtAllocators.h"
#include "scheduler/RuntimeScheduler.h"
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#endif

#define SCENE_EXTENSION ".dt.scene"

static DtScene* dt_scene_parse(const char* path, DtEnvironment* env);
static void dt_scene_parse_ecs_manager(cJSON* json_cfg, DtScene* scene);
static void dt_scene_parse_update_systems(cJSON* systems, DtScene* scene);
static void dt_scene_parse_draw_systems(cJSON* systems, DtScene* scene);
static void dt_scene_parse_entities(cJSON* entities, DtScene* scene);

static u64 get_scene_hash(const char* name) {
    int hash = 2147483647;
    while (*name) {
        hash ^= *name++;
        hash *= 314159;
    }
    return hash;
}

static int is_scene(const char* filename) {
    const size_t len_filename = strlen(filename);
    const size_t len_ext = strlen(SCENE_EXTENSION);

    if (len_filename < len_ext)
        return 0;
    return strcmp(filename + len_filename - len_ext, SCENE_EXTENSION) == 0;
}

void dt_add_scene(const char* path) {
    DtEnvironment* env = dt_environment_instance();
    if (!is_scene(path)) {
        fprintf(stderr, "[DEBUG]File is not a scene or doesn't exist: %s\n", path);
        return;
    }

    char* file_name = strrchr(path, '/');
#if defined(_WIN32) || defined(_WIN64)
    char* win_file_name = strrchr(path, '\\');
    if (win_file_name != NULL)
        file_name = win_file_name;
#endif

    char* ext = strchr(path, '.');

    size_t len = strlen(file_name) - strlen(ext);

    DtScene* scene = dt_scene_parse(path, env);
    if (!scene) {
        fprintf(stderr, "[DEBUG]Failed to parse scene: %s\n", path);
        return;
    }

    char* name = DT_MALLOC(len + 1);
    strncpy(name, file_name, len);

    scene->name = name;

    dt_rb_tree_add(&env->scenes, scene, get_scene_hash(path));
}

static DtScene* dt_scene_parse(const char* path, DtEnvironment* env) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "[ERROR] Could not open file %s\n", path);
    }

    fseek(file, 0, SEEK_END);

    i32 size = (i32) ftell(file);
    fseek(file, 0, SEEK_SET);

    char* scene_info = DT_STACK_ALLOC(size + 1);
    fgets(scene_info, size, file);

    cJSON* root = cJSON_Parse(scene_info);
    if (root == NULL) {
        // TODO: add handle
    }

    cJSON* manager_config = cJSON_GetObjectItem(root, "manager_config");
    cJSON* update_systems = cJSON_GetObjectItem(root, "draw_systems");
    cJSON* draw_systems = cJSON_GetObjectItem(root, "update_systems");
    cJSON* entities = cJSON_GetObjectItem(root, "");

    if (!manager_config || !update_systems || !draw_systems || !entities) {
        // TODO: add handle
    }

    DtScene* scene = DT_MALLOC(sizeof(DtScene));

    if (scene == NULL) {
        // TODO: add handle
        return NULL;
    }

    *scene = (DtScene) {
        .environment = env,
    };

    dt_scene_parse_ecs_manager(manager_config, scene);
    dt_scene_parse_update_systems(update_systems, scene);
    dt_scene_parse_draw_systems(draw_systems, scene);
    dt_scene_parse_entities(entities, scene);


    fclose(file);

    return scene;
}

static void dt_scene_parse_ecs_manager(cJSON* json_cfg, DtScene* scene) {
    DtEcsManagerConfig cfg;
    cJSON* dense_size = cJSON_GetObjectItem(json_cfg, "dense_size");
    if (!dense_size) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"dense_size\" data");
        cfg.dense_size = 0;
    } else {
        cfg.dense_size = dense_size->valueint;
    }

    cJSON* sparse_size = cJSON_GetObjectItem(json_cfg, "sparse_size");
    if (!sparse_size) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"sparse_size\" data");
        cfg.sparse_size = 0;
    } else {
        cfg.sparse_size = sparse_size->valueint;
    }

    cJSON* recycle_size = cJSON_GetObjectItem(json_cfg, "recycle_size");
    if (!recycle_size) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"recycle_size\" data");
        cfg.recycle_size = 0;
    } else {
        cfg.recycle_size = recycle_size->valueint;
    }

    cJSON* children_size = cJSON_GetObjectItem(json_cfg, "children_size");
    if (!children_size) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"children_size\" data");
        cfg.children_size = 0;
    } else {
        cfg.children_size = children_size->valueint;
    }

    cJSON* component_count = cJSON_GetObjectItem(json_cfg, "component_count");
    if (!component_count) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"component_count\" data");
        cfg.components_count = 0;
    } else {
        cfg.components_count = component_count->valueint;
    }

    cJSON* pools_size = cJSON_GetObjectItem(json_cfg, "pools_size");
    if (!pools_size) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"pools_size\" data");
        cfg.pools_size = 0;
    } else {
        cfg.pools_size = pools_size->valueint;
    }

    cJSON* include_mask_count = cJSON_GetObjectItem(json_cfg, "include_mask_count");
    if (!include_mask_count) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"include_mask_count\" data");
        cfg.include_mask_count = 0;
    } else {
        cfg.include_mask_count = include_mask_count->valueint;
    }

    cJSON* exclude_mask_count = cJSON_GetObjectItem(json_cfg, "exclude_mask_count");
    if (!exclude_mask_count) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"exclude_mask_count\" data");
        cfg.exclude_mask_count = 0;
    } else {
        cfg.exclude_mask_count = exclude_mask_count->valueint;
    }

    cJSON* filters_size = cJSON_GetObjectItem(json_cfg, "filters_size");
    if (!filters_size) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"filters_size\" data");
        cfg.filters_size = 0;
    } else {
        cfg.filters_size = filters_size->valueint;
    }

    cJSON* masks_size = cJSON_GetObjectItem(json_cfg, "masks_size");
    if (!masks_size) {
        fprintf(stderr, "[DEBUG]Scene hasn't \"masks_size\" data");
        cfg.masks_size = 0;
    } else {
        cfg.masks_size = masks_size->valueint;
    }

    scene->manager = dt_ecs_manager_new(cfg);
}

static void dt_scene_parse_update_systems(cJSON* systems, DtScene* scene) {
    u16 count = cJSON_GetArraySize(systems);

    scene->update_handler = dt_update_handler_new(scene->manager, count);
    cJSON* system = NULL;
    cJSON_ArrayForEach(system, systems) {
        const DtUpdateData* data = NULL;
        const char* name = cJSON_GetStringValue(system);
        if ((data = dt_update_get_data_by_name(name))) {
            dt_update_handler_add(scene->update_handler, data->new());
        }
    }
}

static void dt_scene_parse_draw_systems(cJSON* systems, DtScene* scene) {
    u16 count = cJSON_GetArraySize(systems);

    scene->draw_handler = dt_draw_handler_new(scene->manager, count);
    cJSON* system = NULL;
    cJSON_ArrayForEach(system, systems) {
        const DtDrawData* data = NULL;
        const char* name = cJSON_GetStringValue(system);
        if ((data = dt_draw_get_data_by_name(name))) {
            dt_draw_handler_add(scene->draw_handler, data->new());
        }
    }
}

static void dt_scene_parse_entities(cJSON* entities, DtScene* scene) {
    const u16 count = cJSON_GetArraySize(entities);
    scene->entities = DT_CALLOC(count, sizeof(DtRawEntity));
    const cJSON* json_entity = NULL;
    cJSON_ArrayForEach(json_entity, entities) {
        const cJSON* components = cJSON_GetObjectItem(json_entity, "components");

        const DtEntity entity = dt_ecs_manager_new_entity(scene->manager);
        const cJSON* component = NULL;
        cJSON_ArrayForEach(component, components) {
            if (cJSON_IsString(component)) {
                dt_ecs_manager_entity_add_component(scene->manager, entity,
                                                            cJSON_GetStringValue(component), NULL);
            } else {
                const char* name = cJSON_GetStringValue(cJSON_GetObjectItem(component, "name"));
                const cJSON* values = cJSON_GetObjectItem(component, "values");
                const DtComponentData* data = dt_component_get_data_by_name(name);
                void* instance = DT_STACK_ALLOC(data->component_size);

                cJSON* value = NULL;
                cJSON_ArrayForEach(value, values) {
                    const char* field_name = value->string;
                    const i32 i = dt_component_get_field_index(data, field_name);
                    if (i == -1)
                        continue;

                    u16 offset = data->field_offsets[i];
                    const char* type = data->field_types[i];

                    dt_parse_type(type, value, (u8*) instance + offset);
                    dt_ecs_manager_entity_add_component(
                        scene->manager, entity, cJSON_GetStringValue(component), instance);
                }
            }
        }
    }
}
