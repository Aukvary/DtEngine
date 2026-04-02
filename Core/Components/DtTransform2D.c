#include "Components.h"
#include "Ecs/RegisterHandler.h"

void dt_transform_2d_reset(void* data) {
    DtTransform2D* transform = data;

    transform->position = (Vector2){0, 0};
    transform->scale = (Vector2){1, 1};
    transform->rotation = 0.0f;
}
DT_REGISTER_COMPONENT(DtTransform2D, DT_TRANSFORM_2D, DT_RESET_ATTR(dt_transform_2d_reset))