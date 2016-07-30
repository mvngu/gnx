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
#include <limits.h>
#include <stdlib.h>

#include <glib.h>
#include <gnx.h>

#include "constant.h"

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* add elements */
static void add_duplicate(void);
static void add_free(void);
static void add_no_memory(void);
static void add_one(void);
static void add_resize(void);

/* any element */
static void any_one(void);
static void any_random(void);

/* delete elements */
static void delete_empty(void);
static void delete_non_member(void);
static void delete_one(void);
static void delete_random_dont_free_elements(void);
static void delete_random_free_elements(void);

/* has an element */
static void has_empty(void);
static void has_member(void);
static void has_non_member(void);

/* iterator */
static void iter_count(void);
static void iter_empty(void);
static void iter_one(void);
static void iter_random(void);

/* new: create and destroy */
static void new_dont_free_elements(void);
static void new_free_elements(void);
static void new_no_memory(void);

/**************************************************************************
 * add elements
 *************************************************************************/

static void
add(void)
{
    add_duplicate();
    add_free();
    add_no_memory();
    add_one();
    add_resize();
}

/* Add a duplicate element to a set.
 */
static void
add_duplicate(void)
{
    GnxSet *set;
    unsigned int a = (unsigned int)g_random_int();
    unsigned int b = a;
    unsigned int *x, *y;
    const unsigned int size = 1;

    /**********************************************************************
     * Do not release memory of elements.
     *********************************************************************/

    set = gnx_init_set();
    assert(gnx_set_add(set, &a));
    assert(size == set->size);

    assert(a == b);
    assert(!gnx_set_add(set, &b));
    assert(size == set->size);

    gnx_destroy_set(set);

    /**********************************************************************
     * Release memory of elements.
     *********************************************************************/

    x = (unsigned int *)malloc(sizeof(unsigned int));
    *x = (unsigned int)g_random_int();
    y = (unsigned int *)malloc(sizeof(unsigned int));
    *y = *x;

    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    assert(gnx_set_add(set, x));
    assert(size == set->size);

    assert(*x == *y);
    assert(!gnx_set_add(set, y));
    assert(size == set->size);

    free(y);
    gnx_destroy_set(set);
}

/* Add elements and allow the set to release the memory of the elements.
 */
static void
add_free(void)
{
    GnxSet *set;
    unsigned int *elem, i;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    assert(0 == set->size);

    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_set_add(set, elem));
    }
    assert(size == set->size);

    gnx_destroy_set(set);
}

/* Test the function gnx_set_add() in low-memory scenarios.
 */
static void
add_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxSet *set;
    int alloc_size;
    unsigned int *elem, i, j, n;
    const unsigned int lista[10] = {  /* Values for the a parameter. */
        3968615651, 1606104101, 2596383589, 968646815, 3849427159,
        818668303, 96063689, 1868525653, 1219000319, 1903981943};
    const unsigned int listc[10] = {  /* Values for the c parameter. */
        24242974, 29023874, 31052654, 9402626, 25541919,
        15114733, 30711375, 23872027, 24260136, 25511161};

    /* Initialize the set to have a pair of pre-determined values for its a
     * and c parameters.  These pairs of values are known to allow the test
     * function to pass.
     */
    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    j = (unsigned int)g_random_int_range(0, 10);
    set->a = lista[j];
    set->c = listc[j];

    /* Insert just enough elements such that a resize is not triggered.  We
     * assume that the load factor is 3/4.  If n is the number of entries in
     * the set and m is the number of buckets, then we will not trigger a
     * resize provided that
     *
     *  n     3
     * --- < ---
     *  m     4
     *
     * The number of buckets m = 2^k is a power of two.  Solving the inequality
     * for n yields n < 3 * 2^(k-2).  Thus we will not trigger a resize
     * provided that we insert at most (3 * 2^(k-2) - 1) elements.
     */
    n = (3 * (1u << (set->k - 2))) - 1;
    for (i = 0; i < n; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = i;
        assert(gnx_set_add(set, elem));
    }
    assert(n == set->size);

    /* A new element that will trigger a resize. */
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = n;

    /* Cannot allocate memory for a new element. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_set_add(set, elem));
    assert(ENOMEM == errno);

    /* Cannot allocate memory to create a new bucket array for resizing. */
    alloc_size = GNX_ALLOC_ARRAY_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_set_add(set, elem));
    assert(ENOMEM == errno);

    /* Cannot allocate memory to create a bucket for the bucket array. */
    alloc_size++;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_set_add(set, elem));
    assert(ENOMEM == errno);

    free(elem);
    gnx_destroy_set(set);
    gnx_alloc_reset_limit();
#endif
}

/* Add one element to an empty set.
 */
