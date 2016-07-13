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

/* has */
static void has_empty(void);

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
    g_test_add_func("/heap/has", has);
    g_test_add_func("/heap/new", new);
    g_test_add_func("/heap/pop", pop);

    return g_test_run();
}
