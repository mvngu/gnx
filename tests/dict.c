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
#include <stdlib.h>

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
static void add_resize_bucket(void);
static void add_resize_dict(void);

/* has an element */
static void has_empty(void);
static void has_member(void);
static void has_non_member(void);

/* new: create and destroy */
static void new_dict(void);
static void new_dict_full(void);
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
    add_resize_bucket();
    add_resize_dict();
}

/* Add a duplicate element to a dictionary.
 */
static void
add_duplicate(void)
{
    double value1 = (double)g_random_double();
    double value2 = (double)g_random_double() + 1.0;
    GnxDict *dict;
    unsigned int key1 = (unsigned int)g_random_int();
    unsigned int key2 = key1;

    dict = gnx_init_dict();
    assert(gnx_dict_add(dict, &key1, &value1));
    assert(key1 == key2);
    assert(!gnx_dict_add(dict, &key2, &value2));
    assert(1 == dict->size);

    gnx_destroy_dict(dict);
}

/* Add elements and allow the dictionary to release the memory of the keys and
 * values.
 */
static void
add_free(void)
{
    double *value;
    GnxDict *dict;
    int unique;
    unsigned int i, j, *key, *list;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    assert(0 == dict->size);

    /* A vector of unique keys. */
    list = (unsigned int *)malloc(sizeof(unsigned int) * size);

    for (i = 0; i < size; i++) {
        /* Generate a unique key. */
        key = (unsigned int *)malloc(sizeof(unsigned int));
        unique = FALSE;
        while (!unique) {
            *key = (unsigned int)g_random_int();
            for (j = 0; j < i; j++) {
                if (*key == list[j])
                    break;
            }
            if (i == j)
                unique = TRUE;
        }
        list[i] = *key;

        value = (double *)malloc(sizeof(double));
        *value = (double)g_random_double();
        assert(gnx_dict_add(dict, key, value));
    }
    assert(size == dict->size);

    free(list);
    gnx_destroy_dict(dict);
}

/* Test the function gnx_dict_add() in low-memory scenarios.
 */
static void
add_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    double *value;
    GnxDict *dict;
    int alloc_size;
    unsigned int i, *key, n;
    const unsigned int a = 3968615651;  /* the a parameter */
    const unsigned int c = 24242974;    /* the c parameter */

    /* Initialize the dictionary to have a pair of pre-determined values for
     * its a and c parameters.
     */
    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    dict->a = a;
    dict->c = c;

    /* Insert just enough elements such that a resize is not triggered.  We
     * assume that the load factor is 3/4.  If n is the number of entries in
     * the dictionary and m is the number of buckets, then we will not trigger
     * a resize provided that
     *
     *  n     3
     * --- < ---
     *  m     4
     *
     * The number of buckets m = 2^k is a power of two.  Solving the inequality
     * for n yields n < 3 * 2^(k-2).  Thus we will not trigger a resize
     * provided that we insert at most (3 * 2^(k-2) - 1) elements.
     */
    n = (3 * (1u << (dict->k - 2))) - 1;
    for (i = 0; i < n; i++) {
        key = (unsigned int *)malloc(sizeof(unsigned int));
        *key = i;
        value = (double *)malloc(sizeof(double));
        *value = (double)g_random_double();
        assert(gnx_dict_add(dict, key, value));
    }
    assert(n == dict->size);

    /* A new key/value pair that will trigger a resize. */
    key = (unsigned int *)malloc(sizeof(unsigned int));
    *key = n;
    value = (double *)malloc(sizeof(double));
    *value = (double)g_random_double();

    /* Cannot allocate memory for a bucket to hold a key/value pair. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_dict_add(dict, key, value));
    assert(ENOMEM == errno);

    /* Cannot allocate memory for the nodes of a bucket. */
    alloc_size = 1;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_dict_add(dict, key, value));
    assert(ENOMEM == errno);

    /* Cannot allocate memory for a key/value pair. */
    alloc_size = GNX_ALLOC_BUCKET_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_dict_add(dict, key, value));
    assert(ENOMEM == errno);

    /* In resizing a dictionary, cannot allocate memory for a new bucket
     * array.
     */
    alloc_size += GNX_ALLOC_BUCKET_NODE_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_dict_add(dict, key, value));
    assert(ENOMEM == errno);

    /* In resizing a dictionary, cannot allocate memory for a bucket of a
     * bucket array.
     */
    alloc_size++;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_dict_add(dict, key, value));
    assert(ENOMEM == errno);

    /* In resizing a dictionary, cannot allocate memory for an entry of
     * a bucket.
     */
    alloc_size += GNX_ALLOC_BUCKET_NODE_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_dict_add(dict, key, value));
    assert(ENOMEM == errno);

    /* Cannot allocate memory to resize the whole dictionary. */
    assert(10 < (n - 1));
    alloc_size += (1 + GNX_ALLOC_BUCKET_NODE_SIZE) * 10;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_dict_add(dict, key, value));
    assert(ENOMEM == errno);

    free(key);
    free(value);
    gnx_destroy_dict(dict);
    gnx_alloc_reset_limit();
#endif
}

/* Add one element to an empty dictionary.
 */
