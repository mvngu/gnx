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

#include <errno.h>
#include <stdlib.h>

#include "dict.h"
#include "heap.h"

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
