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
static void add_no_memory_resize_dict_delete_tail(void);
static void add_one(void);
static void add_resize_bucket(void);
static void add_resize_dict(void);

/* delete elements */
static void delete_bucket_inbetween(void);
static void delete_bucket_tail(void);
static void delete_empty(void);
static void delete_non_member(void);
static void delete_one(void);
static void delete_random_dont_free_key_value(void);
static void delete_random_free_key_value(void);

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
    add_no_memory_resize_dict_delete_tail();
    add_one();
    add_resize_bucket();
    add_resize_dict();
}

/* Add a duplicate element to a dictionary.
 */
static void
add_duplicate(void)
{
    double *value1, *value2;
    GnxDict *dict;
    unsigned int *key1, *key2;

    /**********************************************************************
     * Do not release memory of elements.
     *********************************************************************/

    key1 = (unsigned int *)malloc(sizeof(unsigned int));
    key2 = (unsigned int *)malloc(sizeof(unsigned int));
    value1 = (double *)malloc(sizeof(double));
    value2 = (double *)malloc(sizeof(double));
    *key1 = (unsigned int)g_random_int();
    *key2 = *key1;
    *value1 = (double)g_random_double();
    *value2 = (double)g_random_double() + 1.0;

    dict = gnx_init_dict();
    assert(gnx_dict_add(dict, key1, value1));
    assert(*key1 == *key2);
    assert(!gnx_dict_add(dict, key2, value2));
    assert(1 == dict->size);

    free(key1);
    free(key2);
    free(value1);
    free(value2);
    gnx_destroy_dict(dict);

    /**********************************************************************
     * Release memory of elements.
     *********************************************************************/

    key1 = (unsigned int *)malloc(sizeof(unsigned int));
    key2 = (unsigned int *)malloc(sizeof(unsigned int));
    value1 = (double *)malloc(sizeof(double));
    value2 = (double *)malloc(sizeof(double));
    *key1 = (unsigned int)g_random_int();
    *key2 = *key1;
    *value1 = (double)g_random_double();
    *value2 = (double)g_random_double() + 1.0;

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    assert(gnx_dict_add(dict, key1, value1));
    assert(*key1 == *key2);
    assert(!gnx_dict_add(dict, key2, value2));
    assert(1 == dict->size);

    free(key2);
    free(value2);
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

/* Add enough elements to trigger a resize of a dictionary.  When the resizing
 * fails, we must recover by removing the tail entry of the bucket that was
 * last modified.
 */
static void
add_no_memory_resize_dict_delete_tail(void)
{
#ifdef GNX_ALLOC_TEST
    double *value;
    GnxDict *dict;
    int alloc_size;
    unsigned int i, *key, n;
    const unsigned int a = 3007121345;  /* The a parameter. */
    const unsigned int c = 19788844;    /* The c parameter. */
    const unsigned int target = 83;

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
     *
     * With the above pre-determined values for the a and c parameters,
     * inserting elements with keys in the range [0, n - 1] will result in
     * at least one bucket that has 6 entries.  In particular, each of the
     * following keys will map to the bucket with index 14:
     *
     * 33, 43, 53, 63, 73, 83
     *
     * Instead of inserting the key 83, we substitute in another key value.
     */
    n = (3 * (1u << (dict->k - 2))) - 1;
    for (i = 0; i < n; i++) {
        key = (unsigned int *)malloc(sizeof(unsigned int));
        *key = i;
        if (target == i)
            *key = n;
        value = (double *)malloc(sizeof(double));
        *value = (double)g_random_double();
        assert(gnx_dict_add(dict, key, value));
    }
    assert(n == dict->size);

    /* Now add another key/value pair to trigger a resize of the dictionary. */
    key = (unsigned int *)malloc(sizeof(unsigned int));
    *key = target;
    value = (double *)malloc(sizeof(double));
    *value = (double)g_random_double();
    alloc_size = GNX_ALLOC_BUCKET_NODE_SIZE;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_dict_add(dict, key, value));
    assert(ENOMEM == errno);
    assert(n == dict->size);

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
    double *value;
    GnxDict *dict;
    unsigned int *key;

    /**********************************************************************
     * Do not release the memory of key and value.
     *********************************************************************/

    key = (unsigned int *)malloc(sizeof(unsigned int));
    value = (double *)malloc(sizeof(double));
    *key = (unsigned int)g_random_int();
    *value = (double)g_random_double();

    dict = gnx_init_dict();
    assert(0 == dict->size);
    assert(gnx_dict_add(dict, key, value));
    assert(1 == dict->size);

    free(key);
    free(value);
    gnx_destroy_dict(dict);

    /**********************************************************************
     * Release the memory of key and value.
     *********************************************************************/

    key = (unsigned int *)malloc(sizeof(unsigned int));
    value = (double *)malloc(sizeof(double));
    *key = (unsigned int)g_random_int();
    *value = (double)g_random_double();

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    assert(0 == dict->size);
    assert(gnx_dict_add(dict, key, value));
    assert(1 == dict->size);

    gnx_destroy_dict(dict);
}

