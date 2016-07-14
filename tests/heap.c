/* gnx -- algorithms for graphs and networks
 * Copyright (C) 2016 Minh Van Nguyen <mvngu.name AT gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/* Test the functions in the module src/heap.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>

#include <gnx.h>

#include "constant.h"

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

static void add_nodes(GnxHeap *heap,
                      const unsigned int id[],
                      const double key[],
                      const unsigned int *size);

/* add node */
static void add_no_memory(void);
static void add_one(void);
static void add_random(void);
static void add_resize(void);
static void add_resize_no_memory(void);

/* decrease key */
static void decrease_constant_size(void);
static void decrease_empty(void);
static void decrease_inbetween_node(void);
static void decrease_last_node(void);
static void decrease_non_member(void);
static void decrease_random(void);
static void decrease_root_node(void);
static void decrease_same(void);

/* has */
static void has_empty(void);

/* key of a node */
static void key_empty(void);
static void key_non_node(void);
static void key_one_node(void);
static void key_random_nodes(void);

/* new: create and destroy */
static void new_heap(void);
static void new_no_memory(void);

/* pop node */
static void pop_duplicate(void);
static void pop_empty(void);
static void pop_more(void);
static void pop_one(void);

/**************************************************************************
 * helper functions
 *************************************************************************/

/* Insert a bunch of nodes into a heap.
 *
 * - heap We want to insert nodes into this heap.
 * - id An array of node IDs.
 * - key The key corresponding to each node ID in id.
 * - size How many nodes to insert into the given heap.
 */
static void
add_nodes(GnxHeap *heap,
          const unsigned int id[],
          const double key[],
          const unsigned int *size)
{
    unsigned int i;

    for (i = 0; i < *size; i++)
        assert(gnx_heap_add(heap, &id[i], &key[i]));
}

/**************************************************************************
 * add node
 *************************************************************************/

static void
add(void)
{
    add_no_memory();
    add_one();
    add_random();
    add_resize();
    add_resize_no_memory();
}

/* Add a node under low-memory scenarios. */
static void
add_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxHeap *heap = gnx_init_heap();
    int alloc_size;
    const double key = 3.14159;
    const unsigned int v = 0;

    /* Cannot allocate memory to hold the node index and the node's key. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(0 == heap->size);
    assert(!gnx_heap_add(heap, &v, &key));
    assert(ENOMEM == errno);
    assert(0 == heap->size);

    /* Cannot allocate memory for the node ID. */
    alloc_size++;
    gnx_alloc_set_limit(alloc_size);
    assert(0 == heap->size);
    assert(!gnx_heap_add(heap, &v, &key));
    assert(ENOMEM == errno);
    assert(0 == heap->size);

    gnx_destroy_heap(heap);
    gnx_alloc_reset_limit();
#endif
}

/* Add one node to an empty heap. */
static void
add_one(void)
{
    GnxHeap *heap;
    const double key = (double)g_random_double();
    const unsigned int v = (unsigned int)g_random_int();

    heap = gnx_init_heap();
    assert(0 == heap->size);
    assert(gnx_heap_add(heap, &v, &key));
    assert(1 == heap->size);

    gnx_destroy_heap(heap);
}

/* Add a random number of nodes to a heap. */
static void
add_random(void)
{
    double key;
    GnxHeap *heap;
    unsigned int i, v;
    const unsigned int size = (unsigned int)g_random_int_range(2, 51);

    heap = gnx_init_heap();
    assert(heap);

    for (i = 0; i < size; i++) {
        /* Ensure that we generate unique node IDs. */
        do {
            v = (unsigned int)g_random_int();
        } while (gnx_heap_has(heap, &v));

        key = (double)g_random_double();
        assert(gnx_heap_add(heap, &v, &key));
    }
    assert(size == heap->size);

    gnx_destroy_heap(heap);
}

