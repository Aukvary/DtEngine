#include <assert.h>
#include <string.h>
#include "TestEcs.h"

static const DtEcsManagerConfig cfg = {
    .dense_size = 1,
    .sparse_size = 3,
    .recycle_size = 3,
    .components_count = 0,
    .pools_size = 1,
    .masks_size = 0,

    .children_size = 1,

    .include_mask_count = 0,
    .exclude_mask_count = 0,
    .filters_size = 0,
};

static DtEcsManager* manager;

static DtEntity e1;
static DtEntity e2;
static DtEntity e3;

static DtEcsPool* tag_pool;
static DtEcsPool* data_pool;

void test_pool_1(void);
void test_pool_2(void);
void test_pool_3(void);
void test_pool_4(void);

void test_reset(void* item, size_t size);
void test_copy(void* dst, const void* src, size_t size);

void test_pools(void) {
    printf("\n\t===test_pools===\n");
    manager = dt_ecs_manager_new(cfg);

    e1 = dt_ecs_manager_new_entity(manager);
    e2 = dt_ecs_manager_new_entity(manager);
    e3 = dt_ecs_manager_new_entity(manager);

    printf("\n\t\t===test 1 start===\n");
    test_pool_1();
    printf("\t\t===test 1 success===\n");

    printf("\n\t\t===test 2 start===\n");
    test_pool_2();
    printf("\t\t===test 2 success===\n");

    printf("\n\t\t===test 3 start===\n");
    test_pool_3();
    printf("\t\t===test 3 success===\n");

    printf("\n\t\t===test 4 start===\n");
    test_pool_4();
    printf("\t\t===test 4 success===\n");

    dt_ecs_manager_free(manager);
    printf("\n\t\t===SUCCESS===\n\n");
}

void test_pool_1(void) {
    tag_pool = DT_ECS_MANAGER_GET_POOL(manager, TestEmptyComponent1);
    assert(tag_pool->type == TAG_POOL);

    dt_ecs_pool_add(tag_pool, e1, NULL);
    dt_ecs_pool_add(tag_pool, e2, NULL);

    FOREACH(DtEntity, e, &tag_pool->iterator, { assert(e == e1 || e == e2); });

    assert(dt_ecs_pool_has(tag_pool, e1));
    assert(dt_ecs_pool_has(tag_pool, e2));
    assert(!dt_ecs_pool_has(tag_pool, e3));

    dt_ecs_pool_copy(tag_pool, e3, e1);
    assert(dt_ecs_pool_has(tag_pool, e1));
    assert(dt_ecs_pool_has(tag_pool, e3));
}

void test_pool_2(void) {
    data_pool = DT_ECS_MANAGER_GET_POOL(manager, TestDataComponent1);
    assert(data_pool->type == COMPONENT_POOL);

    dt_ecs_pool_add(data_pool, e1, &(TestDataComponent1) {10});
    dt_ecs_pool_add(data_pool, e2, NULL);
    dt_ecs_pool_add(data_pool, e3, &(TestDataComponent1) {30});

    FOREACH(DtEntity, e, &data_pool->iterator, { assert(dt_ecs_pool_has(data_pool, e)); });

    assert(((TestDataComponent1*) dt_ecs_pool_get(data_pool, e1))->data == 10);
    assert(((TestDataComponent1*) dt_ecs_pool_get(data_pool, e2))->data == 0);
    assert(((TestDataComponent1*) dt_ecs_pool_get(data_pool, e3))->data == 30);
}

void test_pool_3(void) {
    dt_ecs_pool_copy(data_pool, e2, e1);
    dt_ecs_pool_reset(data_pool, e1);

    assert(((TestDataComponent1*) dt_ecs_pool_get(data_pool, e1))->data == 0);

    assert(((TestDataComponent1*) dt_ecs_pool_get(data_pool, e2))->data == 10);
}

void test_pool_4(void) {
    DtEcsPool* pool = DT_COMPONENT_POOL_NEW(TestDataComponent2, manager, test_reset, test_copy);
    dt_ecs_manager_add_pool(manager, pool);

    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestDataComponent2, e1, NULL);
    DT_ECS_MANAGER_ADD_TO_POOL(manager, TestDataComponent2, e2, &(TestDataComponent2) {"void"});

    assert(strcmp(((TestDataComponent2*)dt_ecs_pool_get(pool, e1))->data, "test reset") == 0);
    assert(strcmp(((TestDataComponent2*)dt_ecs_pool_get(pool, e2))->data, "test copy") == 0);
}

void test_reset(void* item, size_t size) { ((TestDataComponent2*) item)->data = "test reset"; }

void test_copy(void* dst, const void* src, size_t size) {
    ((TestDataComponent2*) dst)->data = "test copy";
}
