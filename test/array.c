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
#include <stdlib.h>

#include <gnx.h>

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* append an element */
static void append_free_elements(void);
static void append_no_memory(void);
static void append_resize(void);
static void append_three(void);

/* new: create and destroy */
static void new_capacity_256(void);
static void new_capacity_default(void);
static void new_capacity_minimum(void);
static void new_free_elements(void);
static void new_no_memory(void);

/**************************************************************************
 * append an element
 *************************************************************************/

static void
append(void)
{
    append_free_elements();
    append_no_memory();
    append_resize();
    append_three();
}

/* Append elements to an array that has been set to release the memory of its
 * elements.
 */
static void
append_free_elements(void)
{
    GnxArray *array;
    int *elem;
    unsigned int i;
    const unsigned int capacity = 32;
    const unsigned int size = (unsigned int)g_random_int_range(1, 33);

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS);
    for (i = 0; i < size; i++) {
        elem = (int *)malloc(sizeof(int));
        *elem = (int)g_random_int_range(INT_MIN, INT_MAX);
        assert(gnx_array_append(array, elem));
    }
    assert(size == array->size);

    gnx_destroy_array(array);
}

/* Test the function gnx_array_append() in low-memory scenarios.
 */
static void
append_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxArray *array;
    int alloc_size;
    int a = (int)g_random_int_range(INT_MIN, INT_MAX);
    int b = (int)g_random_int_range(INT_MIN, INT_MAX);
    int c = (int)g_random_int_range(INT_MIN, INT_MAX);
    const unsigned int capacity = 2;

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS);
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(2 == array->size);

    /* Cannot allocate memory to resize the array. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_array_append(array, &c));
    assert(ENOMEM == errno);
    assert(2 == array->size);
    assert(capacity == array->capacity);

    gnx_destroy_array(array);
    gnx_alloc_reset_limit();
#endif
}

/* Append enough elements to trigger a resize of an array.
 */
static void
append_resize(void)
{
    GnxArray *array;
    int *elem;
    unsigned int i;
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    const unsigned int size = capacity + 1;

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS);

    for (i = 0; i < size; i++) {
        elem = (int *)malloc(sizeof(int));
        *elem = (int)g_random_int_range(INT_MIN, INT_MAX);
        assert(gnx_array_append(array, elem));
    }

    assert(size == array->size);
    assert((capacity << 1) == array->capacity);

    gnx_destroy_array(array);
}

/* Append elements to an array.  The array has been set to not release the
 * memory of its elements.
 */
static void
append_three(void)
{
    GnxArray *array;
    int a = (int)g_random_int_range(INT_MIN, INT_MAX);
    int b = (int)g_random_int_range(INT_MIN, INT_MAX);
    int c = (int)g_random_int_range(INT_MIN, INT_MAX);

    array = gnx_init_array();
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &c));

    assert(3 == array->size);
    assert(a == *(array->cell[0]));
    assert(b == *(array->cell[1]));
    assert(c == *(array->cell[2]));

    gnx_destroy_array(array);
}

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

    g_test_add_func("/array/append", append);
    g_test_add_func("/array/new", new);

    return g_test_run();
}
