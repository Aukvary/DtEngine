#include <string.h>
#include "RuntimeScheduler.h"

static DtRbTree parsers;

static void parse_int(cJSON* src, void* dst);
static void parse_unsigned_int(cJSON* src, void* dst);
static void parse_short(cJSON* src, void* dst);
static void parse_unsigned_short(cJSON* src, void* dst);
static void parse_long(cJSON* src, void* dst);
static void parse_unsigned_long(cJSON* src, void* dst);
static void parse_long_long(cJSON* src, void* dst);
static void parse_unsigned_long_long(cJSON* src, void* dst);

static u64 get_hash(const char* name) {
    int hash = 2147483647;
    while (*name) {
        hash ^= *name++;
        hash *= 314159;
    }
    return hash;
}

__attribute__((constructor)) static void dt_parser_initialize(void) {
    parsers = dt_rb_tree_new();
    dt_add_type_parser("int", parse_int);
    dt_add_type_parser("unsigned int", parse_unsigned_int);
    dt_add_type_parser("short", parse_short);
    dt_add_type_parser("unsigned short", parse_unsigned_short);
    dt_add_type_parser("long", parse_long);
    dt_add_type_parser("unsigned long", parse_unsigned_long);
    dt_add_type_parser("long long", parse_long_long);
    dt_add_type_parser("unsigned long long", parse_unsigned_long_long);
}

void dt_add_type_parser(const char* type, const TypeParser parser) {
    const u64 hash = get_hash(type);
    dt_rb_tree_add(&parsers, parser, hash);
}

void dt_link_type_parser(const char* type, const char* base_type) {
    TypeParser parser;
    if ((parser = dt_rb_tree_get(&parsers, get_hash(base_type)))) {
        dt_rb_tree_add(&parsers, parser, get_hash(type));
    }
}

void dt_parse_type(const char* type, cJSON* src, void* dst) {
    const TypeParser parser = dt_rb_tree_get(&parsers, get_hash(type));
    if (parser) {
        parser(src, dst);
    }
}

static void parse_int(cJSON* src, void* dst) {
    if (cJSON_IsNumber(src)) {
        int num = (int)cJSON_GetNumberValue(src);
        memcpy(dst, &num, sizeof(int));
    }
}

static void parse_unsigned_int(cJSON* src, void* dst) {
    if (cJSON_IsNumber(src)) {
        unsigned int num = (unsigned int)cJSON_GetNumberValue(src);
        memcpy(dst, &num, sizeof(unsigned int));
    }
}

static void parse_short(cJSON* src, void* dst) {
    if (cJSON_IsNumber(src)) {
        short num = (short)cJSON_GetNumberValue(src);
        memcpy(dst, &num, sizeof(short));
    }
}

static void parse_unsigned_short(cJSON* src, void* dst) {
    if (cJSON_IsNumber(src)) {
        unsigned short num = (unsigned short)cJSON_GetNumberValue(src);
        memcpy(dst, &num, sizeof(unsigned short));
    }
}

static void parse_long(cJSON* src, void* dst) {
    if (cJSON_IsNumber(src)) {
        long num = (long)cJSON_GetNumberValue(src);
        memcpy(dst, &num, sizeof(long));
    }
}

static void parse_unsigned_long(cJSON* src, void* dst) {
    if (cJSON_IsNumber(src)) {
        unsigned long num = (unsigned long)cJSON_GetNumberValue(src);
        memcpy(dst, &num, sizeof(unsigned long));
    }
}

static void parse_long_long(cJSON* src, void* dst) {
    if (cJSON_IsNumber(src)) {
        long long num = (long long)cJSON_GetNumberValue(src);
        memcpy(dst, &num, sizeof(long long));
    }
}

static void parse_unsigned_long_long(cJSON* src, void* dst) {
    if (cJSON_IsNumber(src)) {
        unsigned long long num = (unsigned long long)cJSON_GetNumberValue(src);
        memcpy(dst, &num, sizeof(unsigned long long));
    }
}