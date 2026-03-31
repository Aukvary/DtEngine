#ifndef GAME_SCRIPTS_H
#define GAME_SCRIPTS_H

#include <raylib.h>
#include "EditorApi.h"
#include "Ecs/RegisterHandler.h"

void on_num_change(DtEcsPool* pool, DtEntity e);

#define GAME_TYPE(X, name)\
    X(int, field1, name, DTE_INSPECTOR_HIDE)\
    X(char*, num, name, DTE_ON_FIELD_CHANGE(on_num_change))
DT_DEFINE_COMPONENT(GameType, GAME_TYPE)

#endif /*GAME_SCRIPTS_H*/
