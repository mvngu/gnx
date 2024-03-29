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

#include <glib.h>
#include <gnx.h>

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* append an element */
static void append_free_elements(void);
static void append_no_memory(void);
static void append_resize(void);
static void append_three(void);

/* delete an element */
static void delete_empty(void);
static void delete_first(void);
static void delete_last(void);
static void delete_middle(void);
static void delete_random(void);

/* delete the tail element */
static void delete_tail_empty(void);
static void delete_tail_free(void);
static void delete_tail_one(void);
static void delete_tail_random(void);

/* new: create and destroy */
static void new_capacity_256(void);
static void new_capacity_default(void);
static void new_capacity_minimum(void);
static void new_free_elements(void);
static void new_no_memory(void);

/* sort the elements in non-decreasing order */
static void sort_duplicate_dont_free_sorted(void);
static void sort_duplicate_dont_free_unsorted(void);
static void sort_duplicate_free_sorted(void);
static void sort_duplicate_free_unsorted(void);
static void sort_empty(void);
static void sort_one(void);
static void sort_random_free(void);
static void sort_two_dont_free(void);
static void sort_two_free(void);

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

/* Append elements to an array of pointer.  The array has been configured to
 * release the memory of its elements.
 */
static void
append_free_elements(void)
{
    GnxArray *array;
    unsigned int *elem, i;
    const unsigned int capacity = 32;
    const unsigned int size = (unsigned int)g_random_int_range(1, 33);

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);
    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
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
    unsigned int a = (unsigned int)g_random_int();
    unsigned int b = (unsigned int)g_random_int();
    unsigned int c = (unsigned int)g_random_int();
    const unsigned int capacity = 2;
    const unsigned int size = 2;

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(size == array->size);

    /* Cannot allocate memory to resize the array. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_array_append(array, &c));
    assert(ENOMEM == errno);
    assert(size == array->size);
    assert(capacity == array->capacity);

    gnx_destroy_array(array);
    gnx_alloc_reset_limit();
#endif
}

/* Append enough elements to trigger a resize of an array of pointers.
 */
static void
append_resize(void)
{
    GnxArray *array;
    unsigned int *elem, i;
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    const unsigned int size = capacity + 1;

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_array_append(array, elem));
    }

    assert(size == array->size);
    assert((capacity << 1) == array->capacity);

    gnx_destroy_array(array);
}

/* Append elements to an array of pointers.  The array has been configured to
 * not release the memory of its elements.
 */
static void
append_three(void)
{
    GnxArray *array;
    unsigned int a = (unsigned int)g_random_int();
    unsigned int b = (unsigned int)g_random_int();
    unsigned int c = (unsigned int)g_random_int();
    const unsigned int size = 3;

    array = gnx_init_array();
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &c));

    assert(size == array->size);
    assert(a == *((unsigned int *)(array->cell[0])));
    assert(b == *((unsigned int *)(array->cell[1])));
    assert(c == *((unsigned int *)(array->cell[2])));

    gnx_destroy_array(array);
}

/**************************************************************************
 * delete an element
 *************************************************************************/

static void
delete(void)
{
    delete_empty();
    delete_first();
    delete_last();
    delete_middle();
    delete_random();
}

/* Deletes an element from an empty array.
 */
static void
delete_empty(void)
{
    GnxArray *array;
    const unsigned int i = 0;

    array = gnx_init_array();
    assert(0 == array->size);
    assert(!gnx_array_delete(array, &i));
    assert(0 == array->size);

    gnx_destroy_array(array);
}

/* Delete the first element.
 */
static void
delete_first(void)
{
    GnxArray *array;
    unsigned int a = (unsigned int)g_random_int();
    unsigned int b = (unsigned int)g_random_int();
    unsigned int c = (unsigned int)g_random_int();
    const unsigned int i = 0;
    const unsigned int size = 3;

    array = gnx_init_array();
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &c));
    assert(size == array->size);

    assert(gnx_array_delete(array, &i));
    assert((size - 1) == array->size);
    assert(b == *((unsigned int *)(array->cell[0])));
    assert(c == *((unsigned int *)(array->cell[1])));

    gnx_destroy_array(array);
}

