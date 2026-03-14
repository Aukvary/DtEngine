#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "DtNumericalTypes.h"

#define DT_VEC_MAGIC 0x44745663

/**
 * @brief struct for handle iteration with FOREACH macros

 * @warning filed in struct must be named "iterator" to use with FOREACH macros
 */
typedef struct {
    void (*start)(void*);
    void* (*current)(void*);
    bool (*has_current)(void*);
    void (*next)(void*);

    void* enumerable;
} DtIterator;

/**
 * @brief iterate for collection
 *
 * @param T pointer to value will be cast to T
 * @param var name of local variable
 * @param container pointer to struct with field with name iterator
 * @param block_code body of loop
 *
 * @note use {}-block in loop body
 */
#define FOREACH(T, var, iter, block_code)                                                          \
    ({                                                                                             \
        (iter)->start((iter)->enumerable);                                                         \
        T var;                                                                                     \
        while ((iter)->has_current((iter)->enumerable)) {                                          \
            var = *(T*) (iter)->current((iter)->enumerable);                                       \
            block_code;                                                                            \
            (iter)->next((iter)->enumerable);                                                      \
        }                                                                                          \
    })

/**
 * @brief vector data
 */
typedef struct {
    int magic;

    size_t count;
    size_t capacity;

    size_t element_size;

    DtIterator iterator;
    int iter_locked;
    size_t current;

    void* data;
} DtVecHeader;

/**
 * @brief wrapper
 */
#define DT_VEC(T) T*

/**
 * @brief return pointer to vector data array with type T
 *
 * @param T type for vector
 * @param capacity base capacity for vector
 */
#define DT_VEC_NEW(T, capacity) (T*) dt_vec_new(sizeof(T), capacity)

/**
 * @brief add value to vector
 *
 * @param data pointer to data array
 */
#define DT_VEC_ADD(data, value)                                                                    \
    ({                                                                                             \
        typeof(value) tmp_var = (value);                                                           \
        data = dt_vec_add(data, &tmp_var);                                                         \
    })

/**
 * @brief remove value to vector
 *
 * @param data pointer to data array
 */
#define DT_VEC_REMOVE(data, el)                                                                    \
    {                                                                                              \
        typeof(el) e = el;                                                                         \
        dt_vec_remove(data, &e);                                                                   \
    }

/**
 * @brief return header for using in FOREACH-macros
 *
 * @param data pointer to data array
 */
#define DT_VEC_ITERATOR(data) &dt_vec_header(data)->iterator

/**
 * @brief return pointer to vector data array
 *
 * @param item_size size of type
 * @param capacity base capacity
 *
 * @warning use VEC_NEW macros-wrapper
 */
void* dt_vec_new(size_t item_size, size_t capacity);

/**
 * @brief return header of vector info
 *
 * @param data pointer to data array
 */
static DtVecHeader* dt_vec_header(void* data) {
    DtVecHeader* header = &((DtVecHeader*) data)[-1];
    if (header->magic != DT_VEC_MAGIC) {
        printf("DtVecHeader magic number mismatch\n");
        exit(1);
    }
    return header;
}
/**
 * @brief return capacity of data array
 *
 * @param data pointer to data array
 */
static size_t dt_vec_capacity(void* data) { return dt_vec_header(data)->capacity; }

/**
 * @brief return count of data array
 *
 * @param data pointer to data array
 */
static size_t dt_vec_count(void* data) { return dt_vec_header(data)->count; }

/**
 * @brief add value to vector data array
 *
 * @param data pointer to data array
 * @param  value pointer to values
 *
 * @warning use VEC_ADD macros-wrapper
 */
void* dt_vec_add(void* data, void* value);

/**
 * @brief remove value from data array with this index
 *
 * @param data pointer to data array
 * @param  idx value index
 */
void dt_vec_pop(void* data, int idx);

/**
 * @brief remove value from data array
 *
 * @param data pointer to data array
 * @param  value pointer to values
 *
 * @warning use VEC_REMOVE macros-wrapper
 */
void dt_vec_remove(void* data, void* value);

/**
 * @brief return header for using in FOREACH macros
 *
 * @param data pointer to data array
 *
 * @warning use VEC_ITERATOR macros-wrapper
 */
DtVecHeader* dt_vec_iterator(void* data);

/**
 * @brief free vector memory
 *
 * @param data pointer to vector header
 */
void dt_vec_free(void* data);

//TODO:comment
typedef struct DtRbNode {
    bool is_red;
    struct DtRbNode* parent;
    struct DtRbNode* left;
    struct DtRbNode* right;

    void* data;
    u64 hash;
} DtRbNode;

//TODO:comment
typedef struct {
    DtRbNode* root;

    DtIterator iterator;
    DtRbNode* iterator_node;
} DtRbTree;

//TODO:comment
DtRbTree dt_rb_tree_new();
//TODO:comment
void dt_rb_tree_add(DtRbTree* head, void* data, u64 hash);
//TODO:comment
void* dt_rb_tree_get(DtRbTree* head, u64 hash);
//TODO:comment
void dt_rb_tree_remove(DtRbTree* head, u64 hash);
//TODO:comment
void dt_rb_tree_free(DtRbTree* head);

#endif