static void
add_one(void)
{
    GnxSet *set;
    unsigned int elem = (unsigned int)g_random_int();
    unsigned int *x;

    /**********************************************************************
     * Do not release memory of elements.
     *********************************************************************/

    set = gnx_init_set();
    assert(0 == set->size);
    assert(gnx_set_add(set, &elem));
    assert(1 == set->size);

    gnx_destroy_set(set);

    /**********************************************************************
     * Release memory of elements.
     *********************************************************************/

    x = (unsigned int *)malloc(sizeof(unsigned int));
    *x = (unsigned int)g_random_int();

    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    assert(0 == set->size);
    assert(gnx_set_add(set, x));
    assert(1 == set->size);

    gnx_destroy_set(set);
}

/* Add enough elements to trigger a resize of a set.
 */
static void
add_resize(void)
{
    GnxSet *set;
    unsigned int *elem, i;

    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    assert(0 == set->size);
    assert(GNX_DEFAULT_ALLOC_SIZE == set->capacity);

    /* Add a bunch of elements to trigger a resize of the set. */
    for (i = 0; i < GNX_DEFAULT_ALLOC_SIZE; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = i;
        assert(gnx_set_add(set, elem));
    }

    /* The set has been resized because the capacity has doubled. */
    assert(GNX_DEFAULT_ALLOC_SIZE == set->size);
    assert((GNX_DEFAULT_ALLOC_SIZE << 1) == set->capacity);
    assert(GNX_DEFAULT_EXPONENT + 1 == set->k);
    assert(set->b - set->k == set->d);
    assert(set->a <= UINT_MAX);
    assert(1 == set->a % 2);
    assert(set->c <= (1u << set->d));

    gnx_destroy_set(set);
}

/**************************************************************************
 * any element
 *************************************************************************/

static void
any(void)
{
    any_one();
    any_random();
}

/* Choose an element from a set that has exactly one element.
 */
static void
any_one(void)
{
    GnxSet *set;
    unsigned int elem = (unsigned int)g_random_int();
    unsigned int target;

    set = gnx_init_set();
    assert(gnx_set_add(set, &elem));
    assert(1 == set->size);

    target = gnx_set_any(set);
    assert(target == elem);
    assert(1 == set->size);

    gnx_destroy_set(set);
}

/* Choose an element from a set that has a random number of elements.
 */
static void
any_random(void)
{
    GnxSet *set;
    unsigned int elem, i, j, *list, target, unique;
    const unsigned int size = (3u << (GNX_DEFAULT_EXPONENT - 2)) - 1;

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    set = gnx_init_set();

    /* Generate a bunch of unique random integers.  The size of the list is
     * chosen such that we do not trigger a resize of the set.  Here we assume
     * that a resize of the set will not be triggered provided that the number
     * of elements inserted into the set is kept below 3 * 2^(k-2), where k is
     * the exponent that is used to compute the number of buckets.  In other
     * words, a resize will not be triggered provided that the load factor is
     * kept below the threshold of 3/4.
     */
    for (i = 0; i < size; i++) {
        unique = FALSE;
        do {
            elem = (unsigned int)g_random_int();
            for (j = 0; j < i; j++) {
                if (elem == list[j])
                    break;
            }
            if (j >= i)
                unique = TRUE;
        } while (!unique);

        list[i] = elem;
        assert(gnx_set_add(set, &(list[i])));
    }
    assert(size == set->size);

    /* Check that any element that is chosen from the set is an element of
     * the list.
     */
    target = gnx_set_any(set);
    for (i = 0; i < size; i++) {
        if (target == list[i])
            break;
    }
    assert(i < size);

    free(list);
    gnx_destroy_set(set);
}

/**************************************************************************
 * delete elements
 *************************************************************************/

static void
delete(void)
{
    delete_empty();
    delete_non_member();
    delete_one();
    delete_random_dont_free_elements();
    delete_random_free_elements();
}

/* Delete elements from an empty set.
 */
static void
delete_empty(void)
{
    GnxSet *set;
    const unsigned int elem = (unsigned int)g_random_int();

    /* The set was configured to not release memory. */
    set = gnx_init_set();
    assert(0 == set->size);
    assert(!gnx_set_delete(set, &elem));
    assert(0 == set->size);
    gnx_destroy_set(set);

    /* The set was configured to release memory. */
    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    assert(0 == set->size);
    assert(!gnx_set_delete(set, &elem));
    assert(0 == set->size);
    gnx_destroy_set(set);
}

/* Delete an element that is not in a set.
 */
static void
delete_non_member(void)
{
    GnxSet *set;
    unsigned int elem, i, *list;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    set = gnx_init_set();

    for (i = 0; i < size; i++) {
        list[i] = i;
        assert(gnx_set_add(set, &(list[i])));
    }
    assert(size == set->size);

    elem = size;
    assert(!gnx_set_has(set, &elem));
    assert(!gnx_set_delete(set, &elem));
    assert(size == set->size);

    free(list);
    gnx_destroy_set(set);
}

