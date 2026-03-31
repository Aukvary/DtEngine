#include "GameComponents.h"

extern DtEFuncTable func_table;

static void sprite_init(void* data);
DT_REGISTER_COMPONENT(Sprite, sprite_init, SPRITE)

static void sprite_init(void* data) {
    Sprite* sprite = data;

    if (!sprite->path_to_sprite) {
        func_table.error("Failed to load image from path: NULL");
        sprite->texture.id = 0;
        return;
    }

    Image image = LoadImage(sprite->path_to_sprite);
    if (!image.data) {
        func_table.error("Failed to load image from path: %s", sprite->path_to_sprite);
    }

    sprite->texture = LoadTextureFromImage(image);
    UnloadImage(image);
}

void on_change_path_to_sprite(DtEcsPool* pool, DtEntity entity) {
    Sprite* sprite = dt_ecs_pool_get(pool, entity);

    if (sprite->texture.id != 0) {
        UnloadTexture(sprite->texture);
    }

    if (!sprite->path_to_sprite) {
        func_table.error("Failed to load image from path: NULL");
        sprite->texture.id = 0;
        return;
    }

    Image image = LoadImage(sprite->path_to_sprite);
    if (!image.data) {
        func_table.error("Failed to load image from path: %s", sprite->path_to_sprite);
    }

    sprite->texture = LoadTextureFromImage(image);
    UnloadImage(image);
}
