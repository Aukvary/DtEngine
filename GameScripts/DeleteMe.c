#include <stdio.h>
#include "scheduler/LazyLoad.h"
void initialize_func(DtEnvironment* game) {
    printf("init\n");
}

void deinitialize_func(DtEnvironment* game) {
    printf("deinit\n");
}

DT_DEFINE_MODULE("game", initialize_func, deinitialize_func)

void test() {
#ifdef  EDITOR
    printf("test\n");
#else
    printf("test\n");
#endif
}