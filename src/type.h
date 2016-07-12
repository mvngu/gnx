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

#include <glib.h>

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
    GNX_DONT_FREE_VALUES = 1 << 5  /**< @hideinitializer
                                    * Do not release the memory of the values
                                    * of a hash table.
                                    */
} GnxBool;

/**************************************************************************
 * data types
 *************************************************************************/

typedef int* gnxintptr;  /**< An @c int pointer. */
typedef void* gnxptr;    /**< An untyped pointer. */

/**************************************************************************
 * data structures
 *************************************************************************/

/**
 * @brief Array of integers.
 */
typedef struct {
    GnxBool free_elem;      /**< Whether to release the memory of each element
                             * in the array.
                             */
    unsigned int capacity;  /**< The capacity of the array.  This is the
                             * maximum possible number of elements in the
                             * array.
                             */
    unsigned int size;      /**< How many elements in the array. */
    gnxintptr *cell;        /**< The actual array of integers. */
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
    gboolean bootstrap;  /**< Are we bootstrapping the process? */
    GnxDict *dict;       /**< Iterate over this dictionary. */
    unsigned int i;      /**< The bucket index. */
    unsigned int j;      /**< The entry index within bucket i. */
} GnxDictIter;

/**
 * @brief A minimum binary heap.
 */
typedef struct {
    GnxDict *map;           /**< A mapping from node ID to (index, key). */
    int *node;              /**< An array of nodes of the binary heap.  The
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
    gnxintptr *cell;        /**< An array of the elements of the queue. */
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
    gboolean bootstrap;  /**< Are we bootstrapping the process? */
    GnxSet *set;         /**< Iterate over this set. */
    unsigned int i;      /**< The bucket index. */
    unsigned int j;      /**< The entry index within bucket i. */
} GnxSetIter;

/**
 * @brief A stack of integers.
 */
typedef struct {
    GnxArray *array;    /**< The array of elements of the stack. */
    unsigned int size;  /**< How many elements are in the stack. */
} GnxStack;

#endif  /* GNX_TYPE_H */
