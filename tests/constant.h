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

#ifndef GNX_CONSTANT_H
#define GNX_CONSTANT_H

/**************************************************************************
 * singular #define
 *************************************************************************/

/* The number of allocations that is required to initialize an array.  Any
 * non-negative integer below this number would result in an out-of-memory
 * error.
 */
#define GNX_ALLOC_ARRAY_SIZE (2)

/* The number of allocations that is required to initialize a bucket for a set
 * or dictionary.  Any non-negative integer below this number would result in
 * an out-of-memory error.
 */
#define GNX_ALLOC_BUCKET_SIZE (2)

/* The number of allocations that is required to add an entry to a bucket of a
 * set or dictionary.  This does not take into account the allocations to
 * possibly resize the bucket.  Any non-negative integer below this number
 * would result in an out-of-memory error.
 */
#define GNX_ALLOC_BUCKET_NODE_SIZE (1)

/* The number of allocations that is required to initialize a dictionary.  Any
 * non-negative integer below this number would result in an out-of-memory
 * error.
 */
#define GNX_ALLOC_DICT_SIZE (2)

/* The number of allocations that is required to add a node to a heap.  This
 * does not take into account the allocations to possibly resize the heap.  Any
 * non-negative integer below this number would result in an out-of-memory
 * error.
 */
#define GNX_ALLOC_HEAP_NODE_SIZE (2)

/* The number of allocations that is required to initialize a set.  Any
 * non-negative integer below this number would result in an out-of-memory
 * error.
 */
#define GNX_ALLOC_SET_SIZE (2)

/**************************************************************************
 * compound #define
 *************************************************************************/

/* The number of allocations that is required to add a node to an unweighted
 * digraph.  This does not take into account the allocations to possibly resize
 * the graph.  Any non-negative integer below this number would result in an
 * out-of-memory error.
 */
#define GNX_ALLOC_NODE_DIRECTED_UNWEIGHTED_SIZE ((2 * GNX_ALLOC_SET_SIZE) + 1)

/* The number of allocations that is required to add a node to a weighted
 * digraph.  This does not take into account the allocations to possibly resize
 * the graph.  Any non-negative integer below this number would result in an
 * out-of-memory error.
 */
#define GNX_ALLOC_NODE_DIRECTED_WEIGHTED_SIZE                              \
    (GNX_ALLOC_DICT_SIZE + GNX_ALLOC_SET_SIZE + 1)

#endif  /* GNX_CONSTANT_H */
