#include "Collections/Collections.h"
#include "DtAllocators.h"
#include "LazyLoad.h"

static DtEnvironment environment;

static int get_hash(const char* name) {
    int hash = 2147483647;
    while (*name) {
        hash ^= *name++;
        hash *= 314159;
    }
    return hash;
}

__attribute__((constructor(102))) static void initialize_environment(void) {
    environment = (DtEnvironment) {
        .active_scene = NULL,
        .modules = dt_rb_tree_new(),
        .scenes = dt_rb_tree_new(),
    };

    environment.components = dt_component_get_all(&environment.components_count);
    environment.updaters = dt_update_get_all(&environment.updaters_count);
    environment.drawers = dt_draw_get_all(&environment.drawers_count);
}

DtEnvironment* dt_environment_instance(void) { return &environment; }

void dt_load_module(DtEnvironment* env, const char* path) {
    DT_LIB_HANDLE lib = DT_LIB_LOAD(path);
    if (!lib) {
        fprintf(stderr, "library hasn't module data: %s\n", path);
    }

    char* module_name = (char*) DT_LIB_GET(lib, "module_name");
    if (!module_name) {
        fprintf(stderr, "library hasn't module data: %s\n", path);
    }

    const u64 hash = get_hash(module_name);

    if (dt_rb_tree_get(&env->modules, hash)) return;

    void (*initialize)(DtEnvironment*) =
        (void (*)(DtEnvironment*)) DT_LIB_GET(lib, "module_initialize");

    void (*deinitialize)(DtEnvironment*) =
        (void (*)(DtEnvironment*)) DT_LIB_GET(lib, "module_deinitialize");

    ModuleInfo* info = DT_MALLOC(sizeof(ModuleInfo));

    *info = (ModuleInfo) {
        .name = module_name,
        .initialize = initialize,
        .deinitialize = deinitialize,
        .handle = lib
    };

    dt_rb_tree_add(&env->modules, info, hash);

    info->initialize(env);
}

void dt_unload_module(DtEnvironment* env, const char* name) {
    ModuleInfo* info;
    if ((info = dt_rb_tree_get(&env->modules, get_hash(name))) == NULL) {
        return;
    }

    info->deinitialize(env);
    dt_rb_tree_remove(&env->modules, get_hash(name));
    DT_LIB_CLOSE(info->handle);
}

void dt_add_scenes(const char* directory) {}

void dt_add_scene(const char* path) {}