static void
add_one(void)
{
    double *v, value;
    GnxDict *dict;
    unsigned int *k, key;

    /**********************************************************************
     * Do not release the memory of key and value.
     *********************************************************************/

    dict = gnx_init_dict();
    assert(0 == dict->size);
    key = (unsigned int)g_random_int();
    value = (double)g_random_double();
    assert(gnx_dict_add(dict, &key, &value));
    assert(1 == dict->size);
    gnx_destroy_dict(dict);

    /**********************************************************************
     * Release the memory of key and value.
     *********************************************************************/

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    assert(0 == dict->size);
    k = (unsigned int *)malloc(sizeof(unsigned int));
    *k = (unsigned int)g_random_int();
    v = (double *)malloc(sizeof(double));
    *v = (double)g_random_double();
    assert(gnx_dict_add(dict, k, v));
    assert(1 == dict->size);
    gnx_destroy_dict(dict);
}

/* Add enough elements to trigger a resize of a bucket.
 */
static void
add_resize_bucket(void)
{
    double *value;
    GnxDict *dict;
    unsigned int i, *key;
    const unsigned int a = 5047397;   /* The a parameter. */
    const unsigned int c = 11657812;  /* The c parameter. */
    const unsigned int n = 6;         /* How many elements to insert. */

    /* Initialize the dictionary to have a pair of pre-determined values for
     * its a and c parameters.
     */
    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    dict->a = a;
    dict->c = c;

    /* Insert elements to trigger a resize of bucket, but does not trigger a
     * resize of the dictionary.  With the given values for the a and c
     * parameters, the keys with values in the range [0, 4] all map to the same
     * bucket index of 0.  However, the key value of 5 maps to the bucket
     * index of 1.
     */
    for (i = 0; i < n; i++) {
        key = (unsigned int *)malloc(sizeof(unsigned int));
        *key = i;
        value = (double *)malloc(sizeof(double));
        *value = (double)g_random_double();
        assert(gnx_dict_add(dict, key, value));
    }
    assert(n == dict->size);

    gnx_destroy_dict(dict);
}

/* Add enough elements to trigger a resize of a dictionary.
 */
static void
add_resize_dict(void)
{
    double *value;
    GnxDict *dict;
    unsigned int i, *key;

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    assert(0 == dict->size);
    assert(GNX_DEFAULT_ALLOC_SIZE == dict->capacity);

    /* Add a bunch of elements to trigger a resize of the dictionary. */
    for (i = 0; i < GNX_DEFAULT_ALLOC_SIZE; i++) {
        key = (unsigned int *)malloc(sizeof(unsigned int));
        *key = i;
        value = (double *)malloc(sizeof(double));
        *value = (double)g_random_double();
        assert(gnx_dict_add(dict, key, value));
    }

    /* The dictionary has been resized because the capacity has doubled. */
    assert(GNX_DEFAULT_ALLOC_SIZE == dict->size);
    assert((GNX_DEFAULT_ALLOC_SIZE << 1) == dict->capacity);
    assert(GNX_DEFAULT_EXPONENT + 1 == dict->k);
    assert(dict->b - dict->k == dict->d);
    assert(dict->a <= UINT_MAX);
    assert(1 == dict->a % 2);
    assert(dict->c <= (1u << dict->d));

    gnx_destroy_dict(dict);
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

/* Search an empty dictionary. */
static void
has_empty(void)
{
    GnxDict *dict;
    const unsigned int key = (unsigned int)g_random_int();

    dict = gnx_init_dict();
    assert(0 == dict->size);
    assert(!gnx_dict_has(dict, &key));
    assert(0 == dict->size);

    gnx_destroy_dict(dict);
}

/* Search for elements that exist in a dictionary. */
static void
has_member(void)
{
    double *value;
    GnxDict *dict;
    unsigned int i, *key;
    const unsigned int size = (unsigned int)g_random_int_range(1, 21);

    key = (unsigned int *)malloc(sizeof(unsigned int) * size);
    value = (double *)malloc(sizeof(double) * size);
    dict = gnx_init_dict();

    for (i = 0; i < size; i++) {
        key[i] = i;
        value[i] = (double)g_random_double();
        assert(gnx_dict_add(dict, &(key[i]), &(value[i])));
    }
    assert(size == dict->size);

    for (i = 0; i < size; i++)
        assert(gnx_dict_has(dict, &(key[i])));

    assert(size == dict->size);

    free(key);
    free(value);
    gnx_destroy_dict(dict);
}

/* Search for elements that do not exist in a dictionary. */
static void
has_non_member(void)
{
    double *value;
    GnxDict *dict;
    unsigned int i, k, *key;
    const unsigned int size = (unsigned int)g_random_int_range(1, 21);

    key = (unsigned int *)malloc(sizeof(unsigned int) * size);
    value = (double *)malloc(sizeof(double) * size);
    dict = gnx_init_dict();

    for (i = 0; i < size; i++) {
        key[i] = i;
        value[i] = (double)g_random_double();
        assert(gnx_dict_add(dict, &(key[i]), &(value[i])));
    }
    assert(size == dict->size);

    k = size;
    assert(!gnx_dict_has(dict, &k));
    assert(size == dict->size);

    free(key);
    free(value);
    gnx_destroy_dict(dict);
}

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

    g_test_add_func("/dict/add", add);
    g_test_add_func("/dict/has", has);
    g_test_add_func("/dict/new", new);

    return g_test_run();
}
