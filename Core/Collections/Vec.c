#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Collections.h"

#include "DtAllocators.h"
#include "DtNumericalTypes.h"

/**
 * @brief set interator pointer to start
 *
 * @param data pointer to vector header
 *
 * @warning func must be called by FOREACH macros else it can throw exceptions
 */
static void vec_start(void* data);

/**
 * @brief return pointer to value
 *
 * @param data pointer to vector header
 *
 * @warning func must be called by FOREACH macros else it can throw exceptions
 */
static void* vec_current(void* data);

static bool vec_has_current(void* data);

/**
 * @brief move iterator pointer and return 1 if current != NULL else 0
 *
 * @param data pointer to vector header
 *
 * @warning func must be called by FOREACH macros else it can throw exceptions
 */
static void vec_next(void* data);

void* dt_vec_new(const size_t item_size, const size_t capacity) {
    DtVecHeader* header = DT_MALLOC(sizeof(DtVecHeader) + item_size * capacity);

    *header = (DtVecHeader) {
        .magic = DT_VEC_MAGIC,
        .element_size = item_size,
        .capacity = capacity,
        .count = 0,

        .data = header + 1,

        .iterator =
            (DtIterator) {
                .start = vec_start,
                .current = vec_current,
                .has_current = vec_has_current,
                .next = vec_next,
                .enumerable = header,
            },
        .iter_locked = 0,
    };

    return header->data;
}

void* dt_vec_add(void* data, void* value) {
    DtVecHeader* header = dt_vec_header(data);

    if (header->count == header->capacity) {
        size_t new_capacity = header->capacity == 0 ? 10 : header->capacity * 2;
        DtVecHeader* temp =
            DT_REALLOC(header, sizeof(DtVecHeader) + new_capacity * header->element_size);

        if (temp == NULL) {
            printf("vector memory allocation exception\n");
            exit(1);
        }

        header = temp;
        header->data = header + 1;
        header->capacity = new_capacity;
        header->iterator.enumerable = header;
    }

    u8* data_ptr = header->data;
    size_t offset = header->count * header->element_size;

    memcpy(data_ptr + offset, value, header->element_size);
    header->count++;

    return header->data;
}

void dt_vec_pop(void* data, int idx) {
    DtVecHeader* header = dt_vec_header(data);

    if (header->iter_locked) {
        printf("cant change vector where it's locked\n");
        exit(1);
    }
    if (idx >= header->count)
        exit(1);

    for (int i = idx; i < header->count - 1; i++) {
        memcpy((u8*) data + header->element_size * i, (u8*) data + header->element_size * (i + 1),
               header->element_size);
    }

    header->count--;
}

void dt_vec_remove(void* data, void* value) {
    DtVecHeader* header = dt_vec_header(data);

    if (header->iter_locked) {
        printf("cant change vector where it's locked\n");
        exit(1);
    }

    int idx;

    for (int i = 0; i < header->count; i++) {
        if (memcmp(value, (u8*) data + header->element_size * i, header->element_size) == 0) {
            idx = i;
            header->count--;
            break;
        }
    }

    for (int i = idx; i < header->count; i++) {
        memcpy((u8*) data + header->element_size * i, (u8*) data + header->element_size * (i + 1),
               header->element_size);
    }
}

void vec_start(void* data) {
    DtVecHeader* header = data;
    header->current = 0;
}

void* vec_current(void* data) {
    DtVecHeader* header = data;

    void* element = (char*) header->data + (header->current * header->element_size);
    return element;
}

static bool vec_has_current(void* data) {
    DtVecHeader* header = data;
    return header->current < header->count;
}

void vec_next(void* data) {
    DtVecHeader* header = data;
    header->current++;
}

inline void dt_vec_free(void* data) { free(dt_vec_header(data)); }
