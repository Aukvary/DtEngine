#ifndef TEST_HEADER_H
#define TEST_HEADER_H
#include "Ecs/RegisterHandler.h"
#include "scheduler/RuntimeScheduler.h"

#define MODULE_TEST_COMPONENT(X, name)                                                             \
    X(int, data, name,                                                                             \
      (DtAttributeData) {                                                                          \
          .attribute_name = "module_test_attr",                                                    \
          .data = &(int) {10},                                                                     \
      })
DT_DEFINE_COMPONENT(ModuleTestComponent, MODULE_TEST_COMPONENT);

void initialize(DtEnvironment* env);
void deinitialize(DtEnvironment* env);

DT_DEFINE_MODULE("TestModule", initialize, deinitialize);

typedef struct {
    UpdateSystem system;
    DtEcsFilter* filter;
} ModuleUpdate;

typedef struct {
    DrawSystem system;
    DtEcsFilter* filter;
} ModuleDraw;

#endif /*TEST_HEADER_H*/