/* Delete the last element.
 */
static void
delete_last(void)
{
    GnxArray *array;
    unsigned int a = (unsigned int)g_random_int();
    unsigned int b = (unsigned int)g_random_int();
    unsigned int c = (unsigned int)g_random_int();
    const unsigned int i = 2;
    const unsigned int size = 3;

    array = gnx_init_array();
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &c));
    assert(size == array->size);

    assert(gnx_array_delete(array, &i));
    assert((size - 1) == array->size);
    assert(a == *((unsigned int *)(array->cell[0])));
    assert(b == *((unsigned int *)(array->cell[1])));

    gnx_destroy_array(array);
}

/* Delete the middle element.
 */
static void
delete_middle(void)
{
    GnxArray *array;
    unsigned int a = (unsigned int)g_random_int();
    unsigned int b = (unsigned int)g_random_int();
    unsigned int c = (unsigned int)g_random_int();
    const unsigned int i = 1;
    const unsigned int size = 3;

    array = gnx_init_array();
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &c));
    assert(size == array->size);

    assert(gnx_array_delete(array, &i));
    assert((size - 1) == array->size);
    assert(a == *((unsigned int *)(array->cell[0])));
    assert(c == *((unsigned int *)(array->cell[1])));

    gnx_destroy_array(array);
}

/* Delete a random element of an array.
 */
static void
delete_random(void)
{
    GnxArray *array;
    unsigned int *elem, i;
    const unsigned int capacity = 32;
    const unsigned int size = (unsigned int)g_random_int_range(4, 33);

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);
    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_array_append(array, elem));
    }
    assert(size == array->size);

    i = (unsigned int)g_random_int_range(0, (int)size);
    assert(gnx_array_delete(array, &i));
    assert((size - 1) == array->size);

    gnx_destroy_array(array);
}

/**************************************************************************
 * delete the tail element
 *************************************************************************/

static void
delete_tail(void)
{
    delete_tail_empty();
    delete_tail_free();
    delete_tail_one();
    delete_tail_random();
}

/* Deletes the tail element of an empty array of pointers.
 */
static void
delete_tail_empty(void)
{
    GnxArray *array;

    array = gnx_init_array();
    assert(0 == array->size);
    assert(!gnx_array_delete_tail(array));
    assert(0 == array->size);

    gnx_destroy_array(array);
}

/* Deletes the tail element and releases its memory.
 */
static void
delete_tail_free(void)
{
    GnxArray *array;
    unsigned int *elem, i;
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    const unsigned int size = 3;

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_array_append(array, elem));
    }
    assert(size == array->size);

    assert(gnx_array_delete_tail(array));
    assert((size - 1) == array->size);

    gnx_destroy_array(array);
}

/* Deletes the tail element of an array that has only one element.
 */
static void
delete_tail_one(void)
{
    GnxArray *array;
    unsigned int a = (unsigned int)g_random_int();
    const unsigned int size = 1;

    array = gnx_init_array();
    assert(gnx_array_append(array, &a));
    assert(size == array->size);

    assert(gnx_array_delete_tail(array));
    assert((size - 1) == array->size);

    gnx_destroy_array(array);
}

/* Deletes the tail of an array that has a random number of elements.
 */
static void
delete_tail_random(void)
{
    GnxArray *array;
    unsigned int *elem, i;
    const unsigned int capacity = 32;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    /* Append a bunch of elements to the array. */
    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_array_append(array, elem));
    }
    assert(size == array->size);

    assert(gnx_array_delete_tail(array));
    assert((size - 1) == array->size);

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

/* An array with a capacity to hold 256 elements.
 */
static void
new_capacity_256(void)
{
    GnxArray *array;
    const unsigned int capacity = 256;

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_POINTER);
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

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_POINTER);
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

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_POINTER);
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
    assert(!gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT));
    assert(ENOMEM == errno);

    gnx_alloc_reset_limit();
#endif
}

/**************************************************************************
 * sort the elements in non-decreasing order
 *************************************************************************/

