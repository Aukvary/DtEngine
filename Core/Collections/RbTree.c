#include "Collections/Collections.h"
#include "DtAllocators.h"

static DtRbNode* dt_rb_node_new(void* data, u64 hash);

static void dt_rb_node_left_rotate(DtRbNode** root, DtRbNode* node);
static void dt_rb_node_right_rotate(DtRbNode** root, DtRbNode* node);

static void dt_rb_transplant(DtRbNode** root, DtRbNode* u, DtRbNode* v);
static DtRbNode* dt_rb_node_min(DtRbNode* node);

static void dt_rb_node_add(DtRbNode** root, void* data, u64 hash);
static void dt_rb_node_add_balance(DtRbNode** root, DtRbNode* node);

static void* dt_rb_node_get(DtRbNode* root, u64 hash);

static DtRbNode* dt_rb_node_remove(DtRbNode** root, u64 hash);
static void dt_rb_node_remove_balance(DtRbNode** root, DtRbNode* x, DtRbNode* parent);

static void dt_rb_start(void* data);
static void* dt_rb_current(void* data);
static bool dt_rb_has_current(void* data);
static void dt_rb_next(void* data);

static DtRbNode* dt_rb_node_new(void* data, u64 hash) {
    DtRbNode* node = DT_MALLOC(sizeof(DtRbNode));

    *node = (DtRbNode) {
        .is_red = true,
        .data = data,
        .left = NULL,
        .right = NULL,
        .parent = NULL,
        .hash = hash,
    };

    return node;
}

static void dt_rb_node_left_rotate(DtRbNode** root, DtRbNode* node) {

    DtRbNode* y = node->right;
    node->right = y->left;

    if (y->left)
        y->left->parent = node;

    y->parent = node->parent;

    if (node->parent == NULL)
        *root = y;
    else if (node == node->parent->left)
        node->parent->left = y;
    else
        node->parent->right = y;

    y->left = node;
    node->parent = y;
}

static void dt_rb_node_right_rotate(DtRbNode** root, DtRbNode* node) {

    DtRbNode* y = node->left;
    node->left = y->right;

    if (y->right)
        y->right->parent = node;

    y->parent = node->parent;

    if (node->parent == NULL)
        *root = y;
    else if (node == node->parent->right)
        node->parent->right = y;
    else
        node->parent->left = y;

    y->right = node;
    node->parent = y;
}

static void dt_rb_transplant(DtRbNode** root, DtRbNode* u, DtRbNode* v) {
    if (u->parent == NULL)
        *root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;

    if (v)
        v->parent = u->parent;
}

static DtRbNode* dt_rb_node_min(DtRbNode* node) {
    while (node->left)
        node = node->left;
    return node;
}

static void dt_rb_node_add(DtRbNode** root, void* data, u64 hash) {

    if (*root == NULL) {
        *root = dt_rb_node_new(data, hash);
        (*root)->is_red = false;
        return;
    }

    DtRbNode* prev = NULL;
    DtRbNode* cur = *root;

    while (cur != NULL) {
        prev = cur;

        if (hash == cur->hash)
            return;

        if (hash < cur->hash)
            cur = cur->left;
        else
            cur = cur->right;
    }

    DtRbNode* node = dt_rb_node_new(data, hash);
    node->parent = prev;

    if (hash < prev->hash)
        prev->left = node;
    else
        prev->right = node;

    dt_rb_node_add_balance(root, node);
}

static void dt_rb_node_add_balance(DtRbNode** root, DtRbNode* node) {
    while (node != *root && node->parent->is_red) {
        DtRbNode* parent = node->parent;
        DtRbNode* grand = parent->parent;

        if (parent == grand->left) {
            DtRbNode* uncle = grand->right;
            if (uncle && uncle->is_red) {
                parent->is_red = false;
                uncle->is_red = false;
                grand->is_red = true;
                node = grand;
            } else {
                if (node == parent->right) {
                    node = parent;
                    dt_rb_node_left_rotate(root, node);
                    parent = node->parent;
                    grand = parent->parent;
                }

                parent->is_red = false;
                grand->is_red = true;
                dt_rb_node_right_rotate(root, grand);
            }

        } else {
            DtRbNode* uncle = grand->left;

            if (uncle && uncle->is_red) {
                parent->is_red = false;
                uncle->is_red = false;
                grand->is_red = true;
                node = grand;
            } else {
                if (node == parent->left) {
                    node = parent;
                    dt_rb_node_right_rotate(root, node);
                    parent = node->parent;
                    grand = parent->parent;
                }
                parent->is_red = false;
                grand->is_red = true;
                dt_rb_node_left_rotate(root, grand);
            }
        }
    }

    (*root)->is_red = false;
}

static void* dt_rb_node_get(DtRbNode* root, u64 hash) {
    DtRbNode* cur = root;

    while (cur != NULL) {
        if (cur->hash == hash)
            return cur->data;
        if (hash < cur->hash)
            cur = cur->left;
        else
            cur = cur->right;
    }

    return NULL;
}

