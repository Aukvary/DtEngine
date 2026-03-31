#ifndef GAME_COMPONENTS_H
#define GAME_COMPONENTS_H
#include <raylib.h>
#include "EditorApi.h"
#include "scheduler/RuntimeScheduler.h"

void on_change_path_to_sprite(DtEcsPool* data, DtEntity entity);

#define SPRITE(x, name)                                                                            \
    x(char*, path_to_sprite, name,                                                                 \
      (DtAttributeData) {                                                                          \
          .attribute_name = DTE_ON_FIELD_CHANGE,                                                   \
          .data = on_change_path_to_sprite,                                                      \
      }) x(Texture2D, texture, name) x(Vector2, origin, name) x(Color, color, name)                \
        x(Rectangle, source, name) x(bool, horizontal_flip, name) x(bool, vertical_flip, name)

DT_DEFINE_COMPONENT(Sprite, SPRITE)

#endif /*GAME_COMPONENTS_H*/
