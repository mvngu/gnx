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

#define GNX_MAXIMUM_ELEMENTS                                               \
    ((unsigned int)INT_MAX + 1)  /**< @hideinitializer
                                  * The maximum number of elements in a
                                  * collection or container.  This maximum
                                  * value depends on the maximum value of an
                                  * @c int type.
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
    GNX_DONT_FREE_ELEMENTS = 1 << 1  /**< @hideinitializer
                                      * Do not release the memory of the
                                      * elements in a container.
                                      */
} GnxBool;

/**************************************************************************
 * data types
 *************************************************************************/

typedef int* gnxintptr;  /**< An @c int pointer. */

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

#endif  /* GNX_TYPE_H */