static void
sort(void)
{
    sort_duplicate_dont_free_sorted();
    sort_duplicate_dont_free_unsorted();
    sort_duplicate_free_sorted();
    sort_duplicate_free_unsorted();
    sort_empty();
    sort_one();
    sort_random_free();
    sort_two_dont_free();
    sort_two_free();
}

/* Sort an array that has duplicate elements.  The elements are already sorted.
 * We do not release the memory of the elements.
 */
static void
sort_duplicate_dont_free_sorted(void)
{
    GnxArray *array;
    unsigned int a, b, c, tmp_a, tmp_b, tmp_c;
    const int low = 0;
    const int high = 32;
    const unsigned int capacity = 4;

    /**********************************************************************
     * Elements are ordered as: a < b == c.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);
    c = b;
    assert(a < b);
    assert(b == c);

    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &c));
    assert(3 == array->size);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are ordered as: a == b < c.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    b = a;
    do {
        c = (unsigned int)g_random_int();
    } while (c <= b);
    assert(a == b);
    assert(b < c);

    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &c));
    assert(3 == array->size);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);
}

/* Sort an array that has duplicate elements.  The elements are yet to be
 * sorted.  We do not release the memory of the elements.
 */
static void
sort_duplicate_dont_free_unsorted(void)
{
    GnxArray *array;
    unsigned int a, b, c, tmp_a, tmp_b, tmp_c;
    const int low = 0;
    const int high = 32;
    const unsigned int capacity = 4;

    /**********************************************************************
     * Elements are ordered as: a < b == c.
     * Elements are appended as: b, c, a.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);
    c = b;
    assert(a < b);
    assert(b == c);

    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &c));
    assert(gnx_array_append(array, &a));
    assert(3 == array->size);
    tmp_b = *((unsigned int *)(array->cell[0]));
    tmp_c = *((unsigned int *)(array->cell[1]));
    tmp_a = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are ordered as: a < b == c.
     * Elements are appended as: b, a, c.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);
    c = b;
    assert(a < b);
    assert(b == c);

    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &c));
    assert(3 == array->size);
    tmp_b = *((unsigned int *)(array->cell[0]));
    tmp_a = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are ordered as: a == b < c.
     * Elements are appended as: c, a, b.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    b = a;
    do {
        c = (unsigned int)g_random_int();
    } while (c <= b);
    assert(a == b);
    assert(b < c);

    assert(gnx_array_append(array, &c));
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(3 == array->size);
    tmp_c = *((unsigned int *)(array->cell[0]));
    tmp_a = *((unsigned int *)(array->cell[1]));
    tmp_b = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are ordered as: a == b < c.
     * Elements are appended as: a, c, b.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    b = a;
    do {
        c = (unsigned int)g_random_int();
    } while (c <= b);
    assert(a == b);
    assert(b < c);

    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &c));
    assert(gnx_array_append(array, &b));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_c = *((unsigned int *)(array->cell[1]));
    tmp_b = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);
}

/* Sort an array that has duplicate elements.  The elements are already sorted.
 * We release the memory of the elements.
 */
static void
sort_duplicate_free_sorted(void)
{
    GnxArray *array;
    unsigned int a, b, c, *elem, tmp_a, tmp_b, tmp_c;
    const int low = 0;
    const int high = 32;
    const unsigned int capacity = 4;

    /**********************************************************************
     * Elements are ordered as: a < b == c.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);
    c = b;
    assert(a < b);
    assert(b == c);

    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = a;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = b;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = c;
    assert(gnx_array_append(array, elem));
    assert(3 == array->size);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are ordered as: a == b < c.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    b = a;
    do {
        c = (unsigned int)g_random_int();
    } while (c <= b);
    assert(a == b);
    assert(b < c);

    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = a;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = b;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = c;
    assert(gnx_array_append(array, elem));
    assert(3 == array->size);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);
}

/* Sort an array that has duplicate elements.  The elements are yet to be
 * sorted.  We release the memory of the elements.
 */
static void
sort_duplicate_free_unsorted(void)
{
    GnxArray *array;
    unsigned int a, b, c, *elem, tmp_a, tmp_b, tmp_c;
    const int low = 0;
    const int high = 32;
    const unsigned int capacity = 4;

    /**********************************************************************
     * Elements are ordered as: a < b == c.
     * Elements are appended as: b, c, a.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);
    c = b;
    assert(a < b);
    assert(b == c);

    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = b;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = c;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = a;
    assert(gnx_array_append(array, elem));
    assert(3 == array->size);

    tmp_b = *((unsigned int *)(array->cell[0]));
    tmp_c = *((unsigned int *)(array->cell[1]));
    tmp_a = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are ordered as: a < b == c.
     * Elements are appended as: b, a, c.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);
    c = b;
    assert(a < b);
    assert(b == c);

    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = b;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = a;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = c;
    assert(gnx_array_append(array, elem));
    assert(3 == array->size);

    tmp_b = *((unsigned int *)(array->cell[0]));
    tmp_a = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are ordered as: a == b < c.
     * Elements are appended as: c, a, b.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    b = a;
    do {
        c = (unsigned int)g_random_int();
    } while (c <= b);
    assert(a == b);
    assert(b < c);

    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = c;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = a;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = b;
    assert(gnx_array_append(array, elem));
    assert(3 == array->size);

    tmp_c = *((unsigned int *)(array->cell[0]));
    tmp_a = *((unsigned int *)(array->cell[1]));
    tmp_b = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are ordered as: a == b < c.
     * Elements are appended as: a, c, b.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    b = a;
    do {
        c = (unsigned int)g_random_int();
    } while (c <= b);
    assert(a == b);
    assert(b < c);

    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = a;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = c;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = b;
    assert(gnx_array_append(array, elem));
    assert(3 == array->size);

    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_c = *((unsigned int *)(array->cell[1]));
    tmp_b = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    assert(gnx_array_sort(array));
    assert(3 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    tmp_c = *((unsigned int *)(array->cell[2]));
    assert(a == tmp_a);
    assert(b == tmp_b);
    assert(c == tmp_c);

    gnx_destroy_array(array);
}

/* Sort an array that has zero elements.
 */
static void
sort_empty(void)
{
    GnxArray *array;
    const unsigned int capacity = 16;

    /**********************************************************************
     * Elements are general pointers.
     *********************************************************************/

    /* Do not release memory of elements. */
    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_POINTER);
    assert(0 == array->size);
    assert(!gnx_array_sort(array));
    assert(0 == array->size);
    gnx_destroy_array(array);

    /* Release memory of elements. */
    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_POINTER);
    assert(0 == array->size);
    assert(!gnx_array_sort(array));
    assert(0 == array->size);
    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are integer pointers.
     *********************************************************************/

    /* Do not release memory of elements. */
    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);
    assert(0 == array->size);
    assert(!gnx_array_sort(array));
    assert(0 == array->size);
    gnx_destroy_array(array);

    /* Release memory of elements. */
    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);
    assert(0 == array->size);
    assert(!gnx_array_sort(array));
    assert(0 == array->size);
    gnx_destroy_array(array);
}

