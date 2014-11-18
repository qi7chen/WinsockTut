/**
 * Copyright (C) 2012-2014 ichenq@gmail.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

struct avl_node;
struct avl_tree;

typedef struct avl_node avl_node_t;
typedef struct avl_tree avl_tree_t;

typedef int32_t   avl_key_t;

/************************************************************************/
/* AVL tree interface                                                   */
/************************************************************************/

/* create and destroy an AVL tree */
avl_tree_t* avl_create_tree();
int         avl_destroy_tree(avl_tree_t* tree);

/* searching */
void*       avl_find(avl_tree_t* tree, avl_key_t key);
avl_key_t   avl_find_min(avl_tree_t* tree);
avl_key_t   avl_find_max(avl_tree_t* tree);

/* insertion and deletion */
int avl_insert(avl_tree_t* tree, avl_key_t key, void* data);
int avl_delete(avl_tree_t* tree, avl_key_t key);
int avl_size(avl_tree_t* tree);

/* serialize data to an array */
int avl_serialize(avl_tree_t* tree, avl_key_t array[], size_t len);
