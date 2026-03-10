#include <assert.h>
#include <stdio.h>
#include "TestEcs.h"

static const DtEcsManagerConfig cfg = {
    .dense_size = 1,
    .sparse_size = 1,
    .recycle_size = 0,
    .components_count = 3,
    .pools_size = 3,
    .masks_size = 1,

    .children_size = 3,

    .include_mask_count = 2,
    .exclude_mask_count = 0,
    .filters_size = 1,
};

static DtEcsManager* manager;
static DtEntity es[3];
static DtEcsPool* empty_pool;
static DtEcsPool* data_pool;

static void test_create_remove_entity_1(void);
static void test_create_remove_entity_2(void);
static void test_create_remove_entity_3(void);
static void test_create_remove_entity_4(void);
static void test_create_remove_entity_5(void);

void test_create_remove_entity(void) {
    printf("\n\t===test_create_remove_entity===\n");

    manager = dt_ecs_manager_new(cfg);

    printf("\n\t\t===test 1 start===\n");
    test_create_remove_entity_1();
    printf("\t\t===test 1 success===\n");

    // printf("\n\t\t===test 2 start===\n");
    // test_create_remove_entity_2();
    // printf("\t\t===test 2 success===\n");

    printf("\n\t\t===test 3 start===\n");
    test_create_remove_entity_3();
    printf("\t\t===test 3 success===\n");

    printf("\n\t\t===test 4 start===\n");
    test_create_remove_entity_4();
    printf("\t\t===test 4 success===\n");

    printf("\n\t\t===test 5 start===\n");
    test_create_remove_entity_5();
    printf("\t\t===test 5 success===\n");

    dt_ecs_manager_free(manager);
    printf("\n\t\t===SUCCESS===\n\n");
}
static void test_create_remove_entity_1(void) {
    for (int i = 0; i < 3; i++) {
        es[i] = dt_ecs_manager_new_entity(manager);
        assert(es[i] == i);

        DtEntityInfo info = dt_ecs_manager_get_entity(manager, es[i]);
        assert(info.id == es[i]);
    }


    for (int i = 0; i < 3; i++) {
        assert(dt_ecs_manager_get_entity(manager, es[i]).gen == 0);
        assert(dt_ecs_manager_get_entity(manager, es[i]).children_count == 0);
        assert(dt_ecs_manager_get_entity(manager, es[i]).parent == DT_ENTITY_NULL);
    }
}

static void test_create_remove_entity_2(void) {
    dt_ecs_manager_kill_entity(manager, es[1]);
    DtEntityInfo info2 = dt_ecs_manager_get_entity(manager, es[1]);

    assert(info2.id == es[1]);
    assert(info2.alive == false);

    es[1] = dt_ecs_manager_new_entity(manager);

    assert(es[1] == 1);

    info2 = dt_ecs_manager_get_entity(manager, es[1]);
    assert(info2.id == es[1]);
    assert(info2.alive == true);
    assert(info2.gen == 1);
}

static void test_create_remove_entity_3(void) {
    empty_pool = DT_ECS_MANAGER_GET_POOL(manager, TestEmptyComponent1);
    data_pool = DT_ECS_MANAGER_GET_POOL(manager, TestDataComponent1);

    dt_ecs_pool_add(empty_pool, es[0], NULL);
    dt_ecs_pool_add(data_pool, es[0], &(TestDataComponent1) {10});

    assert(dt_ecs_manager_get_entity(manager, es[0]).component_count == 2);

    dt_ecs_manager_copy_entity(manager, es[1], es[0]);

    assert(dt_ecs_pool_has(empty_pool, es[1]));
    assert(dt_ecs_pool_has(data_pool, es[1]));
    assert(((TestDataComponent1*) dt_ecs_pool_get(data_pool, es[0]))->data ==
           ((TestDataComponent1*) dt_ecs_pool_get(data_pool, es[1]))->data);
}

static void test_create_remove_entity_4(void) {
    assert(dt_ecs_manager_get_entity(manager, es[0]).component_count == 2);
    assert(((TestDataComponent1*) dt_ecs_pool_get(data_pool, es[0]))->data == 10);

    dt_ecs_manager_reset_entity(manager, es[0]);

    assert(dt_ecs_manager_get_entity(manager, es[0]).component_count == 2);
    assert(((TestDataComponent1*) dt_ecs_pool_get(data_pool, es[0]))->data == 0);
}

static void test_create_remove_entity_5(void) {
    dt_ecs_manager_clear_entity(manager, es[0]);
    assert(dt_ecs_manager_get_entity(manager, es[0]).component_count == 0);
    assert(!dt_ecs_pool_has(empty_pool, es[0]));
    assert(!dt_ecs_pool_has(data_pool, es[0]));
}