/* Sort an array that has exactly one element.
 */
static void
sort_one(void)
{
    GnxArray *array;
    unsigned int e, *elem, tmp;
    const unsigned int capacity = 8;

    /**********************************************************************
     * Do not release memory of elements.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);
    e = (unsigned int)g_random_int();
    assert(gnx_array_append(array, &e));
    assert(1 == array->size);

    assert(gnx_array_sort(array));
    assert(1 == array->size);
    tmp = *((unsigned int *)(array->cell[0]));
    assert(e == tmp);

    gnx_destroy_array(array);

    /**********************************************************************
     * Release memory of elements.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = (unsigned int)g_random_int();
    assert(gnx_array_append(array, elem));
    assert(1 == array->size);

    assert(gnx_array_sort(array));
    assert(1 == array->size);
    tmp = *((unsigned int *)(array->cell[0]));
    assert(*elem == tmp);

    gnx_destroy_array(array);
}

/* Sort an array whose elements were randomly appended.  We release the memory
 * of the elements.
 */
static void
sort_random_free(void)
{
    GnxArray *array;
    unsigned int a, b, *elem, i;
    const int low = 16;
    const int high = 64;
    const unsigned int capacity = (unsigned int)high;
    const unsigned int size = (unsigned int)g_random_int_range(low, high);

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    /* Append elements to the array. */
    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int_range(0, high);
        assert(gnx_array_append(array, elem));
    }
    assert(size == array->size);

    /* Sort the array. */
    assert(gnx_array_sort(array));
    assert(size == array->size);

    /* Verify that the elements are sorted. */
    for (i = 0; i < (size - 1); i++) {
        a = *((unsigned int *)(array->cell[i]));
        b = *((unsigned int *)(array->cell[i + 1]));
        assert(a <= b);
    }

    gnx_destroy_array(array);
}

