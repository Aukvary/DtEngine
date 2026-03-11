#include "TestEcs.h"
#include "scheduler/RuntimeScheduler.h"

static const ModuleInfo* info;

void test_module_load1(void);

void test_module_load(void) {
    printf("\n\t===test_component_register===\n");

    printf("\n\t\t===test 1 start===\n");
    test_module_load1();
    printf("\t\t===test 1 success===\n");

    printf("\n\t\t===SUCCESS===\n\n");
}

void test_module_load1(void) {
    info = dt_module_load(dt_environment_instance(), DT_LIB_NAME("./libModuleTest"));
}