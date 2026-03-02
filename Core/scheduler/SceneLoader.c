// #include <cjson/cJSON.h>
// #include "scheduler/DtScheduler.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "../Core/DtAllocators.h"
// #include "Collections/Collections.h"
// #if defined(_WIN32) || defined(_WIN64)
// #include <windows.h>
// #else
// #include <dirent.h>
// #include <sys/types.h>
// #endif
//
//
// #define SCENES_PATH "./source/scenes/"
// #define SCENE_EXTENSION ".dt.scene"
//
// static DtRbTree scenes;
// static DtScene* dt_scene_parse(const char* file_name);
// static char* dt_scene_parse_name(FILE* file_input);
// static void dt_scene_parse_ecs_manager_cfg(cJSON* cfg, DtScene* scene);
// static void dt_scene_parse_systems(cJSON* systems, DtScene* scene);
// static void dt_scene_parse_entities(cJSON* entities, DtScene* scene);
//
// static u64 get_scene_hash(const char* name) {
//     int hash = 2147483647;
//     while (*name) {
//         hash ^= *name++;
//         hash *= 314159;
//     }
//     return hash;
// }
//
// static int is_scene(const char* filename) {
//     const size_t len_filename = strlen(filename);
//     const size_t len_ext = strlen(SCENE_EXTENSION);
//
//     if (len_filename < len_ext)
//         return 0;
//     return strcmp(filename + len_filename - len_ext, SCENE_EXTENSION) == 0;
// }
//
// void dt_add_all_scenes(void) {
//     scenes = dt_rb_tree_new();
// #if defined(_WIN32) || defined(_WIN64)
//     WIN32_FIND_DATA findFileData;
//     char search_path[MAX_PATH];
//
//     snprintf(search_path, MAX_PATH, "%s\\*", SCENES_PATH);
//     HANDLE hFind = FindFirstFile(search_path, &findFileData);
//
//     if (hFind == INVALID_HANDLE_VALUE) {
//         printf("[DEBUG]Directory not found: %s\n", SCENES_PATH);
//         return;
//     }
//
//     do {
//         if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//             continue;
//         if (!is_scene(findFileData.cFileName)) continue;
//
//         DtScene* scene = dt_scene_parse(findFileData.cFileName);
//
//         // dt_rb_tree_add(&scenes, )
//     } while (FindNextFile(hFind, &findFileData) != 0);
//
//
//
//     FindClose(hFind);
// #else
//     DIR* dir = opendir(directory_path);
//     if (!dir) {
//         perror("opendir");
//         return;
//     }
//
//     struct dirent* entry;
//     while ((entry = readdir(dir)) != NULL) {
//         if (entry->d_type == DT_REG) {
//             if (is_scene(entry->d_name)) {
//                 printf("%s\n", entry->d_name);
//             }
//         }
//     }
//
//     closedir(dir);
// #endif
// }
//
// static DtScene* dt_scene_parse(const char* file_name) {
//     FILE* file = fopen(file_name, "rb");
//
//     if (file == NULL) {
//         //TODO: add handle
//     }
//
//     fseek(file, 0, SEEK_END);
//
//     i32 size = ftell(file);
//     fseek(file, 0, SEEK_SET);
//
//     char* scene_info = DT_STACK_ALLOC(size + 1);
//     fgets(scene_info, size, file);
//
//     // char* scene_json
//
//     cJSON* root = cJSON_Parse(file_name);
//     if (root == NULL) {
//         //TODO: add handle
//     }
//
//     cJSON* id = cJSON_GetObjectItem(root, "id");
//     if (id == NULL) {
//         //TODO: add handle
//     }
//
//
//     fclose(file);
// }
//
// static char* dt_scene_parse_name(FILE* file_input) {
//     char* dot = strstr(file_input, ".");
//     *dot = '\0';
//     u16 name_len = strlen(file_input);
//     char* copy_name = DT_MALLOC(name_len + 1);
//     memcpy(copy_name, file_input, name_len);
//     copy_name[name_len] = '\0';
// }
//
// static void dt_scene_parse_ecs_manager_cfg(cJSON* cfg, DtScene* scene) {
//
// }
//
// static void dt_scene_parse_systems(cJSON* systems, DtScene* scene) {
//
// }
//
// static void dt_scene_parse_entities(cJSON* entities, DtScene* scene) {
//
// }
//
// const DtScene* dt_scenes_get_active(void) {}
//
// const DtScene* dt_scenes_set_active_without_unload(int idx) {}
//
// void dt_scenes_set_active_with_unload(int idx) {}
//
// void dt_scene_load_by_id(int idx) {}
//
// void dt_scene_unload_by_id(int idx) {}
//
// bool dt_scenes_scene_is_load(int idx) {}
