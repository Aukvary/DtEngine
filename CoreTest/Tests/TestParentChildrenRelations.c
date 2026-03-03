#include <assert.h>
#include "TestEcs.h"

static const DtEcsManagerConfig cfg = {
    .dense_size = 3,
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

static void test_parent_children_relations_1(void);
static void test_parent_children_relations_2(void);
static void test_parent_children_relations_3(void);

void test_parent_children_relations(void) {
    printf("\n\t===test_parent_children_relations===\n");

    manager = dt_ecs_manager_new(cfg);

    printf("\n\t\t===test 1 start===\n");
    test_parent_children_relations_1();
    printf("\t\t===test 1 success===\n");

    printf("\n\t\t===test 2 start===\n");
    test_parent_children_relations_2();
    printf("\t\t===test 2 success===\n");

    printf("\n\t\t===test 3 start===\n");
    test_parent_children_relations_3();
    printf("\t\t===test 3 success===\n");

    dt_ecs_manager_free(manager);
    printf("\n\t\t===SUCCESS===\n\n");
}

static void test_parent_children_relations_1(void) {
    e1 = dt_ecs_manager_new_entity(manager);
    e2 = dt_ecs_manager_new_entity(manager);
    e3 = dt_ecs_manager_new_entity(manager);

    assert(dt_ecs_manager_get_entity(manager, e1).children_count == 0);
    assert(dt_ecs_manager_get_parent(manager, e2).id == DT_ENTITY_NULL);
    assert(dt_ecs_manager_get_parent(manager, e3).id == DT_ENTITY_NULL);

    dt_ecs_manager_set_parent(manager, e2, e1);
    dt_ecs_manager_set_parent(manager, e3, e1);

    assert(dt_ecs_manager_get_entity(manager, e1).children_count == 2);

    DtIterator iter = dt_ecs_manager_get_entity(manager, e1).children_iterator;

    FOREACH(DtEntity, e, &iter, {
        assert(e == e2 || e == e3);
    });

    assert(dt_ecs_manager_get_parent(manager, e2).id == e1);
    assert(dt_ecs_manager_get_parent(manager, e2).id == e1);
}

static void test_parent_children_relations_2(void) {
    dt_ecs_manager_add_child(manager, e2, e1);

    DtIterator iter = dt_ecs_manager_get_entity(manager, e1).children_iterator;

    FOREACH(DtEntity, e, &iter, {
        assert(e == e3);
    });

    iter = dt_ecs_manager_get_entity(manager, e2).children_iterator;

    FOREACH(DtEntity, e, &iter, {
        assert(e == e1);
    });

    assert(dt_ecs_manager_get_parent(manager, e2).id == DT_ENTITY_NULL);
    assert(dt_ecs_manager_get_parent(manager, e1).id == e2);
}

static void test_parent_children_relations_3(void) {
    dt_ecs_manager_kill_entity(manager, e1);

    assert(dt_ecs_manager_get_entity(manager, e3).parent == DT_ENTITY_NULL);
    assert(dt_ecs_manager_get_entity(manager, e2).children_count == 0);
}