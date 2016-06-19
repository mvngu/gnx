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

/**
 * @file dict.h
 * @brief Hash table with integer keys.
 *
 * A hash table does not make its own copy of any entry that is inserted.
 * Rather, we copy the pointers for key and value that are passed in and store
 * those pointers.  It is your responsibility to ensure that any key/value pair
 * that is inserted into the hash table exists for the duration of the table
 * itself.
 *
 * Use either of the functions gnx_init_dict() or gnx_init_dict_full() to
 * initialize a new hash table.  Destroy a hash table via the function
 * gnx_destroy_dict().
 */

/**
 * @brief Destroys a hash table.
 *
 * @param dict We want to destroy this hash table.
 */
void
gnx_destroy_dict(GnxDict *dict)
{
    if (!dict)
        return;
    if (dict->bucket) {
        /* FIXME: free keys */
        /* FIXME: free values */
        free(dict->bucket);
        dict->bucket = NULL;
    }
    free(dict);
    dict = NULL;
}

/**
 * @brief Initializes a new hash table.
 *
 * The hash table is initialized with default settings.  In particular, the
 * table will not release the memory of its keys and values.  It is your
 * responsibility to ensure that the memory of the keys and values are released
 * as appropriate.
 *
 * @sa gnx_init_dict_full() This function gives you full control over how a
 *     hash table will be initialized.
 *
 * @return See the return values of gnx_init_dict_full().
 */
GnxDict*
gnx_init_dict(void)
{
    return gnx_init_dict_full(GNX_DONT_FREE_KEYS, GNX_DONT_FREE_VALUES);
}

/**
 * @brief Initializes a new hash table.
 *
 * This function gives you full control over how a hash table will be
 * initialized.
 *
 * @sa gnx_init_dict() This function initializes a hash table with default
 *     settings.
 *
 * @param free_key Whether to release the memory of each key in the hash
 *        table.  The possible values are #GNX_FREE_KEYS and
 *        #GNX_DONT_FREE_KEYS.  If #GNX_FREE_KEYS, then the memory of each key
 *        will be released when you destroy the hash table via
 *        gnx_destroy_dict().  This option should only be used if each key to
 *        be inserted has memory that is allocated on the heap, i.e. via
 *        @c calloc(), @c malloc(), or @c realloc().  Using this option with
 *        stack memory will result in undefined behavior.  If
 *        #GNX_DONT_FREE_KEYS, then you are responsible for releasing the
 *        memory of each key in the hash table.  You can also use this option
 *        if each key has memory that is allocated on the stack.
 * @param free_value Whether to release the memory of each value in the hash
 *        table.  The possibilities are #GNX_FREE_VALUES and
 *        #GNX_DONT_FREE_VALUES.  If #GNX_FREE_VALUES, then the memory of each
 *        value will be released when you destroy the hash table via
 *        gnx_destroy_dict().  This option should only be used if each value to
 *        be inserted has memory that is allocated on the heap, i.e. via
 *        @c calloc(), @c malloc(), or @c realloc().  Using this option with
 *        stack memory will result in undefined behavior.  If
 *        #GNX_DONT_FREE_VALUES, then you are responsible for releasing the
 *        memory of each value in the hash table.  You can also use this option
 *        if each value has memory that is allocated on the stack.
 * @return An initialized hash table.  When you no longer need the hash table,
 *         you must destroy the table via the function gnx_destroy_dict().  If
 *         we are unable to allocate memory, then we set @c errno to @c ENOMEM
 *         and return @c NULL.
 */
GnxDict*
gnx_init_dict_full(const GnxBool free_key,
                   const GnxBool free_value)
{
    GnxDict *dict;
    const unsigned int bits_per_byte = 8;

    errno = 0;
    g_return_val_if_fail((GNX_FREE_KEYS & free_key)
                         || (GNX_DONT_FREE_KEYS & free_key),
                         NULL);
    g_return_val_if_fail((GNX_FREE_VALUES & free_value)
                         || (GNX_DONT_FREE_VALUES & free_value),
                         NULL);

    dict = (GnxDict *)malloc(sizeof(GnxDict));
    if (!dict)
        goto cleanup;

    dict->free_key = free_key;
    dict->free_value = free_value;
    dict->k = GNX_DEFAULT_EXPONENT;           /* Default exponent in 2^k. */
    dict->capacity = GNX_DEFAULT_ALLOC_SIZE;  /* Default number of buckets. */
    dict->size = 0;                           /* Hash table initially empty. */

    /* The hash table is represented as an array of buckets.  Each array
     * element is initialized to zero.  If an element is non-zero, then we know
     * that the corresponding bucket has at least one key/value entry.  If an
     * element is zero, then the corresponding bucket is empty.
     */
    dict->bucket = (gnxptr *)calloc(dict->capacity, sizeof(gnxptr));
    if (!dict->bucket)
        goto cleanup;

    dict->b = sizeof(unsigned int) * bits_per_byte;
    dict->d = dict->b - dict->k;

    /* The parameter a is part of the Woelfel universal family of hash
     * functions.  The parameter is an odd integer that is chosen uniformly at
     * random from the range [1, 2^b - 1].
     */
    do {
        dict->a = (unsigned int)g_random_int();
    } while (!(dict->a & 1));

    /* The parameter c is part of the Woelfel universal family of hash
     * functions.  The parameter is an integer that is chosen uniformly at
     * random from the range [0, 2^(b - k) - 1].
     */
    dict->c = (unsigned int)g_random_int_range(0, 1 << dict->d);

    return dict;

cleanup:
    errno = ENOMEM;
    gnx_destroy_dict(dict);
    return NULL;
}
