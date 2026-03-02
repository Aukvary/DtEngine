#ifndef LAZY_LOAD_H
#define LAZY_LOAD_H
#include "DtScheduler.h"
#include "Ecs/RegisterHandler.h"

#ifdef _WIN32
#include <windows.h>
#define DT_LIB_HANDLE HMODULE
#define DT_LIB_LOAD(name) LoadLibrary(name)
#define DT_LIB_GET(handle, name) GetProcAddress(handle, name)
#define DT_LIB_CLOSE(handle) FreeLibrary(handle)
#define DT_LIB_EXTENSION ".dll"

#define DT_EXPORT __declspec(dllexport)
#else
#include <dlfcn.h>
#define DT_LIB_HANDLE void*
#define DT_LIB_LOAD(name) dlopen(name, RTLD_LAZY)
#define DT_LIB_GET(handle, name) dlsym(handle, name)
#define DT_LIB_CLOSE(handle) dlclose(handle)
#define DT_LIB_EXTENSION ".so"

#define DT_EXPORT __declspec(dllexport)
#endif

#define DT_DEFINE_MODULE(name, init, deinit)                                                       \
    DT_EXPORT const char* module_name = name;                                                      \
    DT_EXPORT void (*initialize)(DtEnvironment * env) = init;                                      \
    DT_EXPORT void (*deinitialize)(DtEnvironment * env) = deinit;

// TODO: add comment
typedef struct DtEnvironment DtEnvironment;

// TODO: add comment
typedef struct {
    char* name;

    void (*initialize)(DtEnvironment* env);
    void (*deinitialize)(DtEnvironment* env);

    DT_LIB_HANDLE handle;
} ModuleInfo;

struct DtEnvironment {
    DtScene* active_scene;
    DtRbTree modules;
    DtRbTree scenes;

    const DtComponentData** components;
    u16 components_count;
    const DtUpdateData** updaters;
    u16 updaters_count;
    const DtDrawData** drawers;
    u16 drawers_count;
};

// TODO: comments
void dt_load_module(DtEnvironment* env, const char* path);

// TODO: comments
DT_EXPORT DtEnvironment* dt_environment_instance(void);

// TODO: comments
void dt_add_scenes(const char* directory);
// TODO: comments
void dt_add_scene(const char* path);


// TODO: comments
DtRawEntity* dt_load_prefab(const char* path);

#endif /*LAZY_LOAD_H*/
