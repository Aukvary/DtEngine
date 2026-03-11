#include "assert.h"
#include "TestHeader.h"
#include "DtAllocators.h"

DT_REGISTER_COMPONENT(ModuleTestComponent, MODULE_TEST_COMPONENT,
                      (DtAttributeData) {
                          .attribute_name = "TestComponent",
                          .data = &(char*) {"test"},
                      });

void initialize(DtEnvironment* env) {
    assert(env->get_component("TestDataComponent1") != NULL);
}

void deinitialize(DtEnvironment* env) {
    assert(env->get_component("TestDataComponent2") != NULL);
}


static void module_update_init(DtEcsManager* manager, void* data);
static void module_update_update(void* data, DtUpdateContext* ctx);
static void module_update_destroy(void* data);

UpdateSystem* module_update_new() {
    ModuleUpdate* update = DT_MALLOC(sizeof(ModuleUpdate));

    *update = (ModuleUpdate) {
        .system =
            (UpdateSystem) {
                .init = module_update_init,
                .update = module_update_update,
                .destroy = module_update_destroy,
                .priority = 0,
                .data = update,
            },
    };

    update->system.data = update;

    return &update->system;
}

static void module_update_init(DtEcsManager* manager, void* data) {
    assert(data != NULL);
}

static void module_update_update(void* data, DtUpdateContext* ctx) {
    assert(data != NULL);
}

static void module_update_destroy(void* data) {
    assert(data != NULL);
}

DT_REGISTER_UPDATE(TestUpdate1, module_update_new);

static void module_draw_init(DtEcsManager* manager, void* data);
static void module_draw_draw(void* data);
static void module_draw_destroy(void* data);

DrawSystem* test_draw_system_new() {
    ModuleDraw* draw = DT_MALLOC(sizeof(ModuleDraw));

    *draw = (ModuleDraw) {
        .system =
            (DrawSystem) {
                .init = module_draw_init,
                .draw = module_draw_draw,
                .destroy = module_draw_destroy,
                .priority = 0,
                .data = draw,
            },
    };

    draw->system.data = draw;

    return &draw->system;
}

static void module_draw_init(DtEcsManager* manager, void* data) {
    assert(data != NULL);
}

static void module_draw_draw(void* data) {
    assert(data != NULL);
}

static void module_draw_destroy(void* data) {
    assert(data != NULL);
}

DT_REGISTER_DRAW(TestDraw1, test_draw_system_new);