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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "array.h"
#include "sanity.h"
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

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static int* gnx_i_has(const GnxSet *set,
                      const int *elem,
                      unsigned int *i,
                      unsigned int *j);
static inline unsigned int gnx_i_hash(const int *key,
                                      const unsigned int *a,
                                      const unsigned int *c,
                                      const unsigned int *d);
static int gnx_i_resize(GnxSet *set);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief Whether a set has a given element.
 *
 * @param set We want to search in this set.
 * @param elem Search the set for this element.
 * @param i This will hold the bucket index to which the element is hashed.  If
 *        you do not want the bucket index, pass @c NULL.
 * @param j This will hold the entry index within the bucket where the element
 *        resides.  If you do not want the entry index, pass @c NULL.
 * @return A pointer to the given element if the element is in the set; @c NULL
 *         otherwise.  We also return @c NULL if the set is empty.
 */
static int*
gnx_i_has(const GnxSet *set,
          const int *elem,
          unsigned int *i,
          unsigned int *j)
{
    GnxArray *bucket;
    unsigned int idx, jdx;

    idx = gnx_i_hash(elem, &(set->a), &(set->c), &(set->d));
    if (i)
        *i = idx;

    if (!set->size)
        return NULL;

    bucket = (GnxArray *)(set->bucket[idx]);
    if (!bucket)
        return NULL;

    /* Linear search through the entries of the bucket. */
    for (jdx = 0; jdx < bucket->size; jdx++) {
        if (*elem == *(bucket->cell[jdx])) {
            if (j)
                *j = jdx;
            return bucket->cell[jdx];
        }
    }

    return NULL;
}

/**
 * @brief Hash of the given key.
 *
 * @param key We want to hash this key.
 * @param a A parameter of the hash function.  This is an odd integer.
 * @param c Another parameter of the hash function.  This is an integer.
 * @param d Still another parameter of the hash function.  This is the
 *        difference between the number of bits in the representation of an
 *        <tt>unsigned int</tt> type, and the exponent that is used to compute
 *        the number of buckets.
 * @return The hash of the given key.  This is also the index of a bucket in
 *         a set.
 */
static inline unsigned int
gnx_i_hash(const int *key,
           const unsigned int *a,
           const unsigned int *c,
           const unsigned int *d)
{
    unsigned int x = (unsigned int)(*key);
    /* Note that the numerator ax + c can wrap around because each operand is
     * an unsigned int.  We expect the wrap around because the numerator is
     * meant to be reduced modulo 2^b, where b is the number of bits in the
     * representation of an unsigned int.  The wrapping behavior of arithmetic
     * with operands that are unsigned ints is equivalent to arithmetic modulo
     * 2^b.
     */
    return (((*a) * x) + (*c)) >> (*d);
}

/**
 * @brief Resize a set.
 *
 * The set is resized by doubling its current number of buckets.  Each element
 * in the set is also rehashed.
 *
 * @param set We want to resize this set.
 * @return Nonzero if the set was successfully resized; zero otherwise.  If we
 *         are unable to allocate memory, then we set @c errno to @c ENOMEM and
 *         return zero.
 */
