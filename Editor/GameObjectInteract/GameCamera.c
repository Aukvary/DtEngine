#include <nuklear.h>


#include "DtAllocators.h"
#include "Ecs/DtEcs.h"
#include "Ecs/RegisterHandler.h"
#include "GameObjectInteract.h"

extern struct nk_context* nk_ctx;

static Camera2D camera;
static float camera_move_speed = 30.0f;
static float camera_zoom_speed = 0.1f;

static UpdateSystem* game_camera_system_new();
static void game_camara_system_init(DtEcsManager* manager, void* data);
static void game_camera_system_update(void* data, DtUpdateContext* ctx);
static void camera_move();
static void camera_zoom();

DT_REGISTER_UPDATE(GameCameraSystem, game_camera_system_new);

Camera2D dte_game_camera() { return camera; }

static UpdateSystem* game_camera_system_new() {
    UpdateSystem* system = DT_MALLOC(sizeof(UpdateSystem));

    *system = (UpdateSystem) {
        .data = NULL,

        .init = game_camara_system_init,
        .update = game_camera_system_update,
        .destroy = NULL,
    };

    return system;
}

static void game_camara_system_init(DtEcsManager* manager, void* data) {
    camera = (Camera2D) {
        .offset =
            (Vector2) {
                (float) GetScreenWidth() / 2,
                (float) GetScreenHeight() / 2,
            },
        .target = (Vector2) {.0f, .0f},
        .rotation = 0.0f,
        .zoom = 1.f,
    };
}

static void game_camera_system_update(void* data, DtUpdateContext* ctx) {
    if (nk_item_is_any_active(nk_ctx) || nk_window_is_any_hovered(nk_ctx)) {
        return;
    }
    camera_move();
    camera_zoom();
}

static void camera_move() {
    float wheel = GetMouseWheelMove();
    float speed = camera_move_speed / camera.zoom;
    if (wheel && !IsKeyDown(KEY_LEFT_ALT)) {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
            camera.target.x -= wheel * speed;
        } else {
            camera.target.y -= wheel * speed;
        }
    }

    if (IsKeyDown(KEY_A)) {
        camera.target.x -= speed * 0.5f;
    }

    if (IsKeyDown(KEY_S)) {
        camera.target.y += speed * 0.5f;
    }

    if (IsKeyDown(KEY_D)) {
        camera.target.x += speed * 0.5f;
    }

    if (IsKeyDown(KEY_W)) {
        camera.target.y -= speed * 0.5f;
    }
}

static void camera_zoom() {
    if (!IsKeyDown(KEY_LEFT_ALT))
        return;

    float wheel = GetMouseWheelMove();
    if (!wheel)
        return;

    float speed = camera_zoom_speed * camera.zoom;
    camera.zoom += wheel * speed;
    if (camera.zoom < 0) {
        camera.zoom = 0;
    }
}
