#include <assert.h>
#include <string.h>

#include "TestEcs.h"

static const DtUpdateData** updaters;
static u16 updaters_count;

static const DtDrawData** drawers;
static u16 drawers_count;

DtEcsManager* manager;
static const DtEcsManagerConfig cfg = {
    .dense_size = 3,
    .sparse_size = 3,
    .recycle_size = 3,
    .components_count = 10,
    .pools_size = 10,
    .masks_size = 10,

    .children_size = 1,

    .include_mask_count = 10,
    .exclude_mask_count = 10,
    .filters_size = 10,
};

UpdateHandler* update_handler;
DrawHandler* draw_handler;

DtUpdateData TestUpdate1_data();
DtDrawData TestDraw1_data();

static void test_systems_register_1(void);
static void test_systems_register_2(void);

void test_systems_register(void) {
    printf("\n\t===test_systems_register===\n");

    manager = dt_ecs_manager_new(cfg);
    updaters = dt_update_get_all(&updaters_count);
    drawers = dt_draw_get_all(&drawers_count);
    update_handler = dt_update_handler_new(manager, updaters_count);
    draw_handler = dt_draw_handler_new(manager, drawers_count);

    DtEntity e1 = dt_ecs_manager_new_entity(manager);
    DtEntity e2 = dt_ecs_manager_new_entity(manager);
    DtEntity e3 = dt_ecs_manager_new_entity(manager);

    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestEmptyComponent1, e1, NULL);
    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestDataComponent1, e1, NULL);

    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestEmptyComponent1, e2, NULL);
    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestDataComponent1, e2, NULL);
    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestEmptyComponent2, e2, NULL);
    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestDataComponent2, e2, NULL);

    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestEmptyComponent2, e3, NULL);
    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestDataComponent2, e3, NULL);

    printf("\n\t\t===test 1 start===\n");
    test_systems_register_1();
    printf("\t\t===test 1 success===\n");

    printf("\n\t\t===test 2 start===\n");
    test_systems_register_2();
    printf("\t\t===test 2 success===\n");

    dt_update_handler_free(update_handler);
    dt_draw_handler_free(draw_handler);
    dt_ecs_manager_free(manager);

    printf("\n\t\t===SUCCESS===\n\n");
}

static void test_systems_register_1(void) {
    assert(strcmp(updaters[0]->name, TestUpdate1_data().name) == 0);
    assert(updaters[0]->new == TestUpdate1_data().new && updaters[0]->new != NULL);

    UpdateSystem* system = updaters[0]->new();
    dt_update_handler_add(update_handler, system);
    dt_update_handler_init(update_handler);
    dt_update_handler_update(update_handler, NULL);
    dt_update_handler_destroy(update_handler);
}

static void test_systems_register_2(void) {
    assert(strcmp(drawers[0]->name, TestDraw1_data().name) == 0);
    assert(drawers[0]->new == TestDraw1_data().new && drawers[0]->new != NULL);

    DrawSystem* system = drawers[0]->new();
    dt_draw_handler_add(draw_handler, system);
    dt_draw_handler_init(draw_handler);
    dt_draw_handler_draw(draw_handler);
    dt_draw_handler_destroy(draw_handler);

}