/* Add enough nodes to trigger a resize of a heap. */
static void
add_resize(void)
{
    double key;
    GnxHeap *heap;
    unsigned int i, v;
    const unsigned int size = GNX_DEFAULT_ALLOC_SIZE;

    heap = gnx_init_heap();
    assert(0 == heap->size);
    assert(size == heap->capacity);

    for (i = 0; i < size; i++) {
        /* Ensure that we generate unique node IDs. */
        do {
            v = (unsigned int)g_random_int();
        } while (gnx_heap_has(heap, &v));

        key = (double)g_random_double();
        assert(gnx_heap_add(heap, &v, &key));
    }
    assert(size == heap->size);
    assert(size == heap->capacity);

    /* Another pair of node/key to trigger a resize of the heap.  Ensure that
     * we generate a unique node ID.  After the resize, the heap size has
     * increased by one, but the heap capacity has doubled.
     */
    do {
        v = (unsigned int)g_random_int();
    } while (gnx_heap_has(heap, &v));
    key = (double)g_random_double();
    assert(gnx_heap_add(heap, &v, &key));
    assert((size + 1) == heap->size);
    assert((2 * size) == heap->capacity);

    gnx_destroy_heap(heap);
}

/* Test the resize of a heap under low-memory scenarios. */
static void
add_resize_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    double key;
    GnxHeap *heap;
    int alloc_size;
    unsigned int i, v;
    const unsigned int size = GNX_DEFAULT_ALLOC_SIZE;

    heap = gnx_init_heap();
    assert(0 == heap->size);
    assert(size == heap->capacity);

    for (i = 0; i < size; i++) {
        /* Ensure that we generate unique node IDs. */
        do {
            v = (unsigned int)g_random_int();
        } while (gnx_heap_has(heap, &v));

        key = (double)g_random_double();
        assert(gnx_heap_add(heap, &v, &key));
    }
    assert(size == heap->size);
    assert(size == heap->capacity);

    /* Another pair of node/key to trigger a resize of the heap.  Ensure that
     * we generate a unique node ID.
     */
    do {
        v = (unsigned int)g_random_int();
    } while (gnx_heap_has(heap, &v));
    key = (double)g_random_double();

    /* Cannot allocate memory for the new array of heap nodes. */
    alloc_size = GNX_ALLOC_HEAP_NODE_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_heap_add(heap, &v, &key));
    assert(ENOMEM == errno);
    assert(size == heap->size);
    assert(size == heap->capacity);

    gnx_destroy_heap(heap);
    gnx_alloc_reset_limit();
#endif
}

/**************************************************************************
 * decrease key
 *************************************************************************/

static void
decrease(void)
{
    decrease_constant_size();
    decrease_empty();
    decrease_inbetween_node();
    decrease_last_node();
    decrease_non_member();
    decrease_random();
    decrease_root_node();
    decrease_same();
}

/* Decreasing the key of a node must not change the number of nodes in a
 * minimum binary heap.
 */
static void
decrease_constant_size(void)
{
    GnxHeap *heap;
    const double k = 0.6;
    const unsigned int id[5] = {0,     1,   2,   3,   4};
    const double key[5]      = {0.5, 1.4, 2.2, 3.1, 4.6};
    const unsigned int size = 5;
    const unsigned int v = 3;

    heap = gnx_init_heap();
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);

    assert(gnx_heap_has(heap, &v));
    assert(gnx_heap_decrease_key(heap, &v, &k));
    assert(gnx_heap_has(heap, &v));
    assert(size == heap->size);

    gnx_destroy_heap(heap);
}

/* Cannot decrease a key if the heap is empty. */
static void
decrease_empty(void)
{
    GnxHeap *heap;
    const double key = (double)g_random_double();
    const unsigned int v = (unsigned int)g_random_int();

    heap = gnx_init_heap();
    assert(0 == heap->size);
    assert(!gnx_heap_decrease_key(heap, &v, &key));
    assert(0 == heap->size);

    gnx_destroy_heap(heap);
}

/* After the key of a node is decreased, we have changed the order in which
 * nodes will be popped from the heap.
 */
