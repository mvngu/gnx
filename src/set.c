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

#include "array.h"
#include "set.h"

/**
 * @file set.h
 * @brief Set of integers.
 *
 * A set does not make its own copy of any element that is inserted.  Rather,
 * the set copies the pointer that is passed in and stores that pointer.  It is
 * your responsibility to ensure that any element that is inserted into the set
 * exists for the duration of the set itself.
 *
 * Use either of the functions gnx_init_set() or gnx_init_set_full() to
 * initialize a set of integers.  Destroy a set via the function
 * gnx_destroy_set().
 */

/**
 * @brief Destroys a set.
 *
 * @param set The set to destroy.  The set must have been initialized via
 *        gnx_init_set() or gnx_init_set_full().
 */
void
gnx_destroy_set(GnxSet *set)
{
    GnxArray *bucket;
    unsigned int i, j;

    if (!set)
        return;
    if (set->bucket) {
        for (i = 0; i < set->capacity; i++) {
            bucket = (GnxArray *)(set->bucket[i]);
            if (bucket) {
                if (GNX_FREE_ELEMENTS & set->free_elem) {
                    for (j = 0; j < bucket->size; j++) {
                        if (bucket->cell[j]) {
                            free(bucket->cell[j]);
                            bucket->cell[j] = NULL;
                        }
                    }
                }
                gnx_destroy_array(bucket);
                set->bucket[i] = NULL;
            }
        }
        free(set->bucket);
        set->bucket = NULL;
    }
    free(set);
    set = NULL;
}

/**
 * @brief Initializes a set of integers.
 *
 * This function initializes a set with default settings.  In particular, the
 * set will not release the memory of each of its elements when you destroy the
 * set via gnx_destroy_set().  If you initialize a set via gnx_init_set(), then
 * you are responsible for releasing the memory of each element in the set.
 *
 * @sa gnx_init_set_full() This function gives you full control over how a set
 *     will be initialized.
 *
 * @return An initialized set.  When you no longer need the set, you must
 *         destroy the set via the function gnx_destroy_set().  See the return
 *         value of gnx_init_set_full().
 */
GnxSet*
gnx_init_set(void)
{
    return gnx_init_set_full(GNX_DONT_FREE_ELEMENTS);
}

/**
 * @brief Initializes a set of integers.
 *
 * This function gives you full control over how a set will be initialized.
 *
 * @sa gnx_init_set() This function initializes a set with default settings.
 *
 * @param destroy Whether we want to directly release the memory of each
 *        element of the set.  The possible values are #GNX_FREE_ELEMENTS and
 *        #GNX_DONT_FREE_ELEMENTS.  If #GNX_FREE_ELEMENTS, then the memory of
 *        each element will be released when you destroy the set via
 *        gnx_destroy_set().  This option should only be used if each element
 *        to be inserted has memory that is allocated on the heap, i.e. via
 *        @c calloc(), @c malloc(), or @c realloc().  Using this option with
 *        stack memory will result in undefined behavior.  If
 *        #GNX_DONT_FREE_ELEMENTS, then you are responsible for releasing the
 *        memory of each element in the set.  You can also use this option if
 *        each element of the set has memory that is allocated on the stack.
 * @return An initialized set.  When you no longer need the set, you must
 *         destroy the set via the function gnx_destroy_set().  If we are
 *         unable to allocate memory, then @c errno is set to @c ENOMEM and we
 *         return @c NULL.
 */
GnxSet*
gnx_init_set_full(const GnxBool destroy)
{
    GnxSet *set;
    const unsigned int bits_per_byte = 8;

    errno = 0;
    g_assert((GNX_FREE_ELEMENTS & destroy)
             || (GNX_DONT_FREE_ELEMENTS & destroy));

    set = (GnxSet *)malloc(sizeof(GnxSet));
    if (!set)
        goto cleanup;

    set->free_elem = destroy;
    set->k = GNX_DEFAULT_EXPONENT;           /* The default exponent in 2^k. */
    set->capacity = GNX_DEFAULT_ALLOC_SIZE;  /* Default number of buckets. */
    set->size = 0;                           /* The set is initially empty. */

    /* The set is represented as an array of buckets.  Each array element is
     * initialized to zero.  If an element is non-zero, then we know that the
     * corresponding bucket has at least one entry.  If an element is zero,
     * then the corresponding bucket is empty.
     */
    set->bucket = (gnxptr *)calloc(set->capacity, sizeof(gnxptr));
    if (!set->bucket)
        goto cleanup;

    set->b = sizeof(unsigned int) * bits_per_byte;
    set->d = set->b - set->k;

    /* The parameter a is part of the Woelfel universal family of hash
     * functions.  The parameter is an odd integer that is chosen uniformly at
     * random from the range [1, 2^b - 1].
     */
    do {
        set->a = (unsigned int)g_random_int();
    } while (!(set->a & 1));

    /* The parameter c is part of the Woelfel universal family of hash
     * functions.  The parameter is an integer that is chosen uniformly at
     * random from the range [0, 2^(b - k) - 1].
     */
    set->c = (unsigned int)g_random_int_range(0, 1 << set->d);

    return set;

cleanup:
    errno = ENOMEM;
    gnx_destroy_set(set);
    return NULL;
}
