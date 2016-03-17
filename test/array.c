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

/* Test the functions in the module src/array.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>

#include <gnx.h>

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* new: create and destroy */
static void new_capacity_256(void);
static void new_capacity_default(void);
static void new_capacity_minimum(void);
static void new_free_elements(void);
static void new_no_memory(void);

/**************************************************************************
 * new: create and destroy
 *************************************************************************/

static void
new(void)
{
    new_capacity_256();
    new_capacity_default();
    new_capacity_minimum();
    new_free_elements();
    new_no_memory();
}

/* An array with a capcity to hold 256 elements.
 */
static void
new_capacity_256(void)
{
    GnxArray *array;
    const unsigned int capacity = 256;

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS);
    assert(array);
    assert(capacity == array->capacity);
    assert(0 == array->size);
    assert(array->cell);

    gnx_destroy_array(array);
}

/* An array with a default capacity.
 */
static void
new_capacity_default(void)
{
    GnxArray *array;

    array = gnx_init_array();
    assert(array);
    assert(GNX_DEFAULT_ALLOC_SIZE == array->capacity);
    assert(0 == array->size);
    assert(array->cell);

    gnx_destroy_array(array);
}

/* An array with the minimum capacity.
 */
static void
new_capacity_minimum(void)
{
    GnxArray *array;
    const unsigned int capacity = 2;

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS);
    assert(array);
    assert(capacity == array->capacity);
    assert(0 == array->size);
    assert(array->cell);

    gnx_destroy_array(array);
}

/* Set the array to release the memory of its elements.
 */
static void
new_free_elements(void)
{
    GnxArray *array;
    const unsigned int capacity = 32;

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS);
    assert(array);
    assert(capacity == array->capacity);
    assert(0 == array->size);
    assert(array->cell);

    gnx_destroy_array(array);
}

/* Test the functions gnx_init_array() and gnx_init_array_full() under
 * low-memory scenarios.
 */
static void
new_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    int alloc_size;
    const unsigned int capacity = 2;

    /* Cannot allocate memory for the array. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS));
    assert(ENOMEM == errno);

    /* cannot allocate memory for the array cells */
    /* alloc_size++; */
    /* gnx_alloc_set_limit(alloc_size); */
    /* assert(!gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS)); */
    /* assert(ENOMEM == errno); */

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

    g_test_add_func("/array/new", new);

    return g_test_run();
}
