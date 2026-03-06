#include "Collections/Collections.h"
#include "DtAllocators.h"
#include "RuntimeScheduler.h"
#include "ExecuteOrder.h"

static DtEnvironment environment;

static int get_hash(const char* name) {
    int hash = 2147483647;
    while (*name) {
        hash ^= *name++;
        hash *= 314159;
    }
    return hash;
}

__attribute__((constructor(DT_ORDER_INIT_ENVIRONMENT))) static void initialize_environment(void) {
    environment = (DtEnvironment) {
        .active_scene = NULL,
        .modules = dt_rb_tree_new(),
        .scenes = dt_rb_tree_new(),

        .get_component = dt_component_get_data_by_name,
        .get_update = dt_update_get_data_by_name,
        .get_draw = dt_draw_get_data_by_name,
    };

    environment.components = dt_component_get_all(&environment.components_count);
    environment.updaters = dt_update_get_all(&environment.updaters_count);
    environment.drawers = dt_draw_get_all(&environment.drawers_count);
}

DtEnvironment* dt_environment_instance(void) { return &environment; }

ModuleInfo* dt_module_load(DtEnvironment* env, const char* path) {
    DT_LIB_HANDLE lib = DT_LIB_LOAD(path);
    if (!lib) {
        fprintf(stderr, "[DEBUG]library hasn't loaded: %s\n", path);
        return NULL;
    }

    char* module_name = *(char**) DT_LIB_GET(lib, "dt_module_name");
    if (!module_name) {
        fprintf(stderr, "[DEBUG]library hasn't module name\n");
        return NULL;
    }

    const u64 hash = get_hash(module_name);

    ModuleInfo* module;
    if ((module = dt_rb_tree_get(&env->modules, hash))) {
        fprintf(stderr, "[DEBUG]module already loaded: %s\n", module_name);
        return module;
    }

    void (*initialize)(DtEnvironment*) =
        *(DtEnvironmentHandle*) DT_LIB_GET(lib, DT_MODULE_INITIALIZE_STR);

    void (*deinitialize)(DtEnvironment*) =
        *(DtEnvironmentHandle*) DT_LIB_GET(lib, DT_MODULE_DEINITIALIZE_STR);

    DtEnvironment* (*get_environment)(void) =
        *(DtEnvironment * (**) (void) ) DT_LIB_GET(lib, "dt_environment_instance");

    ModuleInfo* info = DT_MALLOC(sizeof(ModuleInfo));

    *info = (ModuleInfo) {
        .name = module_name,
        .initialize = initialize,
        .deinitialize = deinitialize,
        .handle = lib,
        .environment = get_environment(),
    };

    dt_rb_tree_add(&env->modules, info, hash);
    if (info->initialize)
        info->initialize(env);

    return info;
}

void dt_module_unload(DtEnvironment* env, ModuleInfo* info) {
    if (!info) {
        fprintf(stderr, "[DEBUG]module was NULL\n");
        return;
    }

    if (dt_rb_tree_get(&env->modules, get_hash(info->name)) == NULL) {
        fprintf(stderr, "[DEBUG]module hasn't loaded: %s\n", info->name);
        return;
    }

    if (info->deinitialize)
        info->deinitialize(env);

    dt_rb_tree_remove(&env->modules, get_hash(info->name));
    DT_LIB_CLOSE(info->handle);
}

void dt_add_scenes(const char* directory) {}

void dt_add_scene(const char* path) {}