static void
decrease_inbetween_node(void)
{
    GnxHeap *heap;
    unsigned int i, w;
    const double k = 4;
    const unsigned int id[5] = {2, 3, 5, 6, 7};
    const double key[5]      = {2, 3, 5, 6, 7};
    const unsigned int known[5] = {2, 3, 5, 6, 7};
    const unsigned int known_new[5] = {2, 3, 6, 5, 7};
    const unsigned int size = 5;
    const unsigned int v = 6;

    /* The original heap with no change in key. */
    heap = gnx_init_heap();
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);
    i = 0;
    while (heap->size) {
        assert(gnx_heap_pop(heap, &w));
        assert(known[i] == w);
        i++;
    }
    gnx_destroy_heap(heap);

    /* The same heap.  This time, we decrease the key of node 6 from the key
     * value of 6 to the new key of 4.
     */
    heap = gnx_init_heap();
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);
    assert(gnx_heap_has(heap, &v));
    assert(gnx_heap_decrease_key(heap, &v, &k));
    i = 0;
    while (heap->size) {
        assert(gnx_heap_pop(heap, &w));
        assert(known_new[i] == w);
        i++;
    }
    gnx_destroy_heap(heap);
}

/* After the key of a node is decreased, we have changed the order in which
 * nodes will be popped from the heap.
 */
static void
decrease_last_node(void)
{
    GnxHeap *heap;
    unsigned int i, w;
    const double k = 1;
    const unsigned int id[4] = {2, 3, 4, 5};
    const double key[4]      = {2, 3, 4, 5};
    const unsigned int known[4] = {2, 3, 4, 5};
    const unsigned int known_new[4] = {5, 2, 3, 4};
    const unsigned int size = 4;
    const unsigned int v = 5;

    /* The original heap with no change in key. */
    heap = gnx_init_heap();
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);
    i = 0;
    while (heap->size) {
        assert(gnx_heap_pop(heap, &w));
        assert(known[i] == w);
        i++;
    }
    gnx_destroy_heap(heap);

    /* The same heap.  This time, we decrease the key of node 5 (the last node)
     * from the key value of 5 to the new key of 1.
     */
    heap = gnx_init_heap();
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);
    assert(gnx_heap_has(heap, &v));
    assert(gnx_heap_decrease_key(heap, &v, &k));
    i = 0;
    while (heap->size) {
        assert(gnx_heap_pop(heap, &w));
        assert(known_new[i] == w);
        i++;
    }
    gnx_destroy_heap(heap);
}

/* Cannot decrease a key if the corresponding node is not in the heap.
 */
static void
decrease_non_member(void)
{
    GnxHeap *heap;
    const double k = 3.35988;
    const unsigned int id[3] = {0,       1,       2};
    const double key[3]      = {0.57721, 1.41421, 2.29558};
    const unsigned int size = 3;
    const unsigned int v = 3;

    heap = gnx_init_heap();
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);

    assert(!gnx_heap_has(heap, &v));
    assert(!gnx_heap_decrease_key(heap, &v, &k));
    assert(size == heap->size);
    assert(!gnx_heap_has(heap, &v));

    gnx_destroy_heap(heap);
}

/* Randomly decrease the key of each node. */
static void
decrease_random(void)
{
    double key;
    GnxHeap *heap;
    int decrease;
    unsigned int v;
    const double high = 10.0;
    const double low = 2.0;
    const unsigned int size = 1024;

    /* Add a bunch of nodes to an empty heap. */
    heap = gnx_init_heap();
    for (v = 0; v < size; v++) {
        key = (double)g_random_double_range(low, high);
        assert(gnx_heap_add(heap, &v, &key));
    }
    assert(size == heap->size);

    /* Randomly decrease the key of each node. */
    for (v = 0; v < size; v++) {
        decrease = (int)g_random_boolean();
        if (decrease) {
            key = (double)g_random_double();
            assert(gnx_heap_decrease_key(heap, &v, &key));
        }
    }
    assert(size == heap->size);

    gnx_destroy_heap(heap);
}

/* Decreasing the key of the root node has no effect on the order in which
 * nodes will be popped from the heap.
 */
