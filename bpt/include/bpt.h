#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

/* FOR TEST RUN ONLY
#define INTERNAL_ORDER 4
#define LEAF_ORDER 4
*/

#define INTERNAL_ORDER 249
#define LEAF_ORDER 32

// Number of pages to allocate when there is no free page.
#define NEW_PAGE 5

// TYPES.

/* Type representing a queue to print the B+ tree.
 * This type helps the print_tree function to print out
 * B+ tree in BFS.
 */
typedef struct queue {
    int64_t page;
    struct queue * next;
} queue;


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
extern int order;
extern int leaf_order;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
extern queue * q;

/* The file stream is used to read from file and/or
 * write to file.
 * The file descriptor is used to fdatasync.
 */
FILE * data_file;
int fd;

// FUNCTION PROTOTYPES.

// File open.

int open_db( char * pathname );

// Getters and Setters.

int64_t get_free_page( void );
int64_t get_root( void );
int64_t get_num_pages( void );

int64_t get_parent_page(int64_t page);
int32_t get_is_leaf(int64_t page);
int32_t get_num_keys(int64_t page);
int64_t get_right_sibling(int64_t leaf);
int64_t get_leaf_key_at(int64_t leaf, int index);
char * get_leaf_value_at(int64_t leaf, int index);
int64_t get_internal_key_at(int64_t page, int index);
int64_t get_internal_value_at(int64_t page, int index);
int64_t get_next_free_page(int64_t page);

void set_free_page(int64_t page);
void set_root(int64_t page);
void set_num_pages(int64_t num);

void set_parent_page(int64_t child, int64_t parent);
void set_is_leaf(int64_t page, int32_t bit);
void set_num_keys(int64_t page, int32_t num);
void set_right_sibling(int64_t leaf, int64_t page);
void set_leaf_key_at(int64_t leaf, int index, int64_t key);
void set_leaf_value_at(int64_t leaf, int index, char * value);
void set_internal_key_at(int64_t page, int index, int64_t key);
void set_internal_value_at(int64_t page, int index, int64_t offset);
void set_next_free_page(int64_t page, int64_t next);

// Output and utility.

void license_notice( void );
void print_license( int licence_part );
void usage_1( void );
void usage_2( void );
void enqueue( int64_t new_node );
int64_t dequeue( void );
int height( void );
int path_to_root( int64_t child );
void print_leaves( void );
void print_tree( void );
void find_and_print(int64_t key);
/*
void find_and_print_range(node * root, int range1, int range2, bool verbose); 
int find_range( node * root, int key_start, int key_end, bool verbose,
        int returned_keys[], void * returned_pointers[]); 
*/
int64_t find_leaf( int64_t key );
char * find( int64_t key );
int cut( int length );

// Insertion.

int64_t make_node( void );
int64_t make_leaf( void );
int get_left_index(int64_t parent, int64_t left);
void insert_into_leaf( int64_t leaf, int64_t key, char * value );
void insert_into_leaf_after_splitting(int64_t leaf, int64_t key, char * value);
void insert_into_node(int64_t n, int left_index, int64_t key, int64_t right);
void insert_into_node_after_splitting(int64_t old_node, int left_index,
                                        int64_t key, int64_t right);
void insert_into_parent(int64_t left, int64_t key, int64_t right);
void insert_into_new_root(int64_t left, int64_t key, int64_t right);
void start_new_tree(int64_t key, char * value);
int insert( int64_t key, char * value );

// Deletion.

int get_neighbor_index( int64_t n );
int64_t remove_entry_from_node(int64_t n, int64_t key);
void adjust_root( int64_t root );
void coalesce_nodes(int64_t n, int64_t neighbor, int neighbor_index, int64_t k_prime);
void redistribute_nodes(int64_t n, int64_t neighbor, int neighbor_index, 
                            int k_prime_index, int64_t k_prime);
void delete_entry( int64_t n, int64_t key );

int delete( int64_t key );
/*
void destroy_tree_nodes(node * root);
node * destroy_tree(node * root);
*/

#endif /* __BPT_H__ */