/* Sort an array that has two elements.  We do not release the memory of the
 * elements.
 */
static void
sort_two_dont_free(void)
{
    GnxArray *array;
    unsigned int a, b, tmp_a, tmp_b;
    const int low = 0;
    const int high = 32;
    const unsigned int capacity = 4;

    /**********************************************************************
     * Elements are already sorted.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);
    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);

    assert(a < b);
    assert(gnx_array_append(array, &a));
    assert(gnx_array_append(array, &b));
    assert(2 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    assert(a == tmp_a);
    assert(b == tmp_b);

    assert(gnx_array_sort(array));
    assert(2 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    assert(a == tmp_a);
    assert(b == tmp_b);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are not yet sorted.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_UINT);
    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);

    assert(a < b);
    assert(gnx_array_append(array, &b));
    assert(gnx_array_append(array, &a));
    assert(2 == array->size);
    tmp_b = *((unsigned int *)(array->cell[0]));
    tmp_a = *((unsigned int *)(array->cell[1]));
    assert(a == tmp_a);
    assert(b == tmp_b);

    assert(gnx_array_sort(array));
    assert(2 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    assert(a == tmp_a);
    assert(b == tmp_b);

    gnx_destroy_array(array);
}

/* Sort an array that has two elements.  We release the memory of the elements.
 */
static void
sort_two_free(void)
{
    GnxArray *array;
    unsigned int a, b, *elem, tmp_a, tmp_b;
    const int low = 0;
    const int high = 32;
    const unsigned int capacity = 4;

    /**********************************************************************
     * Elements are already sorted.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);

    assert(a < b);
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = a;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = b;
    assert(gnx_array_append(array, elem));
    assert(2 == array->size);

    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    assert(a == tmp_a);
    assert(b == tmp_b);

    assert(gnx_array_sort(array));
    assert(2 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    assert(a == tmp_a);
    assert(b == tmp_b);

    gnx_destroy_array(array);

    /**********************************************************************
     * Elements are not yet sorted.
     *********************************************************************/

    array = gnx_init_array_full(&capacity, GNX_FREE_ELEMENTS, GNX_UINT);

    a = (unsigned int)g_random_int_range(low, high);
    do {
        b = (unsigned int)g_random_int();
    } while (a >= b);

    assert(a < b);
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = b;
    assert(gnx_array_append(array, elem));
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = a;
    assert(gnx_array_append(array, elem));
    assert(2 == array->size);

    tmp_b = *((unsigned int *)(array->cell[0]));
    tmp_a = *((unsigned int *)(array->cell[1]));
    assert(a == tmp_a);
    assert(b == tmp_b);

    assert(gnx_array_sort(array));
    assert(2 == array->size);
    tmp_a = *((unsigned int *)(array->cell[0]));
    tmp_b = *((unsigned int *)(array->cell[1]));
    assert(a == tmp_a);
    assert(b == tmp_b);

    gnx_destroy_array(array);
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
    g_test_add_func("/array/delete", delete);
    g_test_add_func("/array/delete-tail", delete_tail);
    g_test_add_func("/array/new", new);
    g_test_add_func("/array/sort", sort);

    return g_test_run();
}
