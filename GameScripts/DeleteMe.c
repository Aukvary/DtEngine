#include <stdio.h>
#include "GameScripts.h"
#include "scheduler/RuntimeScheduler.h"
#include "EditorApi.h"

DT_REGISTER_TAG(TestTag)
DT_REGISTER_COMPONENT(GameType, NULL, GAME_TYPE)

#ifdef EDITOR
DECLARE_EDITOR_FUNC_TABLE
#endif

DT_DEFINE_MODULE("game", NULL, NULL)

void dte_init() {
#ifdef EDITOR
    func_table.log("init: %s", "init");
#endif
}

void dte_deinit() {
#ifdef EDITOR
    func_table.log("deinit: %s", "deinit");
#endif
}

void on_num_change(DtEcsPool* pool, DtEntity e) {
    GameType* gt = dt_ecs_pool_get(pool, e);
#ifdef EDITOR
    func_table.log("ID: %d\tnew val: %s", e, gt->num);
#endif
}

DTE_DECLARE_EDITOR_FUNC(dte_init, dte_deinit)
