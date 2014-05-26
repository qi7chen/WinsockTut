#include "avl.h"
#include <malloc.h>
#include <assert.h>

/* tree node */
struct avl_node 
{
    int     datum;          /* node data */
    int     height;         /* node height */
    struct avl_node* left;  /* left child */
    struct avl_node* right; /* right child */
};

/* AVL tree */
struct avl_tree
{
    avl_node_t* root;   /* root node */
    size_t      size;   /* how many nodes */
};

#define MAX(a, b)       (a > b ? a : b)
#define HEIGHT(n)       (n != NULL ? n->height : -1)


/* allocate a new node */
static avl_node_t*  avl_new_node(int key)
{
    avl_node_t* node = (avl_node_t*)malloc(sizeof(avl_node_t));
    if (node)
    {
        node->datum = key;
        node->height = 0;   // leaf node default
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

/* free new allocated node */
static void avl_free_node(avl_node_t* node)
{
    if (node) {
        free(node);
    }
}

static avl_node_t*  avl_node_min(avl_node_t* node)
{
    if (node != NULL) {
        while (node->left != NULL)
            node = node->left;
    }
    return node;
}

static avl_node_t* avl_node_max(avl_node_t* node)
{
    if (node != NULL) {
        while (node->right != NULL)
            node = node->right;
    }
    return node;
}

/* find if contains `key` */
static avl_node_t* avl_node_find(avl_node_t* root, int key)
{
    if (root == NULL)
        return NULL;
    if (key < root->datum)
        return avl_node_find(root->left, key);
    else if (key > root->datum)
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
    A->left = avl_node_rr_rot(A->left);;    
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
    A->right = avl_node_ll_rot(A->right);
    return avl_node_rr_rot(A);
}

/* re-balance the tree after insertion(deletion) */
static avl_node_t* avl_balance_node(avl_node_t* node, int key)
{
    int factor = 0;
    if (node == NULL)
        return NULL;

    factor = HEIGHT(node->left) - HEIGHT(node->right);
    if (factor > 1) {
        if (key < node->left->datum) {      // left-left case
            return avl_node_ll_rot(node);
        }
        else if (key > node->left->datum) { // left-right case
            return avl_node_lr_rot(node);
        }
    }
    else if (factor < -1) {
        if (key > node->right->datum) {     // right-right case
            return avl_node_rr_rot(node);
        }
        else if (key < node->right->datum) { // right-left case
            return avl_node_rl_rot(node);
        }
    }
    return node;
}

/* insert an node to tree */
static avl_node_t* avl_insert_node(avl_node_t* root, int key)
{
    if (root == NULL) {
        return avl_new_node(key);
    }
    if (key < root->datum)
    {            
        root->left = avl_insert_node(root->left, key);
    }
    else if (key > root->datum)
    {
        root->right = avl_insert_node(root->right, key);
    }
    else
        assert(!"duplicate datum");

    // update height
    root->height = MAX(HEIGHT(root->left), HEIGHT(root->right)) + 1;
    return avl_balance_node(root, key);
}

/* delete an node from tree */
static avl_node_t* avl_remove_node(avl_node_t* root, int key)
{
    if (root == NULL)
        return NULL;
    if (key < root->datum) {
        root->left = avl_remove_node(root->left, key);
    }
    else if (key > root->datum) {
        root->right = avl_remove_node(root->right, key);
    }
    else // this is the node to be deleted
    {
        if ((root->left == NULL) || (root->right == NULL)) {
            avl_node_t* node = (root->left ? root->left : root->right);
            if (node != NULL) { // one child case
                root->datum = node->datum;
                root->height = node->height;
                root->left = node->left;
                root->right = node->right;
            }
            else { // no child case
                node = root;
                root = NULL;
            }
            avl_free_node(node);
        }
        else { // node with two children
            avl_node_t* node = avl_node_min(root->right);
            root->datum = node->datum;
            root->right = avl_remove_node(root->right, node->datum);
        }
    }

    if (root == NULL)
        return NULL;
    root->height = MAX(HEIGHT(root->left), HEIGHT(root->right)) + 1;
    return avl_balance_node(root, key);
}

/* destroy all nodes */
static int avl_destroy_node(avl_node_t* root)
{
    if (root->left != NULL)
        return avl_destroy_node(root->left) + 1;
    else if (root->right != NULL)
        return avl_destroy_node(root->right) + 1;
    else {
        avl_free_node(root);
        return 1;
    }
}

// post-order traversal
static int avl_postorder(avl_node_t* root, int array[], int index)
{
    if (root != NULL)
    {        
        index = avl_postorder(root->left, array, index);
        array[index++] = root->datum;
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
int avl_find(avl_tree_t* tree, int key)
{
    avl_node_t* node = avl_node_find(tree->root, key);
    return (node != NULL ? 1 : 0);
}

/* find the smallest data */
int avl_find_min(avl_tree_t* tree)
{
    avl_node_t* node = avl_node_min(tree->root);
    return (node != NULL ? node->datum : 0);
}

/* find the largest data */
int avl_find_max(avl_tree_t* tree)
{
    avl_node_t* node = avl_node_max(tree->root);
    return (node != NULL ? node->datum : 0);
}

/* insert data to tree */
int avl_insert(avl_tree_t* tree, int key)
{
    if (tree) {
        tree->root = avl_insert_node(tree->root, key);
        tree->size++;
        return 1;
    }
    return 0;
}

/* delete data from tree */
int avl_delete(avl_tree_t* tree, int key)
{
    if (tree && avl_find(tree, key) == 1)
    {
        tree->root = avl_remove_node(tree->root, key);
        tree->size--;
        return 1;
    }
    return 0;
}

/* how many item a tree has */
int avl_size(avl_tree_t* tree)
{
    return (tree ? tree->size : 0);
}

/* print nodes to array */
int avl_serialize(avl_tree_t* tree, int array[], size_t len)
{
    assert(tree && array);
    if (tree->size > len) {
        return 0;
    }
    return avl_postorder(tree->root, array, 0);
}