/* Add enough elements to trigger a resize of a bucket.  The dictionary was
 * configured to release the memory of its key/value pairs.
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
 * delete elements
 *************************************************************************/

static void
delete(void)
{
    delete_bucket_inbetween();
    delete_bucket_tail();
    delete_empty();
    delete_non_member();
    delete_one();
    delete_random_dont_free_key_value();
    delete_random_free_key_value();
}

/* Within a dictionary, we delete an element that is sandwiched between the
 * first and last elements of a bucket.
 */
static void
delete_bucket_inbetween(void)
{
    double *value;
    GnxDict *dict;
    unsigned int i, *key, n, target;
    const unsigned int a = 3007121345;  /* the a parameter */
    const unsigned int c = 19788844;    /* the c parameter */
    const unsigned int list[4] = {43, 53, 63, 73};
    const unsigned int size = 4;

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
     *
     * With the above pre-determined values for the a and c parameters,
     * inserting elements with keys in the range [0, n - 1] will result in
     * at least one bucket that has 6 entries.  In particular, each of the
     * following keys will map to the bucket with index 14:
     *
     * 33, 43, 53, 63, 73, 83.
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

    /* As noted above, each of the keys 33, 43, 53, 63, 73, 83 maps to the
     * bucket with index 14.  The entry with key 33 is the head of the bucket
     * and the entry with key 83 is the tail of the bucket.  Choose any one of
     * the keys 43, 53, 63, 73 uniformly at random and then remove it from the
     * dictionary.
     */
    i = (unsigned int)g_random_int_range(0, (int)size);
    target = list[i];
    assert(gnx_dict_delete(dict, &target));
    assert(!gnx_dict_has(dict, &target));
    assert((n - 1) == dict->size);
    for (i = 0; i < size; i++) {
        if (list[i] == target)
            continue;
        assert(gnx_dict_has(dict, &(list[i])));
    }

    gnx_destroy_dict(dict);
}

/* Within a dictionary, we delete the tail of a bucket that has multiple
 * elements.
 */
static void
delete_bucket_tail(void)
{
    double *value;
    GnxDict *dict;
    unsigned int i, *key, n, target;
    const unsigned int a = 279137349;  /* the a parameter */
    const unsigned int c = 28479211;   /* the c parameter */

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
     *
     * With the above pre-determined values for the a and c parameters,
     * inserting elements with keys in the range [0, n - 1] will result in
     * at least one bucket that has multiple entries.  In particular, each of
     * the keys 1 and 78 will map to the bucket with index 9.
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

    /* As noted above, each of the keys 1 and 78 maps to the bucket with index
     * 9.  Since we first inserted the key 1 followed by the key 78, then the
     * entry with key 1 is the head of the bucket with index 9 and the entry
     * with key 78 is the tail of the bucket.
     */
    target = 78;
    assert(gnx_dict_delete(dict, &target));
    assert((n - 1) == dict->size);

    gnx_destroy_dict(dict);
}

/* Delete elements from an empty dictionary.
 */
static void
delete_empty(void)
{
    GnxDict *dict;
    const unsigned int key = (unsigned int)g_random_int();

    /* The dictionary was configured to not release memory of key and value. */
    dict = gnx_init_dict();
    assert(0 == dict->size);
    assert(!gnx_dict_delete(dict, &key));
    assert(0 == dict->size);
    gnx_destroy_dict(dict);

    /* The dictionary was configured to release memory of key and value. */
    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    assert(0 == dict->size);
    assert(!gnx_dict_delete(dict, &key));
    assert(0 == dict->size);
    gnx_destroy_dict(dict);
}