static DtRbNode* dt_rb_node_remove(DtRbNode** root, u64 hash) {

    DtRbNode* z = *root;

    // поиск узла
    while (z && z->hash != hash) {
        if (hash < z->hash)
            z = z->left;
        else
            z = z->right;
    }

    if (!z)
        return NULL;

    DtRbNode* y = z;
    bool y_original_red = y->is_red;
    DtRbNode* x = NULL;
    DtRbNode* x_parent = NULL;

    if (z->left == NULL) {

        x = z->right;
        x_parent = z->parent;
        dt_rb_transplant(root, z, z->right);

    } else if (z->right == NULL) {

        x = z->left;
        x_parent = z->parent;
        dt_rb_transplant(root, z, z->left);

    } else {

        y = dt_rb_node_min(z->right);
        y_original_red = y->is_red;
        x = y->right;

        if (y->parent == z) {
            x_parent = y;
        } else {
            x_parent = y->parent;
            dt_rb_transplant(root, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        dt_rb_transplant(root, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->is_red = z->is_red;
    }

    if (!y_original_red)
        dt_rb_node_remove_balance(root, x, x_parent);

    return z;
}

static void dt_rb_node_remove_balance(
    DtRbNode** root,
    DtRbNode* x,
    DtRbNode* parent
) {
    while ((x == NULL || !x->is_red) && x != *root) {

        if (parent == NULL)
            break;

        if (x == parent->left) {

            DtRbNode* w = parent->right;

            if (w == NULL) {
                x = parent;
                parent = x->parent;
                continue;
            }

            if (w->is_red) {
                w->is_red = false;
                parent->is_red = true;
                dt_rb_node_left_rotate(root, parent);
                w = parent->right;
            }

            if ((w->left == NULL || !w->left->is_red) &&
                (w->right == NULL || !w->right->is_red)) {

                w->is_red = true;
                x = parent;
                parent = x->parent;
            }
            else {
                if (w->right == NULL || !w->right->is_red) {
                    if (w->left)
                        w->left->is_red = false;

                    w->is_red = true;
                    dt_rb_node_right_rotate(root, w);
                    w = parent->right;
                }
                w->is_red = parent->is_red;
                parent->is_red = false;

                if (w->right)
                    w->right->is_red = false;

                dt_rb_node_left_rotate(root, parent);
                x = *root;
                break;
            }

        } else {
            DtRbNode* w = parent->left;

            if (w == NULL) {
                x = parent;
                parent = x->parent;
                continue;
            }

            if (w->is_red) {
                w->is_red = false;
                parent->is_red = true;
                dt_rb_node_right_rotate(root, parent);
                w = parent->left;
            }

            if ((w->left == NULL || !w->left->is_red) &&
                (w->right == NULL || !w->right->is_red)) {

                w->is_red = true;
                x = parent;
                parent = x->parent;
            }
            else {

                if (w->left == NULL || !w->left->is_red) {
                    if (w->right)
                        w->right->is_red = false;

                    w->is_red = true;
                    dt_rb_node_left_rotate(root, w);
                    w = parent->left;
                }

                w->is_red = parent->is_red;
                parent->is_red = false;

                if (w->left)
                    w->left->is_red = false;

                dt_rb_node_right_rotate(root, parent);
                x = *root;
                break;
            }
        }
    }

    if (x)
        x->is_red = false;
}

DtRbTree dt_rb_tree_new() {
    return (DtRbTree) {
        .root = NULL,
    };
}

void dt_rb_tree_add(DtRbTree* head, void* data, u64 hash) {
    if (head->root == NULL) {
        head->root = dt_rb_node_new(data, hash);
        head->root->is_red = false;
        return;
    }

    dt_rb_node_add(&head->root, data, hash);
    head->root->is_red = false;
}

void* dt_rb_tree_get(DtRbTree* head, u64 hash) {
    return dt_rb_node_get(head->root, hash);
}

void dt_rb_tree_remove(DtRbTree* head, u64 hash) {
    if (head->root == NULL)
        return;

    dt_rb_node_remove(&head->root, hash);
}

static DtRbNode* dt_rb_find_leftmost(DtRbNode* node) {
    if (node == NULL) return NULL;
    while (node->left != NULL) {
        node = node->left;
    }
    return node;
}

static DtRbNode* dt_rb_find_rightmost(DtRbNode* node) {
    if (node == NULL) return NULL;
    while (node->right != NULL) {
        node = node->right;
    }
    return node;
}

static DtRbNode* dt_rb_find_next_inorder(DtRbNode* node) {
    if (node == NULL) return NULL;

    if (node->right != NULL) {
        return dt_rb_find_leftmost(node->right);
    }

    DtRbNode* parent = node->parent;
    while (parent != NULL && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }

    return parent;
}

static DtRbNode* dt_rb_find_prev_inorder(DtRbNode* node) {
    if (node == NULL) return NULL;

    if (node->left != NULL) {
        return dt_rb_find_rightmost(node->left);
    }

    DtRbNode* parent = node->parent;
    while (parent != NULL && node == parent->left) {
        node = parent;
        parent = parent->parent;
    }

    return parent;
}

static void dt_rb_start(void* data) {
    DtRbTree* tree = data;
    tree->iterator_node = dt_rb_find_leftmost(tree->root);
}

static void* dt_rb_current(void* data) {
    return ((DtRbTree*)data)->iterator_node;
}

static bool dt_rb_has_current(void* data) {
    return ((DtRbTree*)data)->iterator_node != NULL;
}

static void dt_rb_next(void* data) {
    DtRbTree* tree = data;
    if (tree->iterator_node != NULL) {
        tree->iterator_node = dt_rb_find_next_inorder(tree->iterator_node);
    }
}

static void dt_rb_prev(void* data) {
    DtRbTree* tree = data;
    if (tree->iterator_node != NULL) {
        tree->iterator_node = dt_rb_find_prev_inorder(tree->iterator_node);
    }
}

static void dt_rb_end(void* data) {
    DtRbTree* tree = data;
    tree->iterator_node = dt_rb_find_rightmost(tree->root);
}