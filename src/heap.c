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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "dict.h"
#include "heap.h"
#include "sanity.h"
#include "util.h"

/**
 * @file heap.h
 * @brief A minimum binary heap.
 *
 * A binary heap is implemented as an array of nodes that is sorted according
 * to the node keys.  Insertion and removal each requires a worst-case time
 * of @f$O(\log_2 n)@f$, given a heap of @f$n@f$ nodes.  We also maintain a
 * look-up table that is effectively the set of nodes in the heap.  The table
 * allows us to search for a node and replace the key of a node, each with a
 * worst-case time of @f$O(1)@f$.  This is better than the usual implementation
 * of binary heaps
 * (see <a href="https://en.wikipedia.org/wiki/Binary_heap">Wikipedia</a>),
 * which requires a worst-case time of @f$O(n)@f$ to search for a node.
 * However, the constant look-up time is achieved at the cost of using more
 * space to store the look-up table.
 *
 * Use the function gnx_init_heap() to create a heap with zero nodes.  Destroy
 * a heap via the function gnx_destroy_heap().
 */

/**************************************************************************
 * internal data structures
 *************************************************************************/

/* @cond */
/* A node of a heap.
 */
typedef struct {
    unsigned int index;  /* The index of the node in the array of nodes. */
    double key;          /* The key of the node. */
} GnxNode;
/* @endcond */

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static inline double gnx_i_node_key(const GnxHeap *heap,
                                    const unsigned int *v);
static inline void gnx_i_update_node_index(GnxHeap *heap,
                                           const unsigned int *v,
                                           const unsigned int *i);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief The key of a node of a binary heap.
 *
 * @param heap A binary heap.
 * @param v A node in the heap.
 * @return The key of the node @a v.
 */
static inline double
gnx_i_node_key(const GnxHeap *heap,
               const unsigned int *v)
{
    GnxNode *node = (GnxNode *)gnx_dict_has(heap->map, v);
    g_assert(node);
    return node->key;
}

/**
 * @brief Updates the index of a node.
 *
 * @param heap A minimum binary heap.
 * @param v We want to update the index of this node.
 * @param i The new index of the node.
 * @return Nonzero if we successfully updated the index of the node; zero
 *         otherwise.  If we are unable to allocate memory while performing the
 *         update, then @c errno is set to @c ENOMEM and we return zero.
 */
static inline void
gnx_i_update_node_index(GnxHeap *heap,
                        const unsigned int *v,
                        const unsigned int *i)
{
    GnxNode *node = (GnxNode *)gnx_dict_has(heap->map, v);
    g_assert(node);
    node->index = *i;
}

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Destroys a minimum binary heap.
 *
 * @param heap We want to destroy this binary heap.
 */
void
gnx_destroy_heap(GnxHeap *heap)
{
    if (!heap)
        return;
    if (heap->map) {
        gnx_destroy_dict(heap->map);
        heap->map = NULL;
    }
    if (heap->node) {
        free(heap->node);
        heap->node = NULL;
    }
    free(heap);
    heap = NULL;
}

/**
 * @brief Adds a new node to a minimum binary heap.
 *
 * @param heap Add a new node to this minimum binary heap.
 * @param v The node ID.  For this node to be successfully inserted into the
 *        heap, the number of nodes currently in the heap must be less than
 *        #GNX_MAXIMUM_NODES.  We assume that this is the case for the current
 *        state of the heap.
 * @param key The key of the node @a v.
 * @return Nonzero if the new node @a v was successfully added to the heap;
 *         zero otherwise.  We also return zero if @a v is already in the heap.
 *         If we are unable to allocate memory for the new node, then @c errno
 *         is set to @c ENOMEM and we return zero.
 */
int
gnx_heap_add(GnxHeap *heap,
             const unsigned int *v,
             const double *key)
{
    double keyp;
    GnxNode *vertex = NULL;
    unsigned int i, new_capacity, p;
    unsigned int *new_array = NULL;
    unsigned int *w = NULL;

    errno = 0;
    g_return_val_if_fail(key, GNX_FAILURE);
    if (gnx_heap_has(heap, v))
        return GNX_FAILURE;
    g_assert(heap->size < GNX_MAXIMUM_NODES);

    vertex = (GnxNode *)malloc(sizeof(GnxNode));
    if (!vertex)
        goto cleanup;

    w = (unsigned int *)malloc(sizeof(unsigned int));
    if (!w)
        goto cleanup;

    /* Possibly resize the heap by doubling the allocated chunk of memory. */
    if (heap->size >= heap->capacity) {
        new_capacity = heap->capacity << 1;
        g_assert(new_capacity <= GNX_MAXIMUM_NODES);

        new_array
            = (unsigned int *)realloc(heap->node,
                                      new_capacity * sizeof(unsigned int));
        if (!new_array)
            goto cleanup;

        heap->capacity = new_capacity;
        heap->node = new_array;
    }

    /* Append the new node to the array and perform a sift-up. */
    i = heap->size;
    while (i > 0) {
        p = (i - 1) >> 1;
        keyp = gnx_i_node_key(heap, &(heap->node[p]));

        if (gnx_double_cmp_le(&keyp, key)) {
            break;
        } else {
            gnx_i_update_node_index(heap, &(heap->node[p]), &i);
            heap->node[i] = heap->node[p];
            i = p;
        }
    }

    *w = *v;
    heap->node[i] = *w;
    vertex->index = i;
    vertex->key = *key;
    assert(gnx_dict_add(heap->map, w, vertex));
    (heap->size)++;
    g_assert(heap->map->size == heap->size);

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (vertex)
        free(vertex);
    if (w)
        free(w);
    return GNX_FAILURE;
}

/**
 * @brief Whether a heap contains a given node.
 *
 * @param heap Query this minimum binary heap.
 * @param v The node to query.
 * @return Nonzero if the node @a v is already in the heap; zero otherwise.
 */
int
gnx_heap_has(const GnxHeap *heap,
             const unsigned int *v)
{
    gnx_i_check_heap(heap);
    g_return_val_if_fail(v, GNX_FAILURE);

    if (gnx_dict_has(heap->map, v))
        return GNX_SUCCESS;

    return GNX_FAILURE;
}

/**
 * @brief Initializes a minimum binary heap.
 *
 * @return An initialized minimum binary heap.  When you no longer need the
 *         heap, you must destroy the heap via the function gnx_destroy_heap().
 *         If we are unable to allocate memory to initialize the heap, then
 *         @c errno is set to @c ENOMEM and we return @c NULL.
 */
GnxHeap*
gnx_init_heap(void)
{
    GnxHeap *heap;
    const unsigned int reserved_size = GNX_DEFAULT_ALLOC_SIZE;

    errno = 0;
    heap = (GnxHeap *)malloc(sizeof(GnxHeap));
    if (!heap)
        goto cleanup;

    heap->map = NULL;
    heap->node = NULL;

    /* Map a node ID to the pair (index, key).  Note that 'index' is the index
     * of the node in the array representation and 'key' is the node's key.
     */
    heap->map = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    if (!heap->map)
        goto cleanup;

    /* The heap is represented as an array of nodes. */
    heap->node = (unsigned int *)malloc(reserved_size * sizeof(unsigned int));
    if (!heap->node)
        goto cleanup;

    heap->size = 0;
    heap->capacity = reserved_size;
    return heap;

cleanup:
    errno = ENOMEM;
    gnx_destroy_heap(heap);
    return NULL;
}
