#include "Ecs/DtEcs.h"
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "GameLibLink/GameLib.h"
#include "raylib-nuklear.h"
#include "scheduler/RuntimeScheduler.h"

#define MAIN_SCENE_PATH "./main menu.dt.scene"
#define GAME_SCENE_PATH "./game_scene.dt.scene"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

const DtScene* main_scene;
DtUpdateContext update_ctx = (DtUpdateContext) {
    .delta_time = 0.2f,
    .fixed_delta_time = 0.2f,
};

extern const DtScene* game_scene;

struct nk_context* ctx;
struct nk_colorf bg;

static void initialize_main_scene();
static void initialize_window();

static void deinitialize_ecs_manager();
static void deinitialize_window();

int main(void) {
    initialize_main_scene();
    load_game_lib();
    game_scene = dt_add_scene(GAME_SCENE_PATH);
    initialize_window();


    while (!WindowShouldClose()) {
        dt_update_handler_update(main_scene->update_handler, &update_ctx);

        UpdateNuklear(ctx);
        dt_draw_handler_draw(main_scene->draw_handler);

        BeginDrawing();

        ClearBackground(ColorFromNuklearF(bg));
        DrawNuklear(ctx);

        EndDrawing();
    }

    deinitialize_ecs_manager();
    deinitialize_window();

    return 0;
}

static void initialize_main_scene() {
    main_scene = dt_add_scene(MAIN_SCENE_PATH);
    dt_scenes_set_active_by(main_scene);
    dt_update_handler_init(main_scene->update_handler);
    dt_draw_handler_init(main_scene->draw_handler);
}

static void initialize_window() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dt Editor");
    SetTargetFPS(60);

    const int fontSize = 18;
    Font font = LoadFontFromNuklear(fontSize);
    ctx = InitNuklearEx(font, fontSize);
    bg = (struct nk_colorf) {
        0.1f,
        0.18f,
        0.24f,
        1.0f,
    };
}

static void deinitialize_ecs_manager() { dt_scene_unload_by(main_scene); }

static void deinitialize_window() {
    UnloadNuklear(ctx);
    CloseWindow();
}
