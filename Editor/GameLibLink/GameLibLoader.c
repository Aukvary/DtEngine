#include <math.h>
#include <string.h>


#include "DtAllocators.h"
#include "EditorApi.h"
#include "GameLib.h"
#include "scheduler/RuntimeScheduler.h"

#define GAME_LIB_PATH "./libGameLib"

#ifdef DEBUG
#define BUILD_MOD "debug_build"
#else
#define BUILD_MOD "realise_build"
#endif /*DEBUG*/

#ifdef _WIN32
#define REBUILD_SCRIPT_PATH "..\\..\\..\\" BUILD_MOD ".bat GameLibShared"
#else
#define REBUILD_SCRIPT_PATH "./../../../" BUILD_MOD ".sh GameLibShared"
#endif /*_WIN32*/

#define GAME_SCENE_PATH "./game_scene.dt.scene"

ModuleInfo* game_lib;

const DtComponentData** components;
u16 components_count;

const DtUpdateData** updates;
u16 updates_count;

const DtDrawData** draws;
u16 draws_count;

char* json_scene = NULL;

const DtScene* game_scene = NULL;

extern DtEFuncTable func_table;

static cJSON* dt_scene_serialize_ecs_manager(const DtEcsManager* manager);
static cJSON* dt_scene_serialize_entities(const DtScene* scene);

static void init_game_data() {
    components = game_lib->environment->components;
    components_count = game_lib->environment->components_count;

    updates = game_lib->environment->updaters;
    updates_count = game_lib->environment->updaters_count;

    draws = game_lib->environment->drawers;
    draws_count = game_lib->environment->drawers_count;
}

void load_game_lib() {
    game_lib = dt_module_load(dt_environment_instance(), DT_LIB_NAME(GAME_LIB_PATH));
    init_game_data();
    DtEFuncTable* game_lib_func_table = (DtEFuncTable*) DT_LIB_GET(game_lib->handle, "func_table");
    void (*init)(void) = *(void (**)())(DtEFuncTable*) DT_LIB_GET(game_lib->handle, DTE_INIT_STR);
    if (game_lib_func_table) {
        *game_lib_func_table = func_table;
    }

    if (init) init();
}

void build_game_lib() { system(REBUILD_SCRIPT_PATH); }

void reload_game_lib(bool rebuild) {
    void (*deinit)(void) =
        *(void (**)())(DtEFuncTable*) DT_LIB_GET(game_lib->handle, DTE_DEINIT_STR);
    if (deinit) {
        deinit();
    }

    dt_module_unload(dt_environment_instance(), game_lib);
    if (rebuild) {
        build_game_lib();
    }
    game_lib = dt_module_load(dt_environment_instance(), DT_LIB_NAME(GAME_LIB_PATH));
    init_game_data();
    DtEFuncTable* game_lib_func_table = (DtEFuncTable*) DT_LIB_GET(game_lib->handle, "func_table");
    void (*init)(void) = *(void (**)())(DtEFuncTable*) DT_LIB_GET(game_lib->handle, DTE_INIT_STR);
    if (game_lib_func_table) {
        *game_lib_func_table = func_table;
    }

    if (init) init();
}

void save_game_scene() {
    cJSON* root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "manager_config",
                          dt_scene_serialize_ecs_manager(game_scene->manager));

    // TODO: add scene -> json parser
    //  cJSON* update_sys = cJSON_AddArrayToObject(root, "update_systems");
    //  for(int i = 0; i < dt_vec_count(scene->update_handler->systems); i++)
    //      cJSON_AddItemToArray(update_sys,
    //      cJSON_CreateString(scene->update_handler->systems[i]->));
    //  cJSON* draw_sys = cJSON_AddArrayToObject(root, "draw_systems");
    //  for(int i = 0; i < dt_vec_count(scene->draw_handler->systems); i++)
    //      cJSON_AddItemToArray(draw_sys,
    //      cJSON_CreateString(scene->draw_handler->systems[i]->name));

    cJSON_AddItemToObject(root, "entities", dt_scene_serialize_entities(game_scene));

    int len;
    if (json_scene) {
        DT_FREE(json_scene);
        len = (int) strlen(json_scene);
    } else {
        len = 256 * 256;
    }
    json_scene = cJSON_PrintBuffered(root, len, true);
    FILE* f = fopen(GAME_SCENE_PATH, "w");
    if (f) {
        fputs(json_scene, f);
        fflush(f);
        fclose(f);
    }

    cJSON_Delete(root);
}