/* Delete an element that is not in a dictionary.
 */
static void
delete_non_member(void)
{
    double *value;
    GnxDict *dict;
    unsigned int i, k, *key;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

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
    assert(!gnx_dict_delete(dict, &k));
    assert(size == dict->size);

    free(key);
    free(value);
    gnx_destroy_dict(dict);
}

/* Delete from a dictionary that has one element.
 */
static void
delete_one(void)
{
    double *value;
    GnxDict *dict;
    unsigned int *key;

    /***********************************************************************
     * The dictionary was configured to not release memory of key and value.
     **********************************************************************/

    dict = gnx_init_dict();
    key = (unsigned int *)malloc(sizeof(unsigned int));
    *key = (unsigned int)g_random_int();
    value = (double *)malloc(sizeof(double));
    *value = (double)g_random_double();
    assert(gnx_dict_add(dict, key, value));
    assert(1 == dict->size);

    assert(gnx_dict_has(dict, key));
    assert(gnx_dict_delete(dict, key));
    assert(0 == dict->size);

    free(key);
    free(value);
    gnx_destroy_dict(dict);

    /***********************************************************************
     * The dictionary was configured to release memory of key and value.
     **********************************************************************/

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    key = (unsigned int *)malloc(sizeof(unsigned int));
    *key = (unsigned int)g_random_int();
    value = (double *)malloc(sizeof(double));
    *value = (double)g_random_double();
    assert(gnx_dict_add(dict, key, value));
    assert(1 == dict->size);

    assert(gnx_dict_has(dict, key));
    assert(gnx_dict_delete(dict, key));
    assert(0 == dict->size);

    gnx_destroy_dict(dict);
}

/* Choose a key/value pair uniformly at random from a dictionary and delete
 * that key/value pair from the dictionary.  The dictionary was configured to
 * not release memory of the keys and values.
 */
static void
delete_random_dont_free_key_value(void)
{
    double *value;
    GnxDict *dict;
    unsigned int i, *key;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    key = (unsigned int *)malloc(sizeof(unsigned int) * size);
    value = (double *)malloc(sizeof(double) * size);
    dict = gnx_init_dict();

    for (i = 0; i < size; i++) {
        key[i] = i;
        value[i] = (double)g_random_double();
        assert(gnx_dict_add(dict, &(key[i]), &(value[i])));
    }
    assert(size == dict->size);

    i = (unsigned int)g_random_int_range(0, (int)size);
    assert(gnx_dict_has(dict, &(key[i])));
    assert(gnx_dict_delete(dict, &(key[i])));
    assert((size - 1) == dict->size);

    free(key);
    free(value);
    gnx_destroy_dict(dict);
}

/* Choose a key/value pair uniformly at random from a dictionary and delete
 * that key/value pair from the dictionary.  The dictionary was configured to
 * release memory of keys and values.
 */
static void
delete_random_free_key_value(void)
{
    double *value;
    GnxDict *dict;
    int has_target;
    unsigned int i, *key, target;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);
    has_target = FALSE;  /* Have we chosen an element to delete? */

    for (i = 0; i < size; i++) {
        key = (unsigned int *)malloc(sizeof(unsigned int));
        *key = (unsigned int)g_random_int();
        value = (double *)malloc(sizeof(double));
        *value = (double)g_random_double();
        assert(gnx_dict_add(dict, key, value));

        /* Choose only one element for deletion. */
        if (!has_target && g_random_boolean()) {
            target = *key;
            has_target = TRUE;
        }

        /* By default, we use the first element as the target for deletion.
         * This is to prevent the case where none of the elements is chosen.
         */
        if (!i)
            target = *key;
    }
    assert(size == dict->size);

    assert(gnx_dict_has(dict, &target));
    assert(gnx_dict_delete(dict, &target));
    assert((size - 1) == dict->size);

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

/* Search an empty dictionary.
 */
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

/* Search for elements that exist in a dictionary.
 */
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

/* Search for elements that do not exist in a dictionary.
 */
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

