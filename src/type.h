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

#ifndef GNX_TYPE_H
#define GNX_TYPE_H

#if !defined(GNX_H_INSIDE) && !defined(GNX_COMPILATION)
#error "Only <gnx.h> can be included directly."
#endif

#include <limits.h>

/**
 * @file type.h
 * @brief Definitions of commonly used types and constants.
 */

/**************************************************************************
 * constants
 *************************************************************************/

#define GNX_DEFAULT_EXPONENT (7)  /**< @hideinitializer
                                   * The exponent that is used to compute the
                                   * default allocation size.
                                   */

#define GNX_DEFAULT_ALLOC_SIZE                                             \
    (1 << GNX_DEFAULT_EXPONENT)  /**< @hideinitializer
                                  * By default, we allocate enough memory
                                  * to hold this many stuff.  The default
                                  * allocation size must be a power of 2.
                                  */

#define GNX_FAILURE (0)  /**< @hideinitializer
                          * Signifies that an operation has failed.
                          */

#define GNX_MAXIMUM_BUCKETS                                                \
    ((unsigned int)INT_MAX + 1)  /**< @hideinitializer
                                  * The maximum number of buckets in a set or
                                  * dictionary.  This maximum value depends on
                                  * the maximum value of an @c int type.
                                  */

#define GNX_MAXIMUM_ELEMENTS                                               \
    ((unsigned int)INT_MAX + 1)  /**< @hideinitializer
                                  * The maximum number of elements in a
                                  * collection or container.  This maximum
                                  * value depends on the maximum value of an
                                  * @c int type.
                                  */

#define GNX_MAXIMUM_NODES                                                  \
    ((unsigned int)INT_MAX + 1)  /**< @hideinitializer
                                  * The maximum number of nodes in a graph or
                                  * heap.  This maximum value depends on the
                                  * integer type that is used to represent
                                  * node IDs.
                                  */

#define GNX_MAXIMUM_NODE_ID                                                \
    ((unsigned int)INT_MAX)      /**< @hideinitializer
                                  * The maximum ID of any node.
                                  */

#define GNX_MPFR_PRECISION (128)  /**< @hideinitializer
                                   * How many bits of precision for a number
                                   * that is represented in MPFR.
                                   */

#define GNX_SUCCESS (!GNX_FAILURE)  /**< @hideinitializer
                                     * Signifies that an operation was
                                     * successful.
                                     */

/**************************************************************************
 * enumerations
 *************************************************************************/

/**
 * @brief Enumeration flags for various properties.
 */
typedef enum {
    GNX_FREE_ELEMENTS = 1 << 0,  /**< @hideinitializer
                                  * When we no longer need the elements of a
                                  * container, we directly release the memory
                                  * of those elements.
                                  */
    GNX_DONT_FREE_ELEMENTS = 1 << 1,  /**< @hideinitializer
                                       * Do not release the memory of the
                                       * elements in a container.
                                       */
    GNX_FREE_KEYS = 1 << 2,        /**< @hideinitializer
                                    * When we no longer need the keys of a hash
                                    * table, we directly release the memory of
                                    * those keys.
                                    */
    GNX_DONT_FREE_KEYS = 1 << 3,   /**< @hideinitializer
                                    * Do not release the memory of the keys of
                                    * a hash table.
                                    */
    GNX_FREE_VALUES = 1 << 4,      /**< @hideinitializer
                                    * When we no longer need the values of a
                                    * hash table, we directly release the
                                    * memory of those values.
                                    */
    GNX_DONT_FREE_VALUES = 1 << 5, /**< @hideinitializer
                                    * Do not release the memory of the values
                                    * of a hash table.
                                    */
    GNX_UNDIRECTED = 1 << 6,   /**< @hideinitializer
                                * A graph is undirected.  In an undirected
                                * graph, the edges @f$(u,v)@f$ and @f$(v,u)@f$
                                * are the same edge.
                                */
    GNX_DIRECTED = 1 << 7,     /**< @hideinitializer
                                * A graph is directed.  In a digraph, the edges
                                * @f$(u,v)@f$ and @f$(v,u)@f$ are distinct
                                * edges.
                                */
    GNX_NO_SELFLOOP = 1 << 8,  /**< @hideinitializer
                                * A graph without self-loops.
                                */
    GNX_SELFLOOP = 1 << 9,     /**< @hideinitializer
                                * A graph with self-loops.  A self-loop, or
                                * loop edge, is an edge both of whose end
                                * points are the same node.
                                */
    GNX_UNWEIGHTED = 1 << 10,  /**< @hideinitializer
                                * A graph is unweighted.  In an unweighted
                                * graph each edge is assigned unit weight, but
                                * for practical purposes we ignore the unit
                                * weights.
                                */
    GNX_WEIGHTED = 1 << 11     /**< @hideinitializer
                                * A graph is weighted.  In a weighted graph,
                                * each edge is assigned a numeric value, also
                                * called an edge weight.
                                */
} GnxBool;

