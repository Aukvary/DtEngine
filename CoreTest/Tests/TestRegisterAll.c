#include <stddef.h>
#include "Ecs/RegisterHandler.h"
#include "TestEcs.h"

DT_REGISTER_TAG(TestEmptyComponent1, {
                                         .attribute_name = "test_attr",
                                         .data = &(TestAttribute) {.test_value = 10},
                                     });
DT_REGISTER_TAG(TestEmptyComponent2);
DT_REGISTER_COMPONENT(TestDataComponent1, TEST_DATA_COMPONENT_1);
DT_REGISTER_COMPONENT(TestDataComponent2, TEST_DATA_COMPONENT_2);

static void module_update_init(DtEcsManager* manager, void* data);
static void module_update_update(void* data, DtUpdateContext* ctx);
static void module_update_destroy(void* data);

UpdateSystem* module_update_new() {
    TestUpdate1* update = DT_MALLOC(sizeof(TestUpdate1));

    *update = (TestUpdate1) {
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
    TestUpdate1* update = data;

    update->tag1_pool = DT_ECS_MANAGER_GET_POOL(manager, TestEmptyComponent1);
    update->data1_pool = DT_ECS_MANAGER_GET_POOL(manager, TestDataComponent1);

    DtEcsMask mask = dt_mask_new(manager, 2, 0);
    DT_MASK_INC(mask, TestEmptyComponent1);
    DT_MASK_INC(mask, TestDataComponent1);

    update->filter = dt_mask_end(mask);
}

static void module_update_update(void* data, DtUpdateContext* ctx) {
    TestUpdate1* update = data;

    FOREACH(DtEntity, e, &update->filter->entities.entities_iterator,
            {
                printf("simulating update\n");
            });
}

static void module_update_destroy(void* data) {}

DT_REGISTER_UPDATE(TestUpdate1, module_update_new);


static void module_draw_init(DtEcsManager* manager, void* data);
static void module_draw_draw(void* data);
static void module_draw_destroy(void* data);

DrawSystem* test_draw_system_new() {
    TestDraw1* draw = DT_MALLOC(sizeof(TestDraw1));

    *draw = (TestDraw1) {
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
    TestDraw1* draw = data;

    draw->tag2_pool = DT_ECS_MANAGER_GET_POOL(manager, TestEmptyComponent2);
    draw->data2_pool = DT_ECS_MANAGER_GET_POOL(manager, TestDataComponent2);

    DtEcsMask mask = dt_mask_new(manager, 2, 0);
    DT_MASK_INC(mask, TestEmptyComponent2);
    DT_MASK_INC(mask, TestDataComponent2);

    draw->filter = dt_mask_end(mask);
}

static void module_draw_draw(void* data) {
    TestDraw1* draw = data;

    FOREACH(DtEntity, e, &draw->filter->entities.entities_iterator,
            {
                printf("simulating draw\n");
            });
}

static void module_draw_destroy(void* data) {}

DT_REGISTER_DRAW(TestDraw1, test_draw_system_new);