static int
gnx_i_resize(GnxSet *set)
{
    GnxArray *new_bucket, *old_bucket;
    gnxptr *new_bucket_array;
    int *key;
    unsigned int i, idx, j, new_a, new_c, new_d, new_k, new_capacity;
    const unsigned int bucket_capacity = 2;

    errno = 0;
    new_k = set->k + 1;
    new_capacity = set->capacity << 1;
    g_assert(new_k <= set->b);
    g_assert((1u << new_k) == new_capacity);
    g_assert(new_capacity <= GNX_MAXIMUM_BUCKETS);
    new_d = set->b - new_k;

    /* The parameter a is part of the Woelfel universal family of hash
     * functions.  The parameter is an odd integer that is chosen uniformly at
     * random from the range [1, 2^b - 1].
     */
    do {
        new_a = (unsigned int)g_random_int();
    } while (!(new_a & 1));

    /* The parameter c is part of the Woelfel universal family of hash
     * functions.  The parameter is an integer that is chosen uniformly at
     * random from the range [0, 2^(b - k) - 1].
     */
    new_c = (unsigned int)g_random_int_range(0, 1 << new_d);

    new_bucket_array = (gnxptr *)calloc(new_capacity, sizeof(gnxptr));
    if (!new_bucket_array)
        goto cleanup;

    /* Rehash each entry. */
    for (i = 0; i < set->capacity; i++) {
        old_bucket = (GnxArray *)(set->bucket[i]);
        if (!old_bucket)
            continue;

        /* Rehash each entry j of the old bucket i.  The entry is then moved
         * to a new bucket.  Finally, we release the memory for the old bucket.
         */
        for (j = 0; j < old_bucket->size; j++) {
            key = old_bucket->cell[j];
            idx = gnx_i_hash(key, &new_a, &new_c, &new_d);
            if (!(new_bucket_array[idx])) {
                new_bucket
                    = gnx_init_array_full(&bucket_capacity, set->free_elem);
                if (!new_bucket)
                    goto cleanup;
                new_bucket_array[idx] = new_bucket;
            }
            new_bucket = (GnxArray *)(new_bucket_array[idx]);
            assert(gnx_array_append(new_bucket, key));
        }
        gnx_destroy_array(old_bucket);
        set->bucket[i] = NULL;
    }
    free(set->bucket);

    /* Set the new parameters of the set. */
    set->k = new_k;
    set->capacity = new_capacity;
    set->bucket = new_bucket_array;
    set->d = new_d;
    set->a = new_a;
    set->c = new_c;

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (new_bucket_array) {
        for (i = 0; i < new_capacity; i++) {
            gnx_destroy_array((GnxArray *)(new_bucket_array[i]));
            new_bucket_array[i] = NULL;
        }
        free(new_bucket_array);
    }
    return GNX_FAILURE;
}

/**************************************************************************
 * public interface
 *************************************************************************/

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
            if (bucket && bucket->cell) {
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
 * @return An initialized set whose number of buckets is determined by
 *         #GNX_DEFAULT_ALLOC_SIZE.  When you no longer need the set, you must
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

/**
 * @brief Inserts an element into a set.
 *
 * @param set We want to insert an element into this set.
 * @param elem Add this element to the set.  It is your responsibility to
 *        ensure that this element exists for the duration of the set.
 * @return Nonzero if the element is successfully inserted into the set; zero
 *         otherwise.  We also return zero if the element is already in the
 *         set.  For an insertion to be successful, the given element must not
 *         already be in the set.
 */
int
gnx_set_add(GnxSet *set,
            int *elem)
{
    GnxArray *bucket;
    int created_bucket = FALSE;  /* Whether a new bucket has been created. */
    unsigned int i;
    const unsigned int capacity = 2;

    errno = 0;
    gnx_i_check_set(set);
    g_return_val_if_fail(elem, GNX_FAILURE);

    if (gnx_i_has(set, elem, &i, NULL))
        return GNX_FAILURE;

    /* Initialize a new empty bucket, which is represented as an array.  Note
     * that we do not allow the underlying array to release the memory of its
     * elements.  The destruction and release of memory must be handled by the
     * function gnx_destroy_set().
     */
    if (!(set->bucket[i])) {
        bucket = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS);
        if (!bucket)
            goto cleanup;
        set->bucket[i] = bucket;
        created_bucket = 1;
    }

    bucket = (GnxArray *)(set->bucket[i]);
    if (!gnx_array_append(bucket, elem))
        goto cleanup;

    /* Possibly resize the set by doubling the current number of buckets.  The
     * threshold on the load factor of the set is 3/4.  A resize will not be
     * triggered provided that
     *
     *   n      3
     * ----- < ---
     *  2^k     4
     *
     * where m = 2^k is the number of buckets.  If b is the number of bits in
     * the representation of an unsigned int type, then k <= b.  Solving the
     * inequality for n yields
     *
     * n < 3 * (2^(k-2))
     */
    if ((set->size + 1) >= (3u << (set->k - 2))) {
        if (!gnx_i_resize(set)) {
            assert(gnx_array_delete_tail(bucket));
            goto cleanup;
        }
    }
    (set->size)++;

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (created_bucket) {
        gnx_destroy_array((GnxArray *)(set->bucket[i]));
        set->bucket[i] = NULL;
    }
    return GNX_FAILURE;
}

/**
 * @brief Chooses an element from a set.
 *
 * Note that this function does not randomly choose an element from a set.  The
 * function merely iterates over the set and returns as soon as an element is
 * found.
 *
 * @param set Choose an element from this set.
 * @return An element from the given set.  If the set is empty, then we return
 *         @c NULL.
 */
int*
gnx_set_any_element(GnxSet *set)
{
    GnxSetIter iter;
    gnxintptr elem;

    gnx_i_check_set(set);
    if (!(set->size))
        return NULL;

    gnx_set_iter_init(&iter, set);
    assert(gnx_set_iter_next(&iter, &elem));

    return elem;
}