void load_game_scene() {
    FILE* file = fopen(GAME_SCENE_PATH, "rb");

    if (file == NULL) {
        fprintf(stderr, "[ERROR] Could not open file %s\n", GAME_SCENE_PATH);
        return;
    }

    fseek(file, 0, SEEK_END);

    i32 size = (i32) ftell(file);
    fseek(file, 0, SEEK_SET);

    json_scene = DT_MALLOC(size + 1);
    fread(json_scene, 1, size, file);

    game_scene = dt_add_scene_from_json(json_scene, "game_scene");

    fclose(file);
}

void reload_game_scene() {
    dt_scene_unload_by(game_scene);
    game_scene = dt_add_scene_from_json(json_scene, game_scene->name);
}

static cJSON* dt_scene_serialize_ecs_manager(const DtEcsManager* manager) {
    cJSON* json_cfg = cJSON_CreateObject();

    cJSON_AddNumberToObject(json_cfg, "dense_size", manager->cfg_dense_size);
    cJSON_AddNumberToObject(json_cfg, "sparse_size", manager->sparse_size);
    cJSON_AddNumberToObject(json_cfg, "recycle_size", manager->cfg_recycle_size);
    cJSON_AddNumberToObject(json_cfg, "children_size", manager->children_size);
    cJSON_AddNumberToObject(json_cfg, "components_count", manager->component_count);
    cJSON_AddNumberToObject(json_cfg, "pools_size", (double) manager->pools_table_size);
    cJSON_AddNumberToObject(json_cfg, "include_mask_count", (double) manager->include_mask_count);
    cJSON_AddNumberToObject(json_cfg, "exclude_mask_count", (double) manager->exclude_mask_count);
    cJSON_AddNumberToObject(json_cfg, "filters_size", (double) manager->filters_size);
    cJSON_AddNumberToObject(json_cfg, "masks_size", (double) manager->include_mask_count);

    return json_cfg;
}

static cJSON* dt_scene_serialize_entities(const DtScene* scene) {
    cJSON* entities_obj = cJSON_CreateObject();
    DtEcsManager* manager = scene->manager;

    for (u16 i = 0; i < manager->entities_ptr; i++) {
        cJSON* entity_json = cJSON_CreateObject();
        DtEntityInfo info = dt_ecs_manager_get_entity(manager, i);

        cJSON* components_arr = cJSON_AddArrayToObject(entity_json, "components");

        for (int c = 0; c < info.component_count; c++) {
            u16 pool_idx = info.components[c];
            DtEcsPool* pool = manager->pools[pool_idx];
            const DtComponentData* data = dt_component_get_data_by_name(pool->name);

            if (data->field_count == 0) {
                cJSON_AddItemToArray(components_arr, cJSON_CreateString(pool->name));
            } else {
                cJSON* comp_obj = cJSON_CreateObject();
                cJSON_AddStringToObject(comp_obj, "name", pool->name);
                cJSON* values_obj = cJSON_AddObjectToObject(comp_obj, "values");

                u8* instance = dt_ecs_pool_get(pool, i);
                for (int f = 0; f < data->field_count; f++) {
                    cJSON* value = dt_serialize_type_to_json(data->field_types[f],
                                                             instance + data->field_offsets[f]);
                    cJSON_AddItemToObject(values_obj, data->field_names[f], value);
                }
                cJSON_AddItemToArray(components_arr, comp_obj);
            }
        }

        if (info.parent != DT_ENTITY_NULL) {
            cJSON_AddNumberToObject(entity_json, "parent", info.parent);
        }

        char idx_str[16];
        snprintf(idx_str, sizeof(idx_str), "%u", i);
        cJSON_AddItemToObject(entities_obj, idx_str, entity_json);
    }
    return entities_obj;
}
