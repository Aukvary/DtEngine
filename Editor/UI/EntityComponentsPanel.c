#include <scheduler/RuntimeScheduler.h>
#include <string.h>
#include "DtAllocators.h"
#include "EditorApi.h"
#include "UI.h"

extern DtEntity selected_entity;

static DrawSystem* component_panel_new();
static void component_panel_draw(void* _);

DT_REGISTER_DRAW(ComponentPanelDraw, component_panel_new)

static DrawSystem* component_panel_new() {
    DrawSystem* system = DT_MALLOC(sizeof(DrawSystem));
    *system = (DrawSystem) {
        .data = NULL,
        .init = NULL,
        .draw = component_panel_draw,
        .destroy = NULL,
    };

    return system;
}
static void draw_component_fields(const DtComponentData* data, DtEntity entity, DtEcsPool* pool,
                                  void* component_ptr) {
    for (int i = 0; i < data->field_count; i++) {
        bool hide = false;
        for (int j = 0; j < data->filed_attributes_count[i]; j++) {
            if (strcmp(data->filed_attributes[i][j].attribute_name, DTE_INSPECTOR_HIDE_NAME) == 0) {
                hide = true;
                break;
            }
        }

        if (hide)
            continue;

        u8* field_addr = (u8*) component_ptr + data->field_offsets[i];
        const char* field_type = data->field_types[i];
        const char* field_name = data->field_names[i];

        bool changed = dte_inspector_field_draw(field_type, field_name, field_addr);

        if (!changed)
            continue;

        for (int j = 0; j < data->filed_attributes_count[i]; j++) {
            if (strcmp(data->filed_attributes[i][j].attribute_name, DTE_ON_FIELD_CHANGE_NAME) != 0)
                continue;
            void (*callback)(DtEcsPool*, DtEntity) = data->filed_attributes[i][j].data;
            if (callback)
                callback(pool, entity);
        }
    }
}

static void draw_single_component(DtEcsManager* manager, DtEntity entity, u16 pool_idx, int id) {
    DtEcsPool* pool = manager->pools[pool_idx];
    const char* comp_name = pool->name;
    const DtComponentData* data = dt_component_get_data_by_name(comp_name);

    if (!data)
        return;

    void* component_ptr = dt_ecs_pool_get(pool, entity);
    if (!component_ptr)
        return;

    if (nk_tree_push_id(ctx, NK_TREE_TAB, comp_name, NK_MAXIMIZED, id)) {
        draw_component_fields(data, entity, pool, component_ptr);

        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_button_label(ctx, "Remove Component")) {
            dt_ecs_manager_entity_remove_component(manager, entity, comp_name);
        }
        nk_tree_pop(ctx);
    }
}

static void component_panel_draw(void* _) {
    if (selected_entity == DT_ENTITY_NULL)
        return;

    float width = (float) GetScreenWidth();
    float height = (float) GetScreenHeight();

    char title[64];
    snprintf(title, sizeof(title), "Components (ID: %u)", selected_entity);

    struct nk_rect bounds = nk_rect(width - width / 5, height / 30, width / 5, height);
    nk_flags flags = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE;

    if (nk_begin(ctx, title, bounds, flags)) {
        DtEcsManager* mgr = game_scene->manager;
        DtEntityInfo info = dt_ecs_manager_get_entity(mgr, selected_entity);

        for (int i = 0; i < info.component_count; i++) {
            draw_single_component(mgr, selected_entity, info.components[i], i);
        }
    }
    nk_end(ctx);
}
