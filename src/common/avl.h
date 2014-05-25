/**
 *  @file   avl.h
 *  @author ichenq@gmail.com
 *  @date   May 26, 2014
 *  @brief  implment AVL balanced tree
 *
 */
#pragma once

#include <stddef.h>

struct avl_node;
struct avl_tree;

typedef struct avl_node avl_node_t;
typedef struct avl_tree avl_tree_t;

/* create and destroy an AVL tree */
avl_tree_t* avl_create_tree();
int         avl_destroy_tree(avl_tree_t* tree);

/* AVL tree interface */
int avl_find(avl_tree_t* tree, int key);
int avl_find_min(avl_tree_t* tree);
int avl_find_max(avl_tree_t* tree);
int avl_insert(avl_tree_t* tree, int key);
int avl_delete(avl_tree_t* tree, int key);
int avl_size(avl_tree_t* tree);

/* serialize data to an array */
int avl_serialize(avl_tree_t* tree, int array[], size_t len);
