#ifndef GAME_LIB_H
#define GAME_LIB_H
#include <stdbool.h>
#include "scheduler/RuntimeScheduler.h"

void load_game_lib();
void build_game_lib();
void reload_game_lib(bool rebuild);
void save_game_scene();
void load_game_scene();
void reload_game_scene();

void init_game_systems();

#endif /*GAME_LIB_H*/
