/*
 *  bpt.c  
 */
#define OriginalVersion "1.14"
#define Version "1.0"
/*
 *  Disk-Based B+ Tree Implementation
 *  Copyright (C) 2017  Jeonghoon Lee  galeb716@daum.net
 *  ALL rights reserved.
 *  Copyright and license information of the original source code is
 *  described below.
 *
 *  Author:  Jeonghoon Lee
 *    galeb716@daum.net
 *  Original Date:  23 Oct 2017
 *  Last modified:  12 Nov 2017
 *
 *  ---------------------------------------------------------------------------
 *  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation 
 *  and/or other materials provided with the distribution.
 
 *  3. Neither the name of the copyright holder nor the names of its 
 *  contributors may be used to endorse or promote products derived from this 
 *  software without specific prior written permission.
 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 
 *  Author:  Amittai Aviram 
 *    http://www.amittai.com
 *    amittai.aviram@gmail.edu or afa13@columbia.edu
 *  Original Date:  26 June 2010
 *  Last modified: 17 June 2016
 *
 *  This implementation demonstrates the B+ tree data structure
 *  for educational purposes, including insertion, deletion, search, and display
 *  of the search path, the leaves, or the whole tree.
 *  ---------------------------------------------------------------------------
 *  Must be compiled with a C99-compliant C compiler such as the latest GCC.
 *
 */

#include "bpt.h"

// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
int order = INTERNAL_ORDER;
int leaf_order = LEAF_ORDER;


/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
queue * q = NULL;


// FUNCTION DEFINITIONS.

// FILE I/O

/* Open file to read and write data.
 */
int open_db( char * pathname ) {
    if ((data_file = fopen(pathname, "r+")) != NULL) {
        return 0;
    }

    if ((data_file = fopen(pathname, "w+")) == NULL) {
        return -1;
    }

    set_free_page(0x2000);
    set_root(0x1000);
    
    set_num_pages(NEW_PAGE);

/* Initializing first leaf page.
 * Parent page offset is 0, is_leaf bit is on, number keys is 0.
 * Offset of right sibling is 0.
 */
    set_parent_page(0x1000, 0);
    set_is_leaf(0x1000, 1);
    set_num_keys(0x1000, 0);
    set_right_sibling(0x1000, 0);

/* Initializing free pages.
 * The link is created to point the next free page.
 */
    int64_t off_value = 0x3000;
    fseek(data_file, 0x2000, SEEK_SET);
    for (; ftell(data_file) < 0x4000; off_value += 0x1000) {
        fwrite(&off_value, sizeof(int64_t), 1, data_file);
        fseek(data_file, 0x0ff8, SEEK_CUR);
    }
    off_value = 0;
    fwrite(&off_value, sizeof(int64_t), 1, data_file);

    fd = fileno(data_file);
    fdatasync(fd);

    return 0;
}

// OUTPUT AND UTILITIES

/* Copyright and license notice to user at startup. 
 */
void license_notice( void ) {
    printf("Disk-Based B+ Tree Version %s\n"
            "\t-- Copyright (C) 2017  Jeonghoon Lee  galeb716@daum.net\n"
            "Copyright and license information of the original code is "
            "described below.\n", Version);
    printf("=================================================================="
            "==============\n");
    printf("bpt version %s -- Copyright (C) 2010  Amittai Aviram "
            "http://www.amittai.com\n", OriginalVersion);
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n"
            "This is free software, and you are welcome to redistribute it\n"
            "under certain conditions.\n");
    printf("=================================================================="
            "==============\n\n");
}


/* First message to the user.
 */
void usage_1( void ) {
    printf("B+ Tree of Internal Order %d and Leaf Order %d.\n", order, leaf_order);
    printf("To start with input from a file of newline-delimited integers, \n"
           "start again and enter the input filename after the data filename:\n"
           "%% bpt <datafile> <inputfile>.\n");
}


/* Second message to the user.
 */
void usage_2( void ) {
    printf("Enter any of the following commands after the prompt > :\n"
    "\to <k>  -- Open existing data file <k> or create one if not existed.\n"
    "\ti <k1> <k2> -- Insert <k1> (an integer) as key and <k2> (a string) as value).\n"
    "\tf <k>  -- Find the value under key <k>.\n"
    "\tp <k>  -- Print the path from the root to key <k> and its associated "
           "value.\n"
    //"\tr <k1> <k2> -- Print the keys and values found in the range "
    //        "[<k1>, <k2>\n"
    "\td <k>  -- Delete key <k> and its associated value.\n"
    //"\tx -- Destroy the whole tree.  Start again with an empty tree of the "
    //       "same order.\n"
    "\tt -- Print the B+ tree.\n"
    "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
    "\tq -- Quit. (Or use Ctl-D.)\n"
    "\t? -- Print this help message.\n");
}