/**
 * @brief Removes an element from a set.
 *
 * @param set We want to remove an element from this set.
 * @param elem Remove this element from the given set.
 * @return Nonzero if the element was found and successfully removed from the
 *         set; zero otherwise.  We also return zero if the set is empty or the
 *         element is not in the set.
 */
int
gnx_set_delete(GnxSet *set,
               const int *elem)
{
    GnxArray *bucket;
    unsigned int i, j;

    gnx_i_check_set(set);
    g_return_val_if_fail(elem, GNX_FAILURE);

    if (!(set->size))
        return GNX_FAILURE;
    if (!gnx_i_has(set, elem, &i, &j))
        return GNX_FAILURE;

    bucket = (GnxArray *)(set->bucket[i]);
    assert(gnx_array_delete(bucket, &j));
    if (!(bucket->size)) {
        gnx_destroy_array(bucket);
        set->bucket[i] = NULL;
    }
    (set->size)--;

    return GNX_SUCCESS;
}

/**
 * @brief Whether a set has a given element.
 *
 * @param set We want to search this set.
 * @param elem Search the set for this element.
 * @return A pointer to the given element if the element is in the set; @c NULL
 *         otherwise.  We also return @c NULL if the set is empty.
 */
int*
gnx_set_has(const GnxSet *set,
            const int *elem)
{
    gnx_i_check_set(set);
    g_return_val_if_fail(elem, NULL);
    return gnx_i_has(set, elem, NULL, NULL);
}

/**
 * @brief Initializes an iterator over the elements of a set.
 *
 * @param iter Initialize this iterator.  The iterator is usually allocated on
 *        the runtime stack.
 * @param set We want to iterate over the elements of this set.
 */
void
gnx_set_iter_init(GnxSetIter *iter,
                  GnxSet *set)
{
    iter->bootstrap = TRUE;
    iter->set = set;
    iter->i = 0;
    iter->j = 0;
}

/**
 * @brief Iterate to the next element of a set.
 *
 * The order in which we iterate over the elements of a set is not the same as
 * the order in which the elements were inserted into the set.
 *
 * @param iter An iterator that has been initialized via the function
 *        gnx_set_iter_init().
 * @param elem This will hold the next element in the set.  If @c NULL, then we
 *        will ignore the next element.
 * @return Nonzero if we have not yet reached the end of the set; zero
 *         otherwise.  If zero, then we have reached the end of the set and the
 *         iterator is now invalid.  We also return zero if the set is empty.
 */
int
gnx_set_iter_next(GnxSetIter *iter,
                  gnxintptr *elem)
{
    GnxArray *bucket;
    int i;

    g_return_val_if_fail(iter, GNX_FAILURE);
    gnx_i_check_set(iter->set);

    /* If we are bootstrapping the process, then we look for the first element
     * to which we can iterate.
     */
    if (iter->bootstrap) {
        /* The set is empty. */
        if (!(iter->set->size))
            return GNX_FAILURE;

        /* The index of the first bucket with at least one entry. */
        i = -1;
        do {
            i++;
            bucket = (GnxArray *)(iter->set->bucket[i]);
        } while (!bucket);
        iter->i = (unsigned int)i;

        /* The first entry within the first non-empty bucket. */
        g_assert(bucket->size);
        iter->j = 0;
        if (elem)
            *elem = bucket->cell[iter->j];

        iter->bootstrap = FALSE;
        return GNX_SUCCESS;
    }

    /* We have already iterated over the first element of the set.  Start from
     * the current element and iterate to the next element of the set.
     */
    bucket = (GnxArray *)(iter->set->bucket[iter->i]);
    for ((iter->j)++; iter->j < bucket->size; (iter->j)++) {
        if (elem)
            *elem = bucket->cell[iter->j];
        return GNX_SUCCESS;
    }

    /* We have exhausted the current bucket.  Find the next bucket that has at
     * least one entry.
     */
    (iter->i)++;
    while (iter->i < (iter->set->capacity)) {
        bucket = (GnxArray *)(iter->set->bucket[iter->i]);
        if (!bucket) {
            (iter->i)++;
            continue;
        }

        g_assert(bucket->size);
        iter->j = 0;
        if (elem)
            *elem = bucket->cell[iter->j];
        return GNX_SUCCESS;
    }

    /* We have exhausted all buckets. */
    return GNX_FAILURE;
}
