#include <stdio.h>
#include "scheduler/LazyLoad.h"

int main(void) {
    const char* path = "libGameLibShared"DT_LIB_EXTENSION;
    DtEnvironment* game = dt_environment_instance();
    dt_load_module(game, path);

    return 0;
}