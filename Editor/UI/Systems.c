#include <stdbool.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include <nuklear.h>

#include "Ecs/RegisterHandler.h"
#include "GameLibLink/GameLib.h"
#include "UI.h"
#include "raylib-nuklear.h"
#include "scheduler/RuntimeScheduler.h"

extern struct nk_context* ctx;
extern const DtComponentData** components;
extern u16 components_count;
const extern DtScene* game_scene;

DtEntity selected_entity = 0;

static void draw_entity_node(DtEntity e);
static void components_tree_draw();
static void console_draw();

DT_REGISTER_UPDATE(HotKeyReloadSystem, reload_system_new)
UpdateSystem* reload_system_new() {
    UpdateSystem* system = DT_MALLOC(sizeof(UpdateSystem));

    *system = (UpdateSystem) {
        .init = NULL,
        .update = reload_system_update,
        .destroy = NULL,
    };

    return system;
}

void reload_system_update(void* data, DtUpdateContext* update_ctx) {
    if (!IsKeyPressed(KEY_C))
        return;
    printf("call reload game lib");
    reload_game_lib(true);
}

DT_REGISTER_DRAW(HierarchyTreeSystem, hierarchy_tree_new)
DrawSystem* hierarchy_tree_new() {
    DrawSystem* system = DT_MALLOC(sizeof(DrawSystem));
    *system = (DrawSystem) {
        .init = NULL,
        .draw = hierarchy_tree_draw,
        .destroy = NULL,
    };

    return system;
}

void hierarchy_tree_draw(void* data) {
    float width = (float) GetScreenWidth();
    float height = (float) GetScreenHeight();

    if (nk_begin(ctx, "Hierarchy", nk_rect(0, 0, width / 6, height - height / 4),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {

        nk_menubar_begin(ctx);
        nk_layout_row_static(ctx, 25, 50, 1);
        if (nk_menu_begin_label(ctx, "Edit", NK_TEXT_LEFT, nk_vec2(150, 200))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_menu_item_label(ctx, "Create Empty", NK_TEXT_LEFT)) { /* logic */
            }
            if (nk_menu_item_label(ctx, "Delete Selected", NK_TEXT_LEFT)) { /* logic */
            }
            nk_menu_end(ctx);
        }
        nk_menubar_end(ctx);


        nk_layout_row_dynamic(ctx, 20, 1);

        for (int i = 0; i < game_scene->manager->entities_ptr; i++) {
            if (game_scene->manager->sparse_entities[i].alive &&
                dt_ecs_manager_get_parent(game_scene->manager, i).id == DT_ENTITY_NULL) {
                draw_entity_node(i);
            }
        }
    }
    nk_end(ctx);

    components_tree_draw();
    console_draw();
}

static void draw_entity_node(DtEntity e) {
    DtEntityInfo info = dt_ecs_manager_get_entity(game_scene->manager, e);
    bool has_children = info.children_count > 0;

    char label[32];
    snprintf(label, sizeof(label), "%d", e);

    int is_selected = (selected_entity == e);
    int old_selected = is_selected;

    enum nk_collapse_states state = NK_MINIMIZED;

    if (nk_tree_element_push_id(ctx, has_children ? NK_TREE_NODE : NK_TREE_TAB, label, state,
                                &is_selected, (nk_hash) e)) {

        if (is_selected != old_selected) {
            selected_entity = e;
        }

        DtEntity count;
        const DtEntity* children = dt_ecs_manager_get_children(game_scene->manager, e, &count);
        for (int i = 0; i < count; i++) {
            draw_entity_node(children[i]);
        }

        nk_tree_pop(ctx);
    } else {
        if (is_selected != old_selected) {
            selected_entity = e;
        }
    }
}

static void components_tree_draw() {
    float width = (float) GetScreenWidth();
    float height = (float) GetScreenHeight();

    char title[30];
    sprintf(title, "components %hu", selected_entity);

    if (nk_begin(ctx, title, nk_rect(width - width / 5, 0, width / 5, height),
                 NK_WINDOW_BORDER | NK_WINDOW_SCALE_LEFT | NK_WINDOW_TITLE)) {
        nk_layout_row_static(ctx, 30, (int) width / 12, 1);

        DtEntityInfo info = dt_ecs_manager_get_entity(game_scene->manager, selected_entity);

        for (int i = 0; i < info.component_count; i++) {
            if (nk_button_label(ctx, game_scene->manager->pools_table[info.components[i]]->name)) {
            }
        }
    }
    nk_end(ctx);
}

static void console_draw() {
    float width = (float) GetScreenWidth();
    float height = (float) GetScreenHeight();

    if (nk_begin(ctx, "Terminal", nk_rect(0, height - height / 4, width - width / 5, height / 4),
                 NK_WINDOW_BORDER | NK_WINDOW_SCALE_LEFT | NK_WINDOW_TITLE)) {
        nk_layout_row_static(ctx, 30, (int) width / 12, 1);


        if (nk_button_label(ctx, "terminal")) {
        }
    }
    nk_end(ctx);
}
