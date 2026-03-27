#include "GameLib.h"
#include "scheduler/RuntimeScheduler.h"

#define GAME_LIB_PATH "./libGameLib"

#ifdef DEBUG
#define BUILD_MOD "debug_build"
#else
#define BUILD_MOD "realise_build"
#endif /*DEBUG*/

#ifdef _WIN32
#define REBUILD_SCRIPT_PATH "..\\..\\..\\"BUILD_MOD".bat GameLibShared"
#else
#define REBUILD_SCRIPT_PATH "./../../../"BUILD_MOD".sh GameLibShared"
#endif

ModuleInfo* game_lib;

const DtComponentData** components;
u16 components_count;

const DtUpdateData** updates;
u16 updates_count;

const DtDrawData** draws;
u16 draws_count;

const DtScene* game_scene = NULL;

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
}

void build_game_lib() {
    system(REBUILD_SCRIPT_PATH);
}

void reload_game_lib(bool rebuild) {
    dt_module_unload(dt_environment_instance(), game_lib);
    if (rebuild) {
        build_game_lib();
    }
    game_lib = dt_module_load(dt_environment_instance(), DT_LIB_NAME(GAME_LIB_PATH));
    init_game_data();
}