/**************************************************************************
 * data types
 *************************************************************************/

typedef void* gnxptr;    /**< An untyped pointer. */

/**************************************************************************
 * fundamental data structures
 *************************************************************************/

/**
 * @brief Array of pointers.
 */
typedef struct {
    GnxBool free_elem;      /**< Whether to release the memory of each element
                             * in the array.
                             */
    unsigned int capacity;  /**< The capacity of the array.  This is the
                             * maximum possible number of elements in the
                             * array.
                             */
    unsigned int size;      /**< How many elements are in the array. */
    gnxptr *cell;           /**< The actual array of pointers. */
} GnxArray;

/**
 * @brief A hash table.
 *
 * The hash table is implemented with integer keys.  Collision is resolved via
 * separate chaining.  We use the universal family of hash functions introduced
 * by Woelfel @cite Woelfel2003; for further details see the
 * <a href="https://en.wikipedia.org/wiki/Universal_hashing">Wikipedia article</a>.
 */
typedef struct {
    GnxBool free_key;       /**< Whether to release the memory of each key in
                             * the hash table.
                             */
    GnxBool free_value;     /**< Whether to release the memory of each value
                             * in the hash table.
                             */
    unsigned int k;         /**< The exponent that is used to compute the number
                             * of buckets.
                             */
    unsigned int capacity;  /**< How many buckets.  This must be a power of 2.
                             */
    unsigned int size;      /**< How many entries in the hash table. */
    gnxptr *bucket;         /**< The array of buckets. */
    unsigned int b;         /**< How many bits are used to represent the
                             * <tt>unsigned int</tt> type.
                             */
    unsigned int d;         /**< The difference @f$b - k@f$. */
    unsigned int a;         /**< Parameter of the hash function.  This is an
                             * odd integer that is chosen uniformly at random
                             * from the range @f$[1,\, 2^b - 1]@f$, where
                             * @f$b@f$ is the number of bits in the
                             * representation of an <tt>unsigned int</tt> type.
                             */
    unsigned int c;         /**< Parameter of the hash function.  This is an
                             * integer that is chosen uniformly at random from
                             * the range @f$[0,\, 2^{b-k} - 1]@f$, where
                             * @f$b@f$ is the number of bits in the
                             * representation of an <tt>unsigned int</tt> type
                             * and @f$k@f$ is the exponent for computing the
                             * number of buckets.
                             */
} GnxDict;

/**
 * @brief An iterator over the elements of a dictionary.
 *
 * An iterator is usually allocated on the runtime stack and then initialized
 * via the function gnx_dict_iter_init().
 */
typedef struct {
    int bootstrap;   /**< Are we bootstrapping the process? */
    GnxDict *dict;   /**< Iterate over this dictionary. */
    unsigned int i;  /**< The bucket index. */
    unsigned int j;  /**< The entry index within bucket i. */
} GnxDictIter;

/**
 * @brief A minimum binary heap.
 */
typedef struct {
    GnxDict *map;           /**< A mapping from node ID to (index, key). */
    unsigned int *node;     /**< An array of nodes of the binary heap.  The
                             * heap is represented as an array.
                             */
    unsigned int size;      /**< How many elements in the binary heap. */
    unsigned int capacity;  /**< Memory is allocated for a given number of
                             * elements.  The capacity is the maximum possible
                             * number of nodes in a heap.  We assume that the
                             * capacity is a power of two.
                             */
} GnxHeap;

/**
 * @brief A queue of integers.
 */
typedef struct {
    GnxBool free_elem;      /**< Whether to release the memory of each element
                             * in the queue.
                             */
    unsigned int i;         /**< The index of the head of the queue. */
    unsigned int j;         /**< The index of the tail of the queue. */
    unsigned int size;      /**< How many elements are in the queue. */
    unsigned int capacity;  /**< The maximum possible number of elements. */
    gnxptr *cell;           /**< An array of the elements of the queue. */
} GnxQueue;

