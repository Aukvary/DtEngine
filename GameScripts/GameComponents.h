#ifndef GAME_COMPONENTS_H
#define GAME_COMPONENTS_H
#include <raylib.h>
#include "EditorApi.h"
#include "scheduler/RuntimeScheduler.h"

void on_change_path_to_sprite(DtEcsPool* data, DtEntity entity);

#define SPRITE(X, name)                                                                            \
    X(char*, path_to_sprite, name, DTE_ON_FIELD_CHANGE(on_change_path_to_sprite))                  \
    X(Texture2D, texture, name)                                                                    \
    X(Vector2, origin, name)                                                                       \
    X(Color, color, name)                                                                          \
    X(Rectangle, source, name)                                                                     \
    X(bool, horizontal_flip, name)                                                                 \
    X(bool, vertical_flip, name)

DT_DEFINE_COMPONENT(Sprite, SPRITE)

#endif /*GAME_COMPONENTS_H*/
