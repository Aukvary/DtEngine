#include <math.h>
#include "Components/Components.h"
#include "DtAllocators.h"
#include "EditorApi.h"
#include "GameComponents.h"
#include "scheduler/RuntimeScheduler.h"

typedef struct {
    DrawSystem system;

    DtEcsFilter* filter;

    DtEcsPool* transforms;
    DtEcsPool* sprites;
} DrawSpriteSystem;

DrawSystem* draw_sprite_new();
void draw_sprite_init(DtEcsManager* manager, void* data);
void draw_sprite_draw(void* data);
void draw_sprite_destroy(void* data);

DT_REGISTER_DRAW(DrawSprite, draw_sprite_new,
                 (DtAttributeData) {
                     .attribute_name = DTE_EDIT_DRAW,
                     .data = draw_sprite_init,
                 })

DrawSystem* draw_sprite_new() {
    DrawSpriteSystem* draw_sprite = DT_MALLOC(sizeof(DrawSpriteSystem));

    *draw_sprite = (DrawSpriteSystem) {
        .system =
            (DrawSystem) {
                .data = draw_sprite,
                .init = draw_sprite_init,
                .draw = draw_sprite_draw,
                .destroy = draw_sprite_destroy,
            },
    };

    return &draw_sprite->system;
}

void draw_sprite_init(DtEcsManager* manager, void* data) {
    DrawSpriteSystem* sys = data;

    DtEcsMask mask = dt_mask_new(manager, 2, 0);
    DT_MASK_INC(mask, DtTransform2D);
    DT_MASK_INC(mask, Sprite);

    sys->filter = dt_mask_end(mask);

    sys->transforms = DT_ECS_MANAGER_GET_POOL(manager, DtTransform2D);
    sys->sprites = DT_ECS_MANAGER_GET_POOL(manager, Sprite);
}

static void draw_sprite_entity(Sprite* sprite, DtTransform2D* transform) {
    float tex_w = (float) sprite->texture.width;
    float tex_h = (float) sprite->texture.height;

    Rectangle sourceRec = {
        .x = sprite->source.x * tex_w,
        .y = sprite->source.y * tex_h,
        .width = sprite->source.width * tex_w,
        .height = sprite->source.height * tex_h,
    };

    if (sprite->horizontal_flip)
        sourceRec.width *= -1.0f;
    if (sprite->vertical_flip)
        sourceRec.height *= -1.0f;

    float final_w = fabsf(sourceRec.width) * transform->scale.x;
    float final_h = fabsf(sourceRec.height) * transform->scale.y;

    Rectangle destRec = {
        .x = transform->position.x,
        .y = transform->position.y,
        .width = final_w,
        .height = final_h,
    };

    Vector2 pivot = {
        .x = sprite->origin.x * destRec.width,
        .y = sprite->origin.y * destRec.height,
    };

    if (sprite->texture.id > 0) {
        DrawTexturePro(sprite->texture, sourceRec, destRec, pivot, transform->rotation,
                       sprite->color);
    } else {
        DrawRectanglePro(destRec, pivot, transform->rotation, sprite->color);
    }
}

void draw_sprite_draw(void* data) {
    DrawSpriteSystem* sys = data;

    FOREACH(DtEntity, e, &sys->filter->entities.entities_iterator, ({
                Sprite* sprite = dt_ecs_pool_get(sys->sprites, e);
                DtTransform2D* transform = dt_ecs_pool_get(sys->transforms, e);

                if (sprite && transform) {
                    draw_sprite_entity(sprite, transform);
                }
            }));
}

void draw_sprite_destroy(void* data) {}