/**
 * @brief A set of integers.
 *
 * The set is implemented as a hash table with only keys and no values.
 * Collision is resolved via separate chaining.  We use the universal family of
 * hash functions introduced by Woelfel @cite Woelfel2003; for further details
 * see the
 * <a href="https://en.wikipedia.org/wiki/Universal_hashing">Wikipedia article</a>.
 */
typedef struct {
    GnxBool free_elem;      /**< Whether the set can directly free its own
                             * elements as part of the destroy procedure.
                             */
    unsigned int k;         /**< The exponent that is used to compute the
                             * number of buckets.
                             */
    unsigned int capacity;  /**< How many buckets.  This must be a power of 2.
                             */
    unsigned int size;      /**< How many entries in the set. */
    gnxptr *bucket;         /**< The array of buckets. */
    unsigned int b;         /**< How many bits are used to represent the
                             * <tt>unsigned int</tt> type.
                             */
    unsigned int d;         /**< The difference @f$b - k@f$. */
    unsigned int a;         /**< Parameter of the hash function.  This is an
                             * odd integer that is chosen uniformly at random
                             * from the range @f$[1,\, 2^b - 1]@f$, where
                             * @f$b@f$ is the number of bits in the
                             * representation of an <tt>unsigned int</tt> type.
                             */
    unsigned int c;         /**< Parameter of the hash function.  This is an
                             * integer that is chosen uniformly at random from
                             * the range @f$[0,\, 2^{b-k} - 1]@f$, where
                             * @f$b@f$ is the number of bits in the
                             * representation of an <tt>unsigned int</tt> type
                             * and @f$k@f$ is the exponent for computing the
                             * number of buckets.
                             */
} GnxSet;

/**
 * @brief An iterator over the elements of a set.
 *
 * An iterator is usually allocated on the runtime stack and then initialized
 * via the function gnx_set_iter_init().
 */
typedef struct {
    int bootstrap;   /**< Are we bootstrapping the process? */
    GnxSet *set;     /**< Iterate over this set. */
    unsigned int i;  /**< The bucket index. */
    unsigned int j;  /**< The entry index within bucket i. */
} GnxSetIter;

/**
 * @brief A stack of integers.
 */
typedef struct {
    GnxArray *array;    /**< The array of elements of the stack. */
    unsigned int size;  /**< How many elements are in the stack. */
} GnxStack;

/**************************************************************************
 * graph data structure
 *************************************************************************/

/**
 * @brief The fundamental graph data structure.
 *
 * A generic graph has the following structure:
 *
 * <pre>
 * graph: {
 *     node_1: {
 *         neighbor_1: {
 *             property_1: {...},
 *             ...
 *         },
 *         ...
 *     },
 *     ...
 * }
 * </pre>
 */
typedef struct {
    GnxSet *node;              /**< A collection of nodes of the graph.  This
                                * set does not necessarily contain all the
                                * nodes of the graph.  The set is used to
                                * determine whether to allocate memory for a
                                * node.
                                */
    gnxptr *graph;             /**< The graph structure as an array of adjacency
                                * lists.  If node[i] is NULL, then the node
                                * with an ID of i is not in the graph.
                                * Otherwise node i is in the graph and node[i]
                                * points to the collection of all nodes that
                                * are adjacent to i.
                                */
    GnxBool directed;          /**< Is the graph directed? */
    GnxBool selfloop;          /**< Do we allow self-loops in the graph? */
    GnxBool weighted;          /**< Is the graph weighted? */
    unsigned int capacity;     /**< The maximum number of nodes that the graph
                                * can hold.  If adding another node would
                                * exceed the capacity, then we need to resize
                                * the array of adjacency lists.
                                */
    unsigned int total_edges;  /**< How many edges? */
    unsigned int total_nodes;  /**< How many nodes or vertices? */
} GnxGraph;

/**
 * @brief Iterator over the nodes of a graph.
 */
typedef struct {
    int bootstrap;    /**< Are we bootstrapping the process? */
    GnxGraph *graph;  /**< Iterate over the nodes of this graph. */
    unsigned int i;   /**< The index into the array of adjacency lists. */
} GnxNodeIter;

#endif  /* GNX_TYPE_H */
