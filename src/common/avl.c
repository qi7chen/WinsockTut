/**
 * Copyright (C) 2012-2014 ichenq@gmail.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "avl.h"
#include <malloc.h>
#include <assert.h>

/* tree node */
struct avl_node
{
    avl_key_t   key;            /* node data */
    void*       data;           /* associate data */
    int         height;         /* node height */
    struct avl_node* left;      /* left child */
    struct avl_node* right;     /* right child */
};

/* AVL tree */
struct avl_tree
{
    avl_node_t* root;   /* root node */
    size_t      size;   /* how many nodes */
};

#define MAX(a, b)       (a > b ? a : b)
#define HEIGHT(n)       (n != NULL ? n->height : -1)
#define ROT_LEFT        -1
#define ROT_RIGHT       1

/* allocate a new node */
static avl_node_t*  avl_new_node(avl_key_t key, void* data)
{
    avl_node_t* node = (avl_node_t*)malloc(sizeof(avl_node_t));
    if (node)
    {
        node->key = key;
        node->data = data;
        node->height = 0;   // leaf node default
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

/* free new allocated node */
static void avl_free_node(avl_node_t* node)
{
    if (node)
    {
        free(node);
    }
}

static int avl_get_balance(avl_node_t* node)
{
    if (node == NULL)
        return 0;
    else
        return HEIGHT(node->left) - HEIGHT(node->right);
}

static avl_node_t*  avl_node_min(avl_node_t* node)
{
    if (node != NULL)
    {
        while (node->left != NULL)
            node = node->left;
    }
    return node;
}

static avl_node_t* avl_node_max(avl_node_t* node)
{
    if (node != NULL)
    {
        while (node->right != NULL)
            node = node->right;
    }
    return node;
}

/* find if contains `key` */
static avl_node_t* avl_node_find(avl_node_t* root, avl_key_t key)
{
    if (root == NULL)
        return NULL;
    if (key < root->key)
        return avl_node_find(root->left, key);
    else if (key > root->key)
        return avl_node_find(root->right, key);
    else
        return root;
}


/*
 * left-left rotation
 *
 *        A               B
 *       / \            /   \
 *      B   C          D     A
 *     / \     ==>    / \   / \
 *    D   E          F   G E   C
 *   / \
 *  F   G
 */
static avl_node_t* avl_node_ll_rot(avl_node_t* A)
{
    avl_node_t* B = A->left;
    A->left = B->right;
    B->right = A;

    // update heights
    A->height = MAX(HEIGHT(A->left), HEIGHT(A->right)) + 1;
    B->height = MAX(HEIGHT(B->left), HEIGHT(B->right)) + 1;

    return B;
}

/*
 * right-right rotation
 *
 *    A                 C
 *   / \              /   \
 *  B   C            A     E
 *     / \   ==>    / \   / \
 *    D   E        B   D F   G
 *       / \
 *      F   G
 */
static avl_node_t* avl_node_rr_rot(avl_node_t* A)
{
    avl_node_t* C = A->right;
    A->right = C->left;
    C->left = A;

    // update heights
    A->height = MAX(HEIGHT(A->left), HEIGHT(A->right)) + 1;
    C->height = MAX(HEIGHT(C->left), HEIGHT(C->right)) + 1;

    return C;
}

/*
 * left-right rotation
 *      A              A              E
 *     / \            / \           /   \
 *    B   C          E   C         B     A
 *   / \     ==>    / \     ==>   / \   / \
 *  D   E          B   G         D   F G   C
 *     / \        / \
 *    F   G      D   F
 */
static avl_node_t* avl_node_lr_rot(avl_node_t* A)
{
    assert(A && A->left);
    A->left = avl_node_rr_rot(A->left);
    return avl_node_ll_rot(A);
}

/*
 * right-right rotation
 *    A             A               D
 *   / \           / \            /   \
 *  B   C         B   D          A     C
 *     / \   ==>     / \   ==>  / \   / \
 *    D   E         F   C      B   F G   E
 *   / \               / \
 *  F   G             G   E
 *
 */
static avl_node_t* avl_node_rl_rot(avl_node_t* A)
{
    assert(A && A->right);
    A->right = avl_node_ll_rot(A->right);
    return avl_node_rr_rot(A);
}

/* re-balance the tree after insertion(deletion) */
static avl_node_t* avl_balance_node(avl_node_t* node, int factor, int rotate)
{
    if (node == NULL)
        return NULL;

    if (factor > 1) /* left subtree */
    {
        switch(rotate)
        {
        case ROT_LEFT: /* left-left case */
            return avl_node_ll_rot(node);
        case ROT_RIGHT: /* left-right case */
            return avl_node_lr_rot(node);
        default:
            assert(!"invalid rotate direction");
        }
    }
    else if (factor < -1)  /* right subtree */
    {
        switch(rotate)
        {
        case ROT_LEFT: /* right-left case */
            return avl_node_rl_rot(node);
        case ROT_RIGHT: /* right-right case */
            return avl_node_rr_rot(node);
        default:
            assert(!"invalid rotate direction");
        }
    }
    return node;
}

/* insert an node to tree */
static avl_node_t* avl_insert_node(avl_node_t* root, avl_key_t key, void* data)
{
    int rotate = 0;
    if (root == NULL)
    {
        return avl_new_node(key, data);
    }
    if (key < root->key)
    {
        rotate = ROT_LEFT;
        root->left = avl_insert_node(root->left, key, data);
    }
    else if (key > root->key)
    {
        rotate = ROT_RIGHT;
        root->right = avl_insert_node(root->right, key, data);
    }
    else
        assert(!"duplicated key insertion not supported");

    /* update height */
    root->height = MAX(HEIGHT(root->left), HEIGHT(root->right)) + 1;
    return avl_balance_node(root, avl_get_balance(root), rotate);
}

/* delete an node from tree */
static avl_node_t* avl_remove_node(avl_node_t* root, avl_key_t key)
{
    int factor = 0;
    int rotate = 0;
    if (root == NULL)
        return NULL;
    if (key < root->key)
    {
        root->left = avl_remove_node(root->left, key);
    }
    else if (key > root->key)
    {
        root->right = avl_remove_node(root->right, key);
    }
    else   /* this is the node to be deleted */
    {
        if ((root->left == NULL) || (root->right == NULL))
        {
            avl_node_t* node = (root->left ? root->left : root->right);
            if (node != NULL)  /* one child case */
            {
                *root = *node;
            }
            else  /* no child case */
            {
                node = root;
                root = NULL;
            }
            avl_free_node(node);
        }
        else  /* node with two children */
        {
            avl_node_t* node = avl_node_min(root->right);
            root->key = node->key;
            root->data = node->data;
            root->right = avl_remove_node(root->right, node->key);
        }
    }

    if (root == NULL)
        return NULL;
    root->height = MAX(HEIGHT(root->left), HEIGHT(root->right)) + 1;
    factor = avl_get_balance(root);
    if (factor > 1)
    {
        int left_balance = avl_get_balance(root->left);
        rotate = (left_balance >= 0 ? ROT_LEFT : ROT_RIGHT);
    }
    else if (factor < -1)
    {
        int right_balance = avl_get_balance(root->right);
        rotate = (right_balance <= 0 ? ROT_RIGHT : ROT_LEFT);
    }
    return avl_balance_node(root, factor, rotate);
}

/* destroy all nodes */
static int avl_destroy_node(avl_node_t* root)
{
    if (root->left != NULL)
        return avl_destroy_node(root->left) + 1;
    else if (root->right != NULL)
        return avl_destroy_node(root->right) + 1;
    else
    {
        avl_free_node(root);
        return 1;
    }
}

/* post-order traversal */
static int avl_postorder(avl_node_t* root, avl_key_t array[], int index)
{
    if (root != NULL)
    {
        index = avl_postorder(root->left, array, index);
        array[index++] = root->key;
        index = avl_postorder(root->right, array, index);
    }
    return index;
}

/* create an new AVL tree */
avl_tree_t* avl_create_tree()
{
    avl_tree_t* tree = (avl_tree_t*)malloc(sizeof(avl_tree_t));
    if (tree)
    {
        tree->root = NULL;
        tree->size = 0;
    }
    return tree;
}

/* destroy tree and its nodes */
int avl_destroy_tree(avl_tree_t* tree)
{
    int r = 0;
    if (tree && tree->root)
    {
        r = avl_destroy_node(tree->root);
        free(tree);
    }
    return r;
}

/* find if contains `key` */
void* avl_find(avl_tree_t* tree, avl_key_t key)
{
    avl_node_t* node = avl_node_find(tree->root, key);
    return (node != NULL ? node->data : NULL);
}

/* find the smallest key */
avl_key_t avl_find_min(avl_tree_t* tree)
{
    avl_node_t* node = avl_node_min(tree->root);
    return (node != NULL ? node->key : 0);
}

/* find the largest key */
avl_key_t avl_find_max(avl_tree_t* tree)
{
    avl_node_t* node = avl_node_max(tree->root);
    return (node != NULL ? node->key : 0);
}

/* insert data to tree */
int avl_insert(avl_tree_t* tree, avl_key_t key, void* data)
{
    if (tree)
    {
        tree->root = avl_insert_node(tree->root, key, data);
        tree->size++;
        return 1;
    }
    return 0;
}

/* delete data from tree */
int avl_delete(avl_tree_t* tree, avl_key_t key)
{
    /* find before deletion, faster in missed case but slower in hit case */
    if (tree && avl_node_find(tree->root, key))
    {
        tree->root = avl_remove_node(tree->root, key);
        tree->size--;
        return 1;
    }
    return 0;
}

/* how many item a tree has */
size_t avl_size(avl_tree_t* tree)
{
    return (tree ? tree->size : 0);
}

/* print nodes to array */
int avl_serialize(avl_tree_t* tree, avl_key_t array[], size_t len)
{
    assert(tree && array);
    if (tree->size > len)
    {
        return 0;
    }
    return avl_postorder(tree->root, array, 0);
}