/* Helper function for printing the
 * tree out.  See print_tree.
 * Enqueue new node at the 
 */
void enqueue( int64_t new_node ) {
    queue * c, * d;
    c = (queue *) malloc(sizeof(queue));
    if (q == NULL) {
        q = c;
        q->page = new_node;
        q->next = NULL;
        
    }
    else {
        d = q;
        while(d->next != NULL) {
            d = d->next;
        }
        c->page = new_node;
        c->next = NULL;
        d->next = c;
    }
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
int64_t dequeue( void ) {
    queue * n = q;
    int64_t ret;
    q = q->next;
    ret = n->page;
    free(n);
    return ret;
}


/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */
void print_leaves( void ) {
    int i;
    int64_t c = get_root();

    if (c == 0) {
        printf("Empty tree.\n");
        return;
    }
    while (!get_is_leaf(c))
        c = get_internal_value_at(c, 0);
    while (true) {
        for (i = 0; i < get_num_keys(c); i++)
            printf("%ld ", get_leaf_key_at(c, i));

        // move to the right sibling if it exists
        if (get_right_sibling(c) != 0) {
            printf("| ");
            c = get_right_sibling(c);
        }
        else
            break;
    }
    printf("\n");
}


/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 */
int height( void ) {
    int h = 0;
    int64_t c = get_root();
    while (!get_is_leaf(c)) {
        c = get_internal_value_at(c, 0);
        h++;
    }
    return h;
}


/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
int path_to_root( int64_t child ) {
    int length = 0;
    int64_t c = child;
    while (c != get_root()) {
        c = get_parent_page(c);
        length++;
    }
    return length;
}


/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */
void print_tree( void ) {

    int64_t n = 0;
    int i = 0;
    int rank = 0;
    int new_rank = 0;
    int64_t root;

    root = get_root();

    if (root == 0) {
        printf("Empty tree.\n");
        return;
    }
    q = NULL;
    enqueue(root);
    while( q != NULL ) {
        n = dequeue();
        // not root & is the left most sibling
        if (get_parent_page(n) != 0 && n == get_internal_value_at(get_parent_page(n), 0)) {
            new_rank = path_to_root(n);
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        for (i = 0; i < get_num_keys(n); i++) {
            if (get_is_leaf(n))
                printf("%ld ", get_leaf_key_at(n, i));
            else 
                printf("%ld ", get_internal_key_at(n, i));
        }
        if (!get_is_leaf(n))
            for (i = 0; i <= get_num_keys(n); i++)
                enqueue(get_internal_value_at(n, i));

        printf("| ");
    }
    printf("\n");
}


/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
void find_and_print(int64_t key) {

    char * r = find(key);
    if (r == NULL)
        printf("Record not found under key %ld.\n", key);
    else
        printf("Record -- key %ld, value %s.\n", key, r);

    free(r);
}

// DO NOT NEED TO IMPLEMENT

/* Finds and prints the keys, pointers, and values within a range
 * of keys between key_start and key_end, including both bounds.

void find_and_print_range( node * root, int key_start, int key_end,
        bool verbose ) {
    int i;
    int array_size = key_end - key_start + 1;
    int returned_keys[array_size];
    void * returned_pointers[array_size];
    int num_found = find_range( root, key_start, key_end, verbose,
            returned_keys, returned_pointers );
    if (!num_found)
        printf("None found.\n");
    else {
        for (i = 0; i < num_found; i++)
            printf("Key: %d   Location: %lx  Value: %s\n",
                    returned_keys[i],
                    (unsigned long)returned_pointers[i],
                    ((record *)
                     returned_pointers[i])->value);
    }
}
*/

// DO NOT NEED TO IMPLEMENT

/* Finds keys and their pointers, if present, in the range specified
 * by key_start and key_end, inclusive.  Places these in the arrays
 * returned_keys and returned_pointers, and returns the number of
 * entries found.

int find_range( node * root, int key_start, int key_end, bool verbose,
        int returned_keys[], void * returned_pointers[]) {
    int i, num_found;
    num_found = 0;
    node * n = find_leaf( root, key_start, verbose );
    if (n == NULL) return 0;
    for (i = 0; i < n->num_keys && n->keys[i] < key_start; i++) ;
    if (i == n->num_keys) return 0;
    while (n != NULL) {
        for ( ; i < n->num_keys && n->keys[i] <= key_end; i++) {
            returned_keys[num_found] = n->keys[i];
            returned_pointers[num_found] = n->pointers[i];
            num_found++;
        }
        n = n->pointers[leaf_order - 1];
        i = 0;
    }
    return num_found;
}
*/

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
int64_t find_leaf( int64_t key ) {
    int i = 0;

    int64_t c = get_root();
    if (c == 0) {
        printf("Empty tree.\n");
        return c;
    }

    while (!get_is_leaf(c)) {

        i = 0;
        while (i < get_num_keys(c)) {
            if (key >= get_internal_key_at(c, i)) i++;
            else break;
        }

        c = get_internal_value_at(c, i);
    }

    return c;
}


/* Finds and returns the record to which
 * a key refers.
 */
char * find( int64_t key ) {
    int i = 0;

    int64_t c = find_leaf( key );
    if (c == 0) return NULL;
    for (i = 0; i < get_num_keys(c); i++)
        if (get_leaf_key_at(c, i) == key) break;
    if (i == get_num_keys(c)) 
        return 0;
    else
        return get_leaf_value_at(c, i);
}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}


// INSERTION


/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
int64_t make_node( void ) {

    int64_t new_node;
    int64_t num_pages, new_num_pages, next_free_page;
    new_node = get_free_page();

    //allocate 5 new free pages and set new_node again
    if (new_node == 0) {
        num_pages = get_num_pages();
        new_num_pages = num_pages + NEW_PAGE;
        next_free_page = num_pages * 0x1000;
        set_free_page(next_free_page);
        fseek(data_file, num_pages * 0x1000, SEEK_SET);
        next_free_page += 0x1000;
        for (; num_pages < new_num_pages - 1; num_pages++) {
            fwrite(&next_free_page, sizeof(int64_t), 1, data_file);
            fseek(data_file, 0x0ff8, SEEK_CUR);
            next_free_page += 0x1000;
        }
        next_free_page = 0;
        fwrite(&next_free_page, sizeof(int64_t), 1, data_file);
        set_num_pages(new_num_pages);

        new_node = get_free_page();
    }
    //rearrange free page list
    set_free_page(get_next_free_page(new_node));

    set_is_leaf(new_node, 0);
    set_num_keys(new_node, 0);
    set_parent_page(new_node, 0);

    fd = fileno(data_file);
    fdatasync(fd);

    return new_node;
}


/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
int64_t make_leaf( void ) {

    int64_t leaf = make_node();

    set_is_leaf(leaf, 1);

    fd = fileno(data_file);
    fdatasync(fd);

    return leaf;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int get_left_index(int64_t parent, int64_t left) {

    int left_index = 0;
    while (left_index <= get_num_keys(parent) && 
            get_internal_value_at(parent, left_index) != left)
        left_index++;
    return left_index;
}


/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
void insert_into_leaf( int64_t leaf, int64_t key, char * value ) {

    int i, insertion_point;

    insertion_point = 0;
    while (insertion_point < get_num_keys(leaf) && get_leaf_key_at(leaf, insertion_point) < key)
        insertion_point++;

    // shift keys and values to the right before inserting a new key.
    for (i = get_num_keys(leaf); i > insertion_point; i--) {
        set_leaf_key_at(leaf, i, get_leaf_key_at(leaf, i - 1));
        set_leaf_value_at(leaf, i, get_leaf_value_at(leaf, i - 1));
    }
    set_leaf_key_at(leaf, insertion_point, key);
    set_leaf_value_at(leaf, insertion_point, value);
    set_num_keys(leaf, get_num_keys(leaf) + 1);
    
    return;
}

// TODO : Opmtimization
/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
*/
void insert_into_leaf_after_splitting(int64_t leaf, int64_t key, char * value) {

    int64_t new_leaf;

    int64_t * temp_keys;
    char ** temp_values;
    int insertion_index, split, i, j;
    int64_t new_key;

    new_leaf = make_leaf();

    // make temporary array to copy
    temp_keys = (int64_t *) malloc( leaf_order * sizeof(int64_t) );
    if (temp_keys == NULL) {
        perror("Temporary keys array.");
        exit(EXIT_FAILURE);
    }

    temp_values = (char **) malloc( leaf_order * sizeof(char *) );
    if (temp_values == NULL) {
        perror("Temporary pointers array.");
        exit(EXIT_FAILURE);
    }

    insertion_index = 0;
    while (insertion_index < leaf_order - 1 && get_leaf_key_at(leaf, insertion_index) < key)
        insertion_index++;

    for (i = 0, j = 0; i < get_num_keys(leaf); i++, j++) {
        if (j == insertion_index) j++;
        temp_keys[j] = get_leaf_key_at(leaf, i);
        // TODO : When should the memory be freed?
        temp_values[j] = get_leaf_value_at(leaf, i);
    }

    temp_keys[insertion_index] = key;
    temp_values[insertion_index] = value;
    //strcpy(temp_values[insertion_index], value);

    set_num_keys(leaf, 0);

    split = cut(leaf_order - 1);

    for (i = 0; i < split; i++) {
        set_leaf_value_at(leaf, i, temp_values[i]);
        set_leaf_key_at(leaf, i, temp_keys[i]);
        set_num_keys(leaf, get_num_keys(leaf) + 1);
    }

    for (i = split, j = 0; i < leaf_order; i++, j++) {
        set_leaf_value_at(new_leaf, j, temp_values[i]);
        set_leaf_key_at(new_leaf, j, temp_keys[i]);
        set_num_keys(new_leaf, get_num_keys(new_leaf) + 1);
    }

    free(temp_values);
    free(temp_keys);

    // Set right sibling leaf
    set_right_sibling(new_leaf, get_right_sibling(leaf));
    set_right_sibling(leaf, new_leaf);

    // No need to implement
    // Seems that a SEGV occurs here
    /*
    // Fill in empty space of the array.
    for (i = get_num_keys(leaf); i < leaf_order - 1; i++)
        //leaf->pointers[i] = NULL;
        set_leaf_value_at(leaf, i, NULL);
    for (i = get_num_keys(new_leaf); i < leaf_order - 1; i++)
        //new_leaf->pointers[i] = NULL;
        set_leaf_value_at(new_leaf, i, NULL);
    */

    set_parent_page(new_leaf, get_parent_page(leaf));
    new_key = get_leaf_key_at(new_leaf, 0);

    insert_into_parent(leaf, new_key, new_leaf);

    return;
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node(int64_t n, int left_index, int64_t key, int64_t right) {
    int i;

    for (i = get_num_keys(n); i > left_index; i--) {
        set_internal_value_at(n, i + 1, get_internal_value_at(n, i));
        set_internal_key_at(n, i, get_internal_key_at(n, i - 1));
    }
    set_internal_value_at(n, left_index + 1, right);
    set_internal_key_at(n, left_index, key);
    set_num_keys(n, get_num_keys(n) + 1);

    return;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
void insert_into_node_after_splitting(int64_t old_node, int left_index, int64_t key, int64_t right) {

    int i, j, split;
    int64_t k_prime;
    int64_t new_node, child;
    int64_t * temp_keys, * temp_pointers;

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */

    temp_pointers = (int64_t *) malloc( (order + 1) * sizeof(int64_t) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array for splitting nodes.");
        exit(EXIT_FAILURE);
    }
    temp_keys = (int64_t *) malloc( order * sizeof(int64_t) );
    if (temp_keys == NULL) {
        perror("Temporary keys array for splitting nodes.");
        exit(EXIT_FAILURE);
    }

    for (i = 0, j = 0; i < get_num_keys(old_node) + 1; i++, j++) {
        if (j == left_index + 1) j++;
        temp_pointers[j] = get_internal_value_at(old_node, i);
    }

    for (i = 0, j = 0; i < get_num_keys(old_node); i++, j++) {
        if (j == left_index) j++;
        temp_keys[j] =  get_internal_key_at(old_node, i);
    }

    temp_pointers[left_index + 1] = right;
    temp_keys[left_index] = key;

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */ 
    split = cut(order);
    new_node = make_node();
    set_num_keys(old_node, 0);
    for (i = 0; i < split - 1; i++) {
        set_internal_value_at(old_node, i, temp_pointers[i]);
        set_internal_key_at(old_node, i, temp_keys[i]);
        set_num_keys(old_node, get_num_keys(old_node) + 1);
    }

    set_internal_value_at(old_node, i, temp_pointers[i]);
    k_prime = temp_keys[split - 1];
    for (++i, j = 0; i < order; i++, j++) {
        set_internal_value_at(new_node, j, temp_pointers[i]);
        set_internal_key_at(new_node, j, temp_keys[i]);
        set_num_keys(new_node, get_num_keys(new_node) + 1);
    }

    set_internal_value_at(new_node, j, temp_pointers[i]);
    free(temp_pointers);
    free(temp_keys);
    set_parent_page(new_node, get_parent_page(old_node));
    for (i = 0; i <= get_num_keys(new_node); i++) {
        child = get_internal_value_at(new_node, i);
        set_parent_page(child, new_node);
    }

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */
    insert_into_parent(old_node, k_prime, new_node);

    return;
}


/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
void insert_into_parent(int64_t left, int64_t key, int64_t right) {

    int left_index;
    int64_t parent;

    parent = get_parent_page(left);

    /* Case: new root. */

    if (parent == 0) {
        insert_into_new_root(left, key, right);
        return;
    }

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */

    left_index = get_left_index(parent, left);


    /* Simple case: the new key fits into the node.
     */

    if (get_num_keys(parent) < order - 1) {
        insert_into_node(parent, left_index, key, right);
        return;
    }

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    insert_into_node_after_splitting(parent, left_index, key, right);
    return;
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
void insert_into_new_root(int64_t left, int64_t key, int64_t right) {

    int64_t root = make_node();

    set_internal_key_at(root, 0, key);
    set_internal_value_at(root, 0, left);
    set_internal_value_at(root, 1, right);
    set_num_keys(root, get_num_keys(root) + 1);
    set_parent_page(root, 0);
    set_parent_page(left, root);
    set_parent_page(right, root);

    set_root(root);
}


// No need to implement; already implemented at open_db
/* First insertion:
 * start a new tree.
 */
void start_new_tree(int64_t key, char * value) {

    int64_t root = make_leaf();
    set_leaf_key_at(root, 0, key);
    set_leaf_value_at(root, 0, value);
    set_right_sibling(root, 0);
    set_parent_page(root, 0);
    set_num_keys(root, get_num_keys(root) + 1);

    set_root(root);
}


/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 *
 * @return 0    if insertion successed
 *         -1   if value with given key is not found
 */
int insert( int64_t key, char * value ) {
    
    int64_t leaf;
    
    /* Ignore duplicate.
     */

    if (find(key) != NULL) {
        return -1;
    }

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */
    if (get_root() == 0) {
        start_new_tree(key, value);
        return 0;
    }


    /* Case: the tree already exists.
     * (Rest of function body.)
     */

    leaf = find_leaf(key);

    /* Case: leaf has room for key and pointer.
     */
    if (get_num_keys(leaf) < leaf_order - 1) {
        insert_into_leaf(leaf, key, value);

        fd = fileno(data_file);
        fdatasync(fd);

        return 0;
    }

    /* Case:  leaf must be split.
     */
    insert_into_leaf_after_splitting(leaf, key, value);

    fd = fileno(data_file);
    fdatasync(fd);

    return 0;
}



// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */
int get_neighbor_index( int64_t n ) {

    int i;

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */
    for (i = 0; i <= get_num_keys(get_parent_page(n)); i++)
        if (get_internal_value_at(get_parent_page(n), i) == n)
            return i - 1;

    // Error state.
    printf("Search for nonexistent pointer to node in parent.\n");
    printf("Node:  #%ld\n", (unsigned long)n);
    exit(EXIT_FAILURE);
}


int64_t remove_entry_from_node(int64_t n, int64_t key) {

    int i, num_pointers;

    // Remove the key and shift other keys accordingly.
    i = 0;
    if (get_is_leaf(n)) {
        while (get_leaf_key_at(n, i) != key)
            i++;
        for (++i; i < get_num_keys(n); i++) {
            set_leaf_key_at(n, i - 1, get_leaf_key_at(n, i));
            set_leaf_value_at(n, i - 1, get_leaf_value_at(n, i));
        }
    }
    else {
        while (get_internal_key_at(n, i) != key)
            i++;
        for (++i; i < get_num_keys(n); i++) {
            set_internal_key_at(n, i - 1, get_internal_key_at(n, i));
            set_internal_value_at(n, i, get_internal_value_at(n, i + 1));
        }
        //set_internal_value_at(n, i - 1, get_internal_value_at(n, i));
    }

    // One key fewer.
    set_num_keys(n, get_num_keys(n) - 1);

    /*
    // Is it necessary to implement?
    // TODO : optimize
    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    if (get_is_leaf(n) n->is_leaf)
        for (i = n->num_keys; i < leaf_order - 1; i++)
            n->pointers[i] = NULL;
    else
        for (i = n->num_keys + 1; i < order; i++)
            n->pointers[i] = NULL;
    */

    return n;
}


void adjust_root( int64_t root ) {

    int64_t new_root;

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (get_num_keys(root) > 0)
        return;

    /* Case: empty root. 
     */

    // If it has a child, promote 
    // the first (only) child
    // as the new root.

    if (!get_is_leaf(root)) {
        new_root = get_internal_value_at(root, 0);
        set_parent_page(new_root, 0);
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.

    else
        new_root = 0;

    set_root(new_root);

    set_next_free_page(root, get_free_page());
    set_free_page(root);

}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */
void coalesce_nodes(int64_t n, int64_t neighbor, int neighbor_index, int64_t k_prime) {

    int i, j, neighbor_insertion_index;
    int32_t n_end;
    int64_t tmp;

    /* Swap neighbor with node if node is on the
     * extreme left and neighbor is to its right.
     */

    if (neighbor_index == -1) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
    }

    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     */

    neighbor_insertion_index = get_num_keys(neighbor);

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */

    if (!get_is_leaf(n)) {

        /* Append k_prime.
         */

        set_internal_key_at(neighbor, neighbor_insertion_index, k_prime);
        set_num_keys(neighbor, get_num_keys(neighbor) + 1);


        n_end = get_num_keys(n);

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            set_internal_key_at(neighbor, i, get_internal_key_at(n, j));
            set_internal_value_at(neighbor, i, get_internal_value_at(n, j));
            set_num_keys(neighbor, get_num_keys(neighbor) + 1);
            set_num_keys(n, get_num_keys(n) - 1);
        }

        /* The number of pointers is always
         * one more than the number of keys.
         */

        set_internal_value_at(neighbor, i, get_internal_value_at(n, j));

        /* All children must now point up to the same parent.
         */
        // TODO : optimize
        for (i = 0; i < get_num_keys(neighbor) + 1; i++) {
            tmp = get_internal_value_at(neighbor, i);
            set_parent_page(tmp, neighbor);
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.
     */

    else {
        for (i = neighbor_insertion_index, j = 0; j < get_num_keys(n); i++, j++) {
            set_leaf_key_at(neighbor, i, get_leaf_key_at(n, j));
            set_leaf_value_at(neighbor, i, get_leaf_value_at(n, j));
            set_num_keys(neighbor, get_num_keys(neighbor) + 1);
        }
        set_right_sibling(neighbor, get_right_sibling(n));
    }

    delete_entry(get_parent_page(n), k_prime);
    
    set_next_free_page(n, get_free_page());
    set_free_page(n);
}


/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */
void redistribute_nodes(int64_t n, int64_t neighbor, int neighbor_index, 
                            int k_prime_index, int64_t k_prime) { 

    int i;
    int64_t tmp;

    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */

    if (neighbor_index != -1) {
        if (!get_is_leaf(n)) {
            //n->pointers[n->num_keys + 1] = n->pointers[n->num_keys];
            set_internal_value_at( n, get_num_keys(n) + 1, get_internal_value_at(n, get_num_keys(n)) );
            for (i = get_num_keys(n); i > 0; i--) {
                set_internal_key_at(n, i, get_internal_key_at(n, i - 1));
                set_internal_value_at(n, i, get_internal_value_at(n, i - 1));

                set_internal_value_at( n, 0, get_internal_value_at(neighbor, get_num_keys(neighbor)) );
                tmp = get_internal_value_at(n, 0);
                set_parent_page(tmp, n);
                set_internal_value_at(neighbor, get_num_keys(neighbor), 0);
                set_internal_key_at(n, 0, k_prime);
                set_internal_key_at( get_parent_page(n), k_prime_index, 
                                    get_internal_key_at(neighbor, get_num_keys(neighbor) - 1) );
            }
        }
        else {
            for (i = get_num_keys(n); i > 0; i--) {
                set_leaf_key_at(n, i, get_leaf_key_at(n, i - 1));
                set_leaf_value_at(n, i, get_leaf_value_at(n, i - 1));

                set_leaf_value_at( n, 0, get_leaf_value_at(neighbor, get_num_keys(neighbor) - 1) );
                set_leaf_value_at(neighbor, get_num_keys(neighbor) - 1, 0);
                set_leaf_key_at( n, 0, get_leaf_key_at(neighbor, get_num_keys(neighbor) - 1) );
                set_internal_key_at(get_parent_page(n), k_prime_index, get_leaf_key_at(n, 0));
            }
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */

    else {  
        if (get_is_leaf(n)) {
            set_leaf_key_at(n, get_num_keys(n), get_leaf_key_at(neighbor, 0));
            set_leaf_value_at(n, get_num_keys(n), get_leaf_value_at(neighbor, 0));
            set_internal_key_at(get_parent_page(n), k_prime_index, get_leaf_key_at(neighbor, 1));

            for (i = 0; i < get_num_keys(neighbor) - 1; i++) {
                set_leaf_key_at(neighbor, i, get_leaf_key_at(neighbor, i + 1));
                set_leaf_value_at(neighbor, i, get_leaf_value_at(neighbor, i + 1));
            }
        }
        else {
            set_internal_key_at(n, get_num_keys(n), k_prime);
            set_internal_value_at(n, get_num_keys(n) + 1, get_internal_value_at(neighbor, 0));
            tmp = get_internal_value_at(n, get_num_keys(n) + 1);
            set_parent_page(tmp, n);
            set_internal_key_at(get_parent_page(n), k_prime_index, get_internal_key_at(neighbor, 0));

            for (i = 0; i < get_num_keys(neighbor) - 1; i++) {
                set_internal_key_at(neighbor, i, get_internal_key_at(neighbor, i + 1));
                set_internal_value_at(neighbor, i, get_internal_value_at(neighbor, i + 1));
            }
            set_internal_value_at(neighbor, i, get_internal_value_at(neighbor, i + 1));
        }
    }

    /* n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     */
    set_num_keys(n, get_num_keys(n) + 1);
    set_num_keys(neighbor, get_num_keys(neighbor) - 1);
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
void delete_entry( int64_t n, int64_t key ) {

    int min_keys;
    int64_t neighbor;
    int neighbor_index, k_prime_index;
    int64_t k_prime;
    int capacity;

    // Remove key and pointer from node.

    n = remove_entry_from_node(n, key);

    /* Case:  deletion from the root. 
     */

    if (n == get_root()) {
        adjust_root(n);
        return;
    }


    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */

    min_keys = get_is_leaf(n) ? cut(leaf_order - 1) : cut(order) - 1;

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

    if (get_num_keys(n) >= min_keys)
        return;

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

    /* Find the appropriate neighbor node with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to node n and the pointer
     * to the neighbor.
     */

    neighbor_index = get_neighbor_index( n );
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = get_internal_key_at(get_parent_page(n), k_prime_index);
    neighbor = neighbor_index == -1 ? get_internal_value_at(get_parent_page(n), 1) : 
        get_internal_value_at(get_parent_page(n), neighbor_index);

    capacity = get_is_leaf(n) ? leaf_order : order - 1;

    /* Coalescence. */

    if (get_num_keys(neighbor) + get_num_keys(n) < capacity) {
        coalesce_nodes(n, neighbor, neighbor_index, k_prime);
        return;
    }

    /* Redistribution. */

    else {
        redistribute_nodes(n, neighbor, neighbor_index, k_prime_index, k_prime);
        return;
    }
}



/* Master deletion function.
 */
int delete( int64_t key ) {

    int64_t key_leaf;
    char * key_record;

    key_record = find(key);
    key_leaf = find_leaf(key);
    if (key_record != NULL && key_leaf != 0) {
        delete_entry(key_leaf, key);
        free(key_record);
    }
    // The value with key is not found.
    if (key_record == NULL) {
        return -1;
    }

    fd = fileno(data_file);
    fdatasync(fd);

    return 0;
}


// TODO : destroy tree in disk efficiently
/*
void destroy_tree_nodes(int64_t root) {
    int i;
    if (get_is_leaf(root))
        for (i = 0; i < get_num_keys(root); i++)
            free(root->pointers[i]);
    else
        for (i = 0; i < get_num_keys(root) + 1; i++)
            destroy_tree_nodes(get_internal_value_at(root, i));
    
    free(root->pointers);
    free(root->keys);
    free(root);
}


node * destroy_tree(node * root) {
    destroy_tree_nodes(root);
    return NULL;
}
*/


// GETTERS & SETTERS

// Getters of header page

int64_t get_free_page( void ) {
    int64_t page;

    fseek(data_file, 0, SEEK_SET);
    fread(&page, sizeof(int64_t), 1, data_file);
    return page;
}

int64_t get_root( void ) {
    int64_t page;

    fseek(data_file, 8, SEEK_SET);
    fread(&page, sizeof(int64_t), 1, data_file);
    return page;
}

int64_t get_num_pages( void ) {
    int64_t num;
    
    fseek(data_file, 16, SEEK_SET);
    fread(&num, sizeof(int64_t), 1, data_file);
    return num;
}

// Getters of node pages

int64_t get_parent_page(int64_t page) {
    int64_t parent;

    fseek(data_file, page, SEEK_SET);
    fread(&parent, sizeof(int64_t), 1, data_file);
    return parent;
}

int32_t get_is_leaf (int64_t leaf) {
    int32_t bit;

    fseek(data_file, leaf + 8, SEEK_SET);
    fread(&bit, sizeof(int32_t), 1, data_file);
    return bit;
}

int32_t get_num_keys(int64_t page) {
    int32_t num;

    fseek(data_file, page + 12, SEEK_SET);
    fread(&num, sizeof(int32_t), 1, data_file);
    return num;
}

int64_t get_right_sibling(int64_t leaf) {
    int64_t page;

    fseek(data_file, leaf + 120, SEEK_SET);
    fread(&page, sizeof(int64_t), 1, data_file);
    return page;
}

int64_t get_leaf_key_at(int64_t leaf, int index) {
    int64_t key;

    fseek(data_file, leaf + ((index + 1) * 128), SEEK_SET);
    fread(&key, sizeof(int64_t), 1, data_file);
    return key;
}

char * get_leaf_value_at(int64_t leaf, int index) {
    char * value;
    value = (char *) malloc(sizeof(char) * 120);

    fseek(data_file, leaf + ((index + 1) * 128) + 8, SEEK_SET);
    fread(value, sizeof(char), 120, data_file);
    return value;
}

int64_t get_internal_key_at(int64_t page, int index) {
    int64_t key;

    fseek(data_file, page + (index * 16) + 128, SEEK_SET);
    fread(&key, sizeof(int64_t), 1, data_file);
    return key;
}

int64_t get_internal_value_at(int64_t page, int index) {
    int64_t value;

    fseek(data_file, page + (index * 16) + 120, SEEK_SET);
    fread(&value, sizeof(int64_t), 1, data_file);
    return value;
}

int64_t get_next_free_page(int64_t page) {
    int64_t next;

    fseek(data_file, page, SEEK_SET);
    fread(&next, sizeof(int64_t), 1, data_file);
    return next;
}

// Setters

// Setters of header page

void set_free_page(int64_t page) {
    fseek(data_file, 0, SEEK_SET);
    fwrite(&page, sizeof(int64_t), 1, data_file);
}

void set_root(int64_t page) {
    fseek(data_file, 8, SEEK_SET);
    fwrite(&page, sizeof(int64_t), 1, data_file);
}

void set_num_pages(int64_t num) {
    fseek(data_file, 16, SEEK_SET);
    fwrite(&num, sizeof(int64_t), 1, data_file);
}

// Setters of node pages

void set_parent_page(int64_t child, int64_t parent) {
    fseek(data_file, child, SEEK_SET);
    fwrite(&parent, sizeof(int64_t), 1, data_file);
}

void set_is_leaf(int64_t page, int32_t bit) {
    fseek(data_file, page + 8, SEEK_SET);
    fwrite(&bit, sizeof(int32_t), 1, data_file);
}

void set_num_keys(int64_t page, int32_t num) {
    fseek(data_file, page + 12, SEEK_SET);
    fwrite(&num, sizeof(int32_t), 1, data_file);
}

void set_right_sibling(int64_t leaf, int64_t page) {
    fseek(data_file, leaf + 120, SEEK_SET);
    fwrite(&page, sizeof(int64_t), 1, data_file);
}

void set_leaf_key_at(int64_t leaf, int index, int64_t key) {
    fseek(data_file, leaf + ((index + 1) * 128), SEEK_SET);
    fwrite(&key, sizeof(int64_t), 1, data_file);
}

void set_leaf_value_at(int64_t leaf, int index, char * value) {
    fseek(data_file, leaf + ((index + 1) * 128) + 8, SEEK_SET);
    fwrite(value, sizeof(char), 120, data_file);
}

void set_internal_key_at(int64_t page, int index, int64_t key) {
    fseek(data_file, page + (index * 16) + 128, SEEK_SET);
    fwrite(&key, sizeof(int64_t), 1, data_file);
}

void set_internal_value_at(int64_t page, int index, int64_t offset) {
    fseek(data_file, page + (index * 16) + 120, SEEK_SET);
    fwrite(&offset, sizeof(int64_t), 1, data_file);
}

void set_next_free_page(int64_t page, int64_t next) {
    fseek(data_file, page, SEEK_SET);
    fwrite(&next, sizeof(int64_t), 1, data_file);
}

