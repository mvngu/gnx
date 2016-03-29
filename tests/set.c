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

/* Test the functions in the module src/set.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>

#include <gnx.h>

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* new: create and destroy */
static void new_dont_free_elements(void);
static void new_free_elements(void);
static void new_no_memory(void);

/**************************************************************************
 * new set: create and destroy
 *************************************************************************/

static void
new(void)
{
    new_dont_free_elements();
    new_free_elements();
    new_no_memory();
}

/* Initialize a set with the option to not free the elements.
 */
static void
new_dont_free_elements(void)
{
    GnxSet *set;

    set = gnx_init_set();
    assert(set);
    assert(GNX_DONT_FREE_ELEMENTS & set->free_elem);
    assert(GNX_DEFAULT_EXPONENT == set->k);
    assert(GNX_DEFAULT_ALLOC_SIZE == set->capacity);
    assert(1 << GNX_DEFAULT_EXPONENT == set->capacity);
    assert(0 == set->size);
    assert(set->b - set->k == set->d);
    assert(set->a <= UINT_MAX);
    assert(1 == set->a % 2);
    assert(set->c <= (1u << set->d));

    gnx_destroy_set(set);
}

/* Initialize a set with the option to free the elements.
 */
static void
new_free_elements(void)
{
    GnxSet *set;

    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    assert(set);
    assert(GNX_FREE_ELEMENTS & set->free_elem);
    assert(GNX_DEFAULT_EXPONENT == set->k);
    assert(GNX_DEFAULT_ALLOC_SIZE == set->capacity);
    assert(1 << GNX_DEFAULT_EXPONENT == set->capacity);
    assert(0 == set->size);
    assert(set->b - set->k == set->d);
    assert(set->a <= UINT_MAX);
    assert(1 == set->a % 2);
    assert(set->c <= (1u << set->d));

    gnx_destroy_set(set);
}

/* Test the function gnx_init_set() under low-memory scenarios.
 */
static void
new_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    int alloc_size;

    /* Cannot allocate memory for the set. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_init_set());
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

    g_test_add_func("/set/new", new);

    return g_test_run();
}
