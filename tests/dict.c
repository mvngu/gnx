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

/* Test the functions in the module src/dict.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>

#include <gnx.h>

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* new: create and destroy */
static void new_dict(void);
static void new_dict_full(void);
static void new_no_memory(void);

/**************************************************************************
 * new: create and destroy
 *************************************************************************/

static void
new(void)
{
    new_dict();
    new_dict_full();
    new_no_memory();
}

/* A dictionary with default settings. */
static void
new_dict(void)
{
    GnxDict *dict;

    dict = gnx_init_dict();
    assert(dict);
    assert(GNX_DONT_FREE_KEYS & dict->free_key);
    assert(GNX_DONT_FREE_VALUES & dict->free_value);
    assert(GNX_DEFAULT_EXPONENT == dict->k);
    assert(GNX_DEFAULT_ALLOC_SIZE == dict->capacity);
    assert(1u << GNX_DEFAULT_EXPONENT == dict->capacity);
    assert(0 == dict->size);
    assert(dict->b - dict->k == dict->d);
    assert(dict->a <= UINT_MAX);
    assert(1 == dict->a % 2);
    assert(dict->c <= (1u << dict->d));

    gnx_destroy_dict(dict);
}

/* A dictionary with custom settings. */
static void
new_dict_full(void)
{
    GnxDict *dict;

    /***********************************************************************
     * Free keys only.
     **********************************************************************/

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_DONT_FREE_VALUES);
    assert(dict);
    assert(GNX_FREE_KEYS & dict->free_key);
    assert(GNX_DONT_FREE_VALUES & dict->free_value);
    assert(GNX_DEFAULT_EXPONENT == dict->k);
    assert(GNX_DEFAULT_ALLOC_SIZE == dict->capacity);
    assert(1u << GNX_DEFAULT_EXPONENT == dict->capacity);
    assert(0 == dict->size);
    assert(dict->b - dict->k == dict->d);
    assert(dict->a <= UINT_MAX);
    assert(1 == dict->a % 2);
    assert(dict->c <= (1u << dict->d));
    gnx_destroy_dict(dict);

    /***********************************************************************
     * Free values only.
     **********************************************************************/

    dict = gnx_init_dict_full(GNX_DONT_FREE_KEYS, GNX_FREE_VALUES);
    assert(dict);
    assert(GNX_DONT_FREE_KEYS & dict->free_key);
    assert(GNX_FREE_VALUES & dict->free_value);
    assert(GNX_DEFAULT_EXPONENT == dict->k);
    assert(GNX_DEFAULT_ALLOC_SIZE == dict->capacity);
    assert(1u << GNX_DEFAULT_EXPONENT == dict->capacity);
    assert(0 == dict->size);
    assert(dict->b - dict->k == dict->d);
    assert(dict->a <= UINT_MAX);
    assert(1 == dict->a % 2);
    assert(dict->c <= (1u << dict->d));
    gnx_destroy_dict(dict);

    /***********************************************************************
     * Free keys and values.
     **********************************************************************/

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    assert(dict);
    assert(GNX_FREE_KEYS & dict->free_key);
    assert(GNX_FREE_VALUES & dict->free_value);
    assert(GNX_DEFAULT_EXPONENT == dict->k);
    assert(GNX_DEFAULT_ALLOC_SIZE == dict->capacity);
    assert(1u << GNX_DEFAULT_EXPONENT == dict->capacity);
    assert(0 == dict->size);
    assert(dict->b - dict->k == dict->d);
    assert(dict->a <= UINT_MAX);
    assert(1 == dict->a % 2);
    assert(dict->c <= (1u << dict->d));
    gnx_destroy_dict(dict);
}

/* Test under low-memory scenarios. */
static void
new_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    int alloc_size;

    /* Cannot allocate memory for a hash table. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_init_dict());
    assert(ENOMEM == errno);

    gnx_alloc_reset_limit();
#endif
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/dict/new", new);

    return g_test_run();
}