static void
decrease_root_node(void)
{
    GnxHeap *heap;
    unsigned int i, w;
    const double k = 1;
    const unsigned int id[4] = {2, 3, 4, 5};
    const double key[4]      = {2, 3, 4, 5};
    const unsigned int known[4] = {2, 3, 4, 5};
    const unsigned int size = 4;
    const unsigned int v = 2;

    /* The original heap with no change in key. */
    heap = gnx_init_heap();
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);
    i = 0;
    while (heap->size) {
        assert(gnx_heap_pop(heap, &w));
        assert(known[i] == w);
        i++;
    }
    gnx_destroy_heap(heap);

    /* The same heap.  This time, we decrease the key of node 2 (the root node)
     * from the key value of 2 to the new key of 1.  The change in key value
     * should not change the order in which nodes are popped from the heap.
     */
    heap = gnx_init_heap();
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);
    assert(gnx_heap_has(heap, &v));
    assert(gnx_heap_decrease_key(heap, &v, &k));
    i = 0;
    while (heap->size) {
        assert(gnx_heap_pop(heap, &w));
        assert(known[i] == w);
        i++;
    }
    gnx_destroy_heap(heap);
}

/* Cannot decrease a key if the new key compares equal to the current key. */
static void
decrease_same(void)
{
    GnxHeap *heap;
    const double key = 2.13157;
    const unsigned int v = 11;

    heap = gnx_init_heap();
    assert(gnx_heap_add(heap, &v, &key));
    assert(1 == heap->size);
    assert(!gnx_heap_decrease_key(heap, &v, &key));
    assert(EINVAL == errno);
    assert(1 == heap->size);

    gnx_destroy_heap(heap);
}

/**************************************************************************
 * has
 *************************************************************************/

static void
has(void)
{
    has_empty();
}

/* Query for a node in an empty heap. */
static void
has_empty(void)
{
    GnxHeap *heap;
    const unsigned int v = (unsigned int)g_random_int();

    heap = gnx_init_heap();
    assert(0 == heap->size);
    assert(!gnx_heap_has(heap, &v));

    gnx_destroy_heap(heap);
}

/**************************************************************************
 * key of a node
 *************************************************************************/

static void
key(void)
{
    key_empty();
    key_non_node();
    key_one_node();
    key_random_nodes();
}

/* Query a node key from an empty heap. */
static void
key_empty(void)
{
    double key;
    GnxHeap *heap;
    const unsigned int v = (unsigned int)g_random_int();

    heap = gnx_init_heap();
    assert(0 == heap->size);
    assert(!gnx_heap_key(heap, &v, &key));
    assert(0 == heap->size);

    gnx_destroy_heap(heap);
}

/* Query the key of a node that is not in the heap. */
static void
key_non_node(void)
{
    double key;
    GnxHeap *heap;
    unsigned int i, v;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    heap = gnx_init_heap();
    for (i = 0; i < size; i++) {
        /* Ensure that we have a unique node ID. */
        do {
            v = (unsigned int)g_random_int();
        } while (gnx_heap_has(heap, &v));

        key = (double)g_random_double();
        assert(gnx_heap_add(heap, &v, &key));
    }
    assert(size == heap->size);

    /* Generate a node ID that is not in the heap. */
    do {
        v = (unsigned int)g_random_int();
    } while (gnx_heap_has(heap, &v));

    assert(size == heap->size);
    assert(!gnx_heap_has(heap, &v));
    assert(!gnx_heap_key(heap, &v, &key));
    assert(size == heap->size);

    gnx_destroy_heap(heap);
}

/* Query a node key from a heap that has exactly one node. */
static void
key_one_node(void)
{
    double k;
    GnxHeap *heap;
    const unsigned int v = 0;
    const double key = 3.14159;

    heap = gnx_init_heap();
    assert(gnx_heap_add(heap, &v, &key));
    assert(1 == heap->size);

    assert(gnx_heap_key(heap, &v, &k));
    assert(gnx_double_cmp_eq(&key, &k));

    gnx_destroy_heap(heap);
}

/* Query a node key from a heap that has nodes with random keys. */
static void
key_random_nodes(void)
{
    double k, key[100];
    GnxHeap *heap;
    unsigned int i;
    const unsigned int size = 100;

    heap = gnx_init_heap();

    for (i = 0; i < size; i++) {
        key[i] = (double)g_random_double_range(0.0, 200.0);
        assert(gnx_heap_add(heap, &i, &key[i]));
    }
    assert(size == heap->size);

    i = (unsigned int)g_random_int_range(0, (int)size);
    assert(gnx_heap_key(heap, &i, &k));
    assert(gnx_double_cmp_eq(&k, &key[i]));

    gnx_destroy_heap(heap);
}