/* Delete from a set that has one element.
 */
static void
delete_one(void)
{
    GnxSet *set;
    unsigned int *elem;

    /***********************************************************************
     * The set was configured to not release memory.
     **********************************************************************/

    set = gnx_init_set();
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = (unsigned int)g_random_int();
    assert(gnx_set_add(set, elem));
    assert(1 == set->size);

    assert(gnx_set_has(set, elem));
    assert(gnx_set_delete(set, elem));
    assert(0 == set->size);
    assert(!gnx_set_has(set, elem));

    free(elem);
    gnx_destroy_set(set);

    /***********************************************************************
     * The set was configured to release memory.
     **********************************************************************/

    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = (unsigned int)g_random_int();
    assert(gnx_set_add(set, elem));
    assert(1 == set->size);

    assert(gnx_set_has(set, elem));
    assert(gnx_set_delete(set, elem));
    assert(0 == set->size);
    assert(!gnx_set_has(set, elem));

    gnx_destroy_set(set);
}

/* Choose an element uniformly at random from a set and delete that element
 * from the set.  The set was configured to not release memory.
 */
static void
delete_random_dont_free_elements(void)
{
    GnxSet *set;
    unsigned int i, *list;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    set = gnx_init_set();

    for (i = 0; i < size; i++) {
        list[i] = i;
        assert(gnx_set_add(set, &(list[i])));
    }
    assert(size == set->size);

    i = (unsigned int)g_random_int_range(0, (int)size);
    assert(gnx_set_has(set, &(list[i])));
    assert(gnx_set_delete(set, &(list[i])));
    assert((size - 1) == set->size);
    assert(!gnx_set_has(set, &(list[i])));

    free(list);
    gnx_destroy_set(set);
}

/* Choose an element uniformly at random from a set and delete that element
 * from the set.  The set was configured to release memory.
 */
static void
delete_random_free_elements(void)
{
    GnxSet *set;
    unsigned int *elem, has_target, i, target;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    has_target = FALSE;  /* Have we chosen an element to delete? */

    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_set_add(set, elem));

        /* Choose only one element for deletion. */
        if (!has_target && g_random_boolean()) {
            target = *elem;
            has_target = TRUE;
        }

        /* By default, we use the first element as the target for deletion.
         * This is to prevent the case where none of the elements is chosen.
         */
        if (!i)
            target = *elem;
    }
    assert(size == set->size);

    assert(gnx_set_has(set, &target));
    assert(gnx_set_delete(set, &target));
    assert((size - 1) == set->size);
    assert(!gnx_set_has(set, &target));

    gnx_destroy_set(set);
}

/**************************************************************************
 * has an element
 *************************************************************************/

static void
has(void)
{
    has_empty();
    has_member();
    has_non_member();
}

/* Search an empty set.
 */
static void
has_empty(void)
{
    GnxSet *set;
    const unsigned int a = (unsigned int)g_random_int();

    /**********************************************************************
     * Do no release memory of elements.
     *********************************************************************/

    set = gnx_init_set();
    assert(0 == set->size);
    assert(!gnx_set_has(set, &a));
    assert(0 == set->size);

    gnx_destroy_set(set);

    /**********************************************************************
     * Release memory of elements.
     *********************************************************************/

    set = gnx_init_set_full(GNX_FREE_ELEMENTS);
    assert(0 == set->size);
    assert(!gnx_set_has(set, &a));
    assert(0 == set->size);

    gnx_destroy_set(set);
}

/* Search for elements that exist in a set.
 */
static void
has_member(void)
{
    GnxSet *set;
    unsigned int *elem, i, *list;
    const unsigned int size = (unsigned int)g_random_int_range(1, 21);

    /**********************************************************************
     * Do no release memory of elements.
     *********************************************************************/

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    set = gnx_init_set();

    for (i = 0; i < size; i++) {
        list[i] = i;
        assert(gnx_set_add(set, &(list[i])));
    }
    assert(size == set->size);

    for (i = 0; i < size; i++)
        assert(gnx_set_has(set, &(list[i])));

    assert(size == set->size);

    free(list);
    gnx_destroy_set(set);

    /**********************************************************************
     * Release memory of elements.
     *********************************************************************/

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    set = gnx_init_set_full(GNX_FREE_ELEMENTS);

    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = i;
        list[i] = *elem;
        assert(gnx_set_add(set, elem));
    }
    assert(size == set->size);

    for (i = 0; i < size; i++)
        assert(gnx_set_has(set, &(list[i])));

    assert(size == set->size);

    free(list);
    gnx_destroy_set(set);
}

