#include "scheduler/RuntimeScheduler.h"

int main(void) {
    const char* path = DT_LIB_NAME("./libGameLib");
    DtEnvironment* game = dt_environment_instance();
    void* ptr = dt_module_load(game, path);
    dt_module_load(game, path);

    dt_module_unload(game, ptr);

    dt_module_load(game, path);

    return 0;
}