/**************************************************************************
 * new: create and destroy
 *************************************************************************/

static void
new(void)
{
    new_heap();
    new_no_memory();
}

/* Initialize a heap. */
static void
new_heap(void)
{
    GnxHeap *heap;

    heap = gnx_init_heap();
    assert(0 == heap->size);
    gnx_destroy_heap(heap);

    gnx_destroy_heap(NULL);
}

/* Test the function gnx_init_heap() under low-memory scenarios. */
static void
new_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    /* Cannot allocate memory for the heap. */
    gnx_alloc_set_limit(0);
    assert(!gnx_init_heap());
    assert(ENOMEM == errno);

    gnx_alloc_reset_limit();
#endif
}

/**************************************************************************
 * pop node
 *************************************************************************/

static void
pop(void)
{
    pop_duplicate();
    pop_empty();
    pop_more();
    pop_one();
}

/* Pop from a heap that has two nodes with the same key.  The popped node IDs
 * are compared with the known order in which nodes will be popped.
 */
static void
pop_duplicate(void)
{
    GnxHeap *heap;
    unsigned int i, k, v;
    const unsigned int id[4] = {2,    3,    5,    19};
    const double key[4]      = {1.20, 3.14, 2.68, 2.68};
    const unsigned int known[4] = {2, 5, 19, 3};
    const unsigned int size = 4;

    heap = gnx_init_heap();
    assert(heap);
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);

    /* Ensure that we always remove from the heap the node with minimum key.
     * With the above ID/key pairs, the order in which nodes are popped from
     * the heap is specified by the array 'known'.
     */
    k = size;
    for (i = 0; i < size; i++) {
        k--;
        assert(gnx_heap_pop(heap, &v));
        assert(v == known[i]);
        assert(k == heap->size);
    }

    gnx_destroy_heap(heap);
}

/* Pop an empty heap. */
static void
pop_empty(void)
{
    GnxHeap *heap = gnx_init_heap();
    assert(heap);
    assert(0 == heap->size);
    assert(!gnx_heap_pop(heap, NULL));
    assert(0 == heap->size);
    gnx_destroy_heap(heap);
}

/* Pop all the nodes of a heap.  The popped node IDs are compared with the
 * known order in which nodes will be popped.
 */
static void
pop_more(void)
{
    GnxHeap *heap;
    unsigned int i, k, v;
    const unsigned int id[9] = {2,    3,    5,    7,    11,   13,   17,   19,   23};
    const double key[9]      = {2.71, 3.14, 1.41, 0.57, 4.66, 1.20, 1.61, 2.68, 3.3};
    const unsigned int known[9] = {7, 13, 5, 17, 19, 2, 3, 23, 11};
    const unsigned int size = 9;

    heap = gnx_init_heap();
    assert(heap);
    add_nodes(heap, id, key, &size);
    assert(size == heap->size);

    /* Ensure that we always remove from the heap the node with minimum key.
     * With the above ID/key pairs, the order in which nodes are popped from
     * the heap is specified by the array 'known'.
     */
    k = size;
    for (i = 0; i < size; i++) {
        k--;
        assert(gnx_heap_pop(heap, &v));
        assert(v == known[i]);
        assert(k == heap->size);
    }

    gnx_destroy_heap(heap);
}

/* Pop a heap that has exactly one node. */
static void
pop_one(void)
{
    GnxHeap *heap;
    unsigned int w;
    const double key = (double)g_random_double();
    const unsigned int v = (unsigned int)g_random_int();

    heap = gnx_init_heap();
    assert(heap);
    assert(gnx_heap_add(heap, &v, &key));
    assert(1 == heap->size);
    assert(gnx_heap_pop(heap, &w));
    assert(v == w);
    assert(0 == heap->size);

    gnx_destroy_heap(heap);
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/heap/add", add);
    g_test_add_func("/heap/decrease", decrease);
    g_test_add_func("/heap/has", has);
    g_test_add_func("/heap/key", key);
    g_test_add_func("/heap/new", new);
    g_test_add_func("/heap/pop", pop);

    return g_test_run();
}