/* Search for elements that do not exist in a set.
 */
static void
has_non_member(void)
{
    GnxSet *set;
    unsigned int elem, i, *list, *target;
    const unsigned int size = (unsigned int)g_random_int_range(1, 21);

    /**********************************************************************
     * Do no release memory of elements.
     *********************************************************************/

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    set = gnx_init_set();

    for (i = 0; i < size; i++) {
        list[i] = i;
        assert(gnx_set_add(set, &(list[i])));
    }
    assert(size == set->size);

    elem = size;
    assert(!gnx_set_has(set, &elem));
    assert(size == set->size);

    free(list);
    gnx_destroy_set(set);

    /**********************************************************************
     * Release memory of elements.
     *********************************************************************/

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    set = gnx_init_set_full(GNX_FREE_ELEMENTS);

    for (i = 0; i < size; i++) {
        target = (unsigned int *)malloc(sizeof(unsigned int));
        *target = i;
        list[i] = *target;
        assert(gnx_set_add(set, target));
    }
    assert(size == set->size);

    target = (unsigned int *)malloc(sizeof(unsigned int));
    *target = size;
    assert(!gnx_set_has(set, target));
    assert(size == set->size);

    free(list);
    free(target);
    gnx_destroy_set(set);
}

/**************************************************************************
 * iterator
 *************************************************************************/

static void
iter(void)
{
    iter_count();
    iter_empty();
    iter_one();
    iter_random();
}

/* Count the number of elements in a set.
 */
static void
iter_count(void)
{
    GnxSet *set;
    GnxSetIter iter;
    unsigned int *elem, i;
    const unsigned int size = (unsigned int)g_random_int_range(2, 51);

    set = gnx_init_set_full(GNX_FREE_ELEMENTS);

    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_set_add(set, elem));
    }

    gnx_set_iter_init(&iter, set);
    i = 0;
    while (gnx_set_iter_next(&iter, NULL))
        i++;

    assert(i == set->size);

    gnx_destroy_set(set);
}

/* Iterate over an empty set.
 */
static void
iter_empty(void)
{
    GnxSet *set;
    GnxSetIter iter;

    set = gnx_init_set();
    assert(0 == set->size);

    gnx_set_iter_init(&iter, set);
    assert(!gnx_set_iter_next(&iter, NULL));

    gnx_destroy_set(set);
}

/* Iterate over a set that has exactly one element.
 */
static void
iter_one(void)
{
    GnxSet *set;
    GnxSetIter iter;
    unsigned int a = (unsigned int)g_random_int();
    unsigned int elem;

    set = gnx_init_set();
    assert(gnx_set_add(set, &a));
    assert(1 == set->size);

    gnx_set_iter_init(&iter, set);
    assert(gnx_set_iter_next(&iter, &elem));
    assert(elem == a);
    assert(!gnx_set_iter_next(&iter, NULL));

    gnx_destroy_set(set);
}

/* Iterate over a set that has a random number of elements.
 */
static void
iter_random(void)
{
    GnxSet *set;
    GnxSetIter iter;
    unsigned int elem, i, j, *list, unique;
    const unsigned int size = (3u << (GNX_DEFAULT_EXPONENT - 2)) - 1;

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    set = gnx_init_set();

    /* Generate a bunch of unique random integers.  The size of the list is
     * chosen such that we do not trigger a resize of the set.  Here we assume
     * that a resize of the set will not be triggered provided that the number
     * of elements inserted into the set is kept below 3 * 2^(k-2), where k is
     * the exponent that is used to compute the number of buckets.  In other
     * words, a resize will not be triggered provided that the load factor is
     * kept below the threshold of 3/4.
     */
    for (i = 0; i < size; i++) {
        unique = FALSE;
        do {
            elem = (unsigned int)g_random_int();
            for (j = 0; j < i; j++) {
                if (elem == list[j])
                    break;
            }
            if (j >= i)
                unique = TRUE;
        } while (!unique);

        list[i] = elem;
        assert(gnx_set_add(set, &(list[i])));
    }
    assert(size == set->size);

    /* Check that all the elements that were inserted into the set are the
     * elements of the list.
     */
    gnx_set_iter_init(&iter, set);
    while (gnx_set_iter_next(&iter, &elem)) {
        for (i = 0; i < size; i++) {
            if (elem == list[i])
                break;
        }
        assert(i < size);
    }

    free(list);
    gnx_destroy_set(set);
}

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
    assert(GNX_FREE_ELEMENTS == set->free_elem);
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

    g_test_add_func("/set/add", add);
    g_test_add_func("/set/any", any);
    g_test_add_func("/set/delete", delete);
    g_test_add_func("/set/has", has);
    g_test_add_func("/set/iter", iter);
    g_test_add_func("/set/new", new);

    return g_test_run();
}