/* Count the number of elements in a dictionary.
 */
static void
iter_count(void)
{
    double *value;
    GnxDict *dict;
    GnxDictIter iter;
    unsigned int i, *key;
    const unsigned int size = (unsigned int)g_random_int_range(2, 51);

    dict = gnx_init_dict_full(GNX_FREE_KEYS, GNX_FREE_VALUES);

    for (i = 0; i < size; i++) {
        key = (unsigned int *)malloc(sizeof(unsigned int));
        *key = (unsigned int)g_random_int();
        value = (double *)malloc(sizeof(double));
        *value = (double)g_random_double();
        assert(gnx_dict_add(dict, key, value));
    }

    gnx_dict_iter_init(&iter, dict);
    i = 0;
    while (gnx_dict_iter_next(&iter, NULL, NULL))
        i++;

    assert(i == dict->size);

    gnx_destroy_dict(dict);
}

/* Iterate over an empty dictionary.
 */
static void
iter_empty(void)
{
    GnxDict *dict;
    GnxDictIter iter;

    dict = gnx_init_dict();
    assert(0 == dict->size);

    gnx_dict_iter_init(&iter, dict);
    assert(!gnx_dict_iter_next(&iter, NULL, NULL));

    gnx_destroy_dict(dict);
}

/* Iterate over a dictionary that has exactly one element.
 */
static void
iter_one(void)
{
    double v = (double)g_random_double();
    GnxDict *dict;
    GnxDictIter iter;
    unsigned int k = (unsigned int)g_random_int();
    unsigned int key;

    dict = gnx_init_dict();
    assert(gnx_dict_add(dict, &k, &v));
    assert(1 == dict->size);

    gnx_dict_iter_init(&iter, dict);
    assert(gnx_dict_iter_next(&iter, &key, NULL));
    assert(key == k);
    assert(!gnx_dict_iter_next(&iter, NULL, NULL));

    gnx_destroy_dict(dict);
}

/* Iterate over a dictionary that has a random number of elements.
 */
static void
iter_random(void)
{
    double *listv, value;
    GnxDict *dict;
    GnxDictIter iter;
    int unique;
    unsigned int i, j, k, *listk;
    const unsigned int size = (3u << (GNX_DEFAULT_EXPONENT - 2)) - 1;

    listk = (unsigned int *)malloc(sizeof(unsigned int) * size);
    listv = (double *)malloc(sizeof(double) * size);
    dict = gnx_init_dict();

    /* Generate a bunch of unique random integers.  The size of the list is
     * chosen such that we do not trigger a resize of the dictionary.  Here we
     * assume that a resize of the dictionary will not be triggered provided
     * that the number of elements inserted into the dictionary is kept below
     * 3 * 2^(k-2), where k is the exponent that is used to compute the number
     * of buckets.  In other words, a resize will not be triggered provided
     * that the load factor is kept below the threshold of 3/4.
     */
    for (i = 0; i < size; i++) {
        unique = FALSE;
        do {
            k = (unsigned int)g_random_int();
            for (j = 0; j < i; j++) {
                if (k == listk[j])
                    break;
            }
            if (j >= i)
                unique = TRUE;
        } while (!unique);

        listk[i] = k;
        listv[i] = (double)g_random_double();
        assert(gnx_dict_add(dict, &(listk[i]), &(listv[i])));
    }
    assert(size == dict->size);

    /* Check that all the elements that were inserted into the dictionary are
     * the elements of the list.
     */
    gnx_dict_iter_init(&iter, dict);
    while (gnx_dict_iter_next(&iter, &k, &value)) {
        for (i = 0; i < size; i++) {
            if (k == listk[i])
                break;
        }
        assert(i < size);
    }

    free(listk);
    free(listv);
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

/* A dictionary with default settings.
 */
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

/* A dictionary with custom settings.
 */
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

/* Test under low-memory scenarios.
 */
static void
new_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    int alloc_size;

    /* Cannot allocate memory for a dictionary. */
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
    g_test_add_func("/dict/delete", delete);
    g_test_add_func("/dict/has", has);
    g_test_add_func("/dict/iter", iter);
    g_test_add_func("/dict/new", new);

    return g_test_run();
}
