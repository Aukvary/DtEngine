#ifndef EDITOR_API_H
#define EDITOR_API_H
#include "scheduler/FileHandle.h"
#include "scheduler/RuntimeScheduler.h"

#define DECLARE_EDITOR_FUNC_TABLE DT_EXPORT DtEFuncTable func_table;

#define DTE_EDIT_DRAW "editor_draw"
#define DTE_EDIT_UPDATE "editor_update"

#define DTE_INSPECTOR_HIDE "hide"
#define DTE_ON_FIELD_CHANGE "on_field_change"

#define DTE_INIT dte_gl_init
#define DTE_DEINIT dte_gl_deinit

#define DTE_INIT_STR "dte_gl_init"
#define DTE_DEINIT_STR "dte_gl_deinit"

#define DTE_DECLARE_EDITOR_FUNC(init, deinit)                                                      \
    DT_EXPORT void (*DTE_INIT)(void) = init;                                                       \
    DT_EXPORT void (*DTE_DEINIT)(void) = deinit;

typedef enum { DTE_NORMAL, DTE_WARNING, DTE_ERROR } DtEMessageType;

typedef struct {
    char text[256];
    DtEMessageType type;
} DtEMessage;

typedef struct {
    // general
    DtEnvironment* (*environment_instance)(void);

    // logs
    void (*log)(const char*, ...);
    void (*warn)(const char*, ...);
    void (*error)(const char*, ...);

    // type parsing
    void (*add_inspector_type)(const char* type, bool (*handle)(const char* name, void* data));
    void (*add_parser_json_to_type)(const char* type, TypeParser parser);
    void (*link_parser_json_to_type)(const char* type, const char* base_type);
    void (*add_serializer_type_to_json)(const char* type, TypeSerializer serializer);
} DtEFuncTable;

DT_EXPORT
void dte_log(const char* format, ...);

DT_EXPORT
void dte_warning_log(const char* format, ...);

DT_EXPORT
void dte_error_log(const char* format, ...);

DT_EXPORT
void dte_add_inspector_type(const char* type, bool (*handle)(const char* name, void* data));

bool dte_inspector_field_draw(const char* type, const char* name, void* data);

#endif /*EDITOR_API_H*/
