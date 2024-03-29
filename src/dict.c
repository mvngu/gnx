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

#include <glib.h>

#include "array.h"
#include "dict.h"
#include "sanity.h"

/**
 * @file dict.h
 * @brief Dictionary with integer keys.
 *
 * A dictionary does not make its own copy of any entry that is inserted.
 * Rather, we copy the pointers for key and value that are passed in and store
 * those pointers.  It is your responsibility to ensure that any key/value pair
 * that is inserted into the dictionary exists for the duration of the
 * dictionary itself.
 *
 * Use either of the functions gnx_init_dict() or gnx_init_dict_full() to
 * initialize a new dictionary.  Destroy a dictionary via the function
 * gnx_destroy_dict().
 */

/**************************************************************************
 * internal data structures
 *************************************************************************/

/* @cond */
/* An entry of a bucket.  Each bucket entry is a key/value pair.
 */
typedef struct {
    unsigned int *key;  /* The key of a bucket entry. */
    gnxptr value;       /* The value of a bucket entry. */
} GnxNode;
/* @endcond */

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static int gnx_i_append_node(GnxArray *bucket,
                             unsigned int *key,
                             gnxptr value);
static GnxNode* gnx_i_has(const GnxDict *dict,
                          const unsigned int *key,
                          unsigned int *i,
                          unsigned int *j);
static inline unsigned int gnx_i_hash(const unsigned int *key,
                                      const unsigned int *a,
                                      const unsigned int *c,
                                      const unsigned int *d);
static int gnx_i_resize_dict(GnxDict *dict);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief Append a new entry to a bucket.
 *
 * We assume that the key of the entry is not already in the bucket.
 *
 * @param bucket We want to append a new entry to this bucket.
 * @param key The key of the new entry.  This key is assumed to be different
 *        from all other keys in the bucket.
 * @param value The value of the new entry.
 * @return Nonzero if we successfully appended the given entry to the bucket;
 *         zero otherwise.  If we are unable to allocate memory for the new
 *         entry, then we set @c errno to @c ENOMEM and return zero.
 */
static int
gnx_i_append_node(GnxArray *bucket,
                  unsigned int *key,
                  gnxptr value)
{
    GnxNode *node;

    errno = 0;
    node = (GnxNode *)malloc(sizeof(GnxNode));
    if (!node)
        goto cleanup;

    node->key = key;
    node->value = value;
    if (!gnx_array_append(bucket, node))
        goto cleanup;

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (node) {
        node->key = NULL;
        node->value = NULL;
        free(node);
        node = NULL;
    }
    return GNX_FAILURE;
}

/**
 * @brief Whether a dictionary has a given key.
 *
 * @param dict We want to search in this dictionary.
 * @param key Search the dictionary for this key.
 * @param i This will hold the bucket index to which the key is hashed.  If
 *        you do not want the bucket index, pass @c NULL.
 * @param j This will hold the entry index within the bucket where the
 *        key/value pair resides.  If you do not want the entry index, pass
 *        @c NULL.
 * @return A pointer to the found key/value pair if the given key is in the
 *         dictionary; @c NULL otherwise.  We also return @c NULL if the
 *         dictionary is empty.
 */
static GnxNode*
gnx_i_has(const GnxDict *dict,
          const unsigned int *key,
          unsigned int *i,
          unsigned int *j)
{
    GnxArray *bucket;
    GnxNode *node;
    unsigned int idx, jdx;

    idx = gnx_i_hash(key, &(dict->a), &(dict->c), &(dict->d));
    if (i)
        *i = idx;

    /* The dictionary is empty.  Therefore the given key is not in the
     * dictionary.
     */
    if (!dict->size)
        return NULL;

    /* The bucket to which the key hashes is empty.  Therefore the given key
     * is not in the dictionary.
     */
    bucket = (GnxArray *)(dict->bucket[idx]);
    if (!bucket)
        return NULL;

    /* Linear search through the entries of the bucket. */
    for (jdx = 0; jdx < bucket->size; jdx++) {
        node = (GnxNode *)(bucket->cell[jdx]);
        if (*key == *(node->key)) {
            if (j)
                *j = jdx;
            return node;
        }
    }

    return NULL;
}

/**
 * @brief Hash of the given key.
 *
 * @param key We want to hash this key.
 * @param a A parameter of the hash function.  This is an odd positive integer.
 * @param c Another parameter of the hash function.  This is a positive integer.
 * @param d Still another parameter of the hash function.  This is the
 *        difference between the number of bits in the representation of an
 *        <tt>unsigned int</tt> type, and the exponent that is used to compute
 *        the number of buckets.
 * @return The hash of the given key.  This is also the index of a bucket in
 *         a dictionary.
 */
static inline unsigned int
gnx_i_hash(const unsigned int *key,
           const unsigned int *a,
           const unsigned int *c,
           const unsigned int *d)
{
    /* Let x be the key value.  Note that the numerator ax + c can wrap around
     * because each operand is an unsigned int.  We expect the wrap around
     * because the numerator is meant to be reduced modulo 2^b, where b is the
     * number of bits in the representation of an unsigned int.  The wrapping
     * behavior of arithmetic with operands that are unsigned ints is
     * equivalent to arithmetic modulo 2^b.
     */
    return (((*a) * (*key)) + (*c)) >> (*d);
}

/**
 * @brief Resize a dictionary.
 *
 * The dictionary is resized by doubling its current number of buckets.  Each
 * element in the dictionary is also rehashed.
 *
 * @param dict We want to resize this dictionary.
 * @return Nonzero if the dictionary was successfully resized; zero otherwise.
 *         If we are unable to allocate memory, then we set @c errno to
 *         @c ENOMEM and return zero.
 */
static int
gnx_i_resize_dict(GnxDict *dict)
{
    GnxArray *new_bucket, *old_bucket;
    GnxNode *node;
    gnxptr *new_bucket_array;
    unsigned int i, idx, j, new_a, new_c, new_d, new_k, new_capacity;
    const unsigned int bucket_capacity = GNX_DEFAULT_BUCKET_SIZE;

    errno = 0;
    new_k = dict->k + 1;
    new_capacity = dict->capacity << 1;
    g_assert(new_k <= dict->b);
    g_assert((1u << new_k) == new_capacity);
    g_assert(new_capacity <= GNX_MAXIMUM_BUCKETS);
    new_d = dict->b - new_k;

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

    /* First, we only rehash each entry.  Each entry is rehashed and moved to
     * a new bucket.  If for any reason we are unable to rehash an entry, then
     * the content of the old buckets would remain unmodified.
     */
    for (i = 0; i < dict->capacity; i++) {
        old_bucket = (GnxArray *)(dict->bucket[i]);
        if (!old_bucket)
            continue;

        /* Rehash each entry j of the old bucket i.  The entry is then moved
         * to a new bucket.
         */
        for (j = 0; j < old_bucket->size; j++) {
            node = (GnxNode *)(old_bucket->cell[j]);
            idx = gnx_i_hash(node->key, &new_a, &new_c, &new_d);
            if (!(new_bucket_array[idx])) {
                new_bucket
                    = gnx_init_array_full(&bucket_capacity,
                                          GNX_DONT_FREE_ELEMENTS, GNX_POINTER);
                if (!new_bucket)
                    goto cleanup;

                new_bucket_array[idx] = new_bucket;
            }
            new_bucket = (GnxArray *)(new_bucket_array[idx]);
            if (!gnx_array_append(new_bucket, node))
                goto cleanup;
        }
    }

    /* Next, we destroy the old buckets.  We have successfully rehashed each
     * entry.  Now free the memory of the old buckets.  Note that each bucket
     * was configured to not release the memory of its elements.  So it is safe
     * to destroy each bucket as below.
     */
    for (i = 0; i < dict->capacity; i++) {
        old_bucket = (GnxArray *)(dict->bucket[i]);
        if (!old_bucket)
            continue;

        g_assert(!old_bucket->free_elem);
        gnx_destroy_array(old_bucket);
        dict->bucket[i] = NULL;
    }
    free(dict->bucket);

    /* Finally, we configure the new parameters of the dictionary. */
    dict->k = new_k;
    dict->capacity = new_capacity;
    dict->bucket = new_bucket_array;
    dict->d = new_d;
    dict->a = new_a;
    dict->c = new_c;

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (new_bucket_array) {
        for (i = 0; i < new_capacity; i++) {
            new_bucket = (GnxArray *)(new_bucket_array[i]);
            if (!new_bucket)
                continue;

            g_assert(!new_bucket->free_elem);
            gnx_destroy_array(new_bucket);
            new_bucket_array[i] = NULL;
        }
        free(new_bucket_array);
        new_bucket_array = NULL;
    }
    return GNX_FAILURE;
}

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Destroys a dictionary.
 *
 * The given dictionary must have been initialized by either of the functions
 * gnx_init_dict() or gnx_init_dict_full().
 *
 * @param dict We want to destroy this dictionary.
 */
void
gnx_destroy_dict(GnxDict *dict)
{
    GnxArray *bucket;
    GnxNode *node;
    int free_key, free_value;
    unsigned int i, j;

    if (!dict)
        return;
    if (dict->bucket) {
        free_key = GNX_FREE_KEYS & dict->free_key;
        free_value = GNX_FREE_VALUES & dict->free_value;

        for (i = 0; i < dict->capacity; i++) {
            bucket = (GnxArray *)(dict->bucket[i]);
            if (!bucket)
                continue;

            for (j = 0; j < bucket->size; j++) {
                node = (GnxNode *)(bucket->cell[j]);
                if (free_key)
                    free(node->key);
                if (free_value)
                    free(node->value);

                node->key = NULL;
                node->value = NULL;
                free(node);
            }
            g_assert(!bucket->free_elem);
            gnx_destroy_array(bucket);
            dict->bucket[i] = NULL;
        }
        free(dict->bucket);
        dict->bucket = NULL;
    }
    free(dict);
    dict = NULL;
}

/**
 * @brief Inserts a key/value pair into a dictionary.
 *
 * The dictionary or one of its buckets might possibly be resized to
 * accommodate the key/value pair.  If a bucket is resized, the new capacity
 * of the bucket must not exceed #GNX_MAXIMUM_ELEMENTS.  If the dictionary is
 * resized, then the bucket capacity (the number of possible buckets) of the
 * dictionary must not exceed #GNX_MAXIMUM_BUCKETS.
 *
 * @param dict We want to insert a key/value pair into this dictionary.
 * @param key Add this key (and its corresponding value) to the dictionary.  It
 *        is your responsibility to ensure that the key exists for the duration
 *        of the dictionary.
 * @param value Add this value (and its corresponding key) to the dictionary.
 *        It is your responsibility to ensure that the value exists for the
 *        duration of the dictionary.
 * @return Nonzero if the key/value pair is successfully inserted into the
 *         dictionary; zero otherwise.  We also return zero if the key is
 *         already in the dictionary.  For an insertion to be successful, the
 *         given key must not already be in the dictionary.  If we are unable
 *         to allocate memory, then we set @c errno to @c ENOMEM and return
 *         zero.
 */
int
gnx_dict_add(GnxDict *dict,
             unsigned int *key,
             gnxptr value)
{
    GnxArray *bucket;            /* A bucket of entries. */
    GnxNode *node;               /* An entry of a bucket. */
    int created_bucket = FALSE;  /* Whether a new bucket has been created. */
    unsigned int i, tail;
    const unsigned int capacity = GNX_DEFAULT_BUCKET_SIZE;

    errno = 0;
    gnx_i_check_dict(dict);
    g_return_val_if_fail(key, GNX_FAILURE);
    g_return_val_if_fail(value, GNX_FAILURE);

    if (gnx_i_has(dict, key, &i, NULL))
        return GNX_FAILURE;

    /* Initialize a new empty bucket, which is represented as an array of
     * nodes.  The destruction and release of memory must be handled by the
     * function gnx_destroy_dict().
     */
    if (!(dict->bucket[i])) {
        bucket = gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS,
                                     GNX_POINTER);
        if (!bucket)
            goto cleanup;

        dict->bucket[i] = bucket;
        created_bucket = TRUE;
    }

    /* Append the new entry to the bucket that has index i. */
    if (!gnx_i_append_node((GnxArray *)(dict->bucket[i]), key, value))
        goto cleanup;

    /* Possibly resize the dictionary by doubling the current number of
     * buckets.  The threshold on the load factor of the dictionary is 3/4.
     * A resize will not be triggered provided that
     *
     *   n      3
     * ----- < ---
     *  2^k     4
     *
     * where m = 2^k is the number of buckets and n counts the current total
     * number of entries.  If b is the number of bits in the representation of
     * an unsigned int type, then k <= b.  Solving the inequality for n yields
     *
     * n < 3 * (2^(k-2))
     */
    if ((dict->size + 1) >= (3u << (dict->k - 2))) {
        if (!gnx_i_resize_dict(dict)) {
            /* Remove the entry that was last added to the bucket.  In other
             * words, we remove the tail of the bucket.
             */
            bucket = (GnxArray *)(dict->bucket[i]);
            if (created_bucket) {
                g_assert(1 == bucket->size);
                tail = 0;
            } else {
                g_assert(bucket->size > 1);
                tail = bucket->size - 1;
            }

            node = (GnxNode *)(bucket->cell[tail]);
            node->key = NULL;
            node->value = NULL;
            free(node);
            g_assert(!bucket->free_elem);
            assert(gnx_array_delete_tail(bucket));
            goto cleanup;
        }
    }
    (dict->size)++;

    return GNX_SUCCESS;

cleanup:
    errno = ENOMEM;
    if (created_bucket) {
        bucket = (GnxArray *)(dict->bucket[i]);
        g_assert(0 == bucket->size);
        gnx_destroy_array(bucket);
        dict->bucket[i] = NULL;
    }
    return GNX_FAILURE;
}

/**
 * @brief Remove a key and its value from a dictionary.
 *
 * @param dict We want to update this dictionary.
 * @param key Remove this key (and its corresponding value) from the given
 *        dictionary.
 * @return Nonzero if the deletion was successful; zero otherwise.  We also
 *         return zero if the dictionary is empty.
 */
int
gnx_dict_delete(GnxDict *dict,
                const unsigned int *key)
{
    GnxArray *bucket;
    GnxNode *node;
    unsigned int i, j;

    gnx_i_check_dict(dict);
    g_return_val_if_fail(key, GNX_FAILURE);

    if (!dict->size)
        return GNX_FAILURE;
    if (!gnx_i_has(dict, key, &i, &j))
        return GNX_FAILURE;

    /* Delete the entry that has index j in bucket i. */
    bucket = (GnxArray *)(dict->bucket[i]);
    g_assert(bucket);
    g_assert(bucket->size);
    node = (GnxNode *)(bucket->cell[j]);
    if (GNX_FREE_KEYS & dict->free_key)
        free(node->key);
    if (GNX_FREE_VALUES & dict->free_value)
        free(node->value);
    node->key = NULL;
    node->value = NULL;
    free(node);
    bucket->cell[j] = NULL;

    /* Use the backend to shift all entries from index j upward down by one
     * position.
     */
    g_assert(!bucket->free_elem);
    assert(gnx_array_delete(bucket, &j));
    if (!bucket->size) {
        gnx_destroy_array(bucket);
        dict->bucket[i] = NULL;
    }
    (dict->size)--;

    return GNX_SUCCESS;
}

/**
 * @brief Whether a dictionary has a given key.
 *
 * @param dict We want to search this dictionary.
 * @param key Whether this key is in the dictionary.
 * @return If the given key is in the dictionary, we return a pointer to the
 *         value that is associated with the key.  Otherwise we return @c NULL.
 *         We also return @c NULL if the dictionary is empty.
 */
gnxptr
gnx_dict_has(const GnxDict *dict,
             const unsigned int *key)
{
    GnxNode *node;

    gnx_i_check_dict(dict);
    g_return_val_if_fail(key, NULL);
    if (!dict->size)
        return NULL;

    node = gnx_i_has(dict, key, NULL, NULL);
    if (!node)
        return NULL;

    return node->value;
}

/**
 * @brief Initializes an iterator over the elements of a dictionary.
 *
 * @param iter Initialize this iterator.  The iterator is usually allocated on
 *        the runtime stack.
 * @param dict We want to iterate over the elements of this dictionary.
 */
void
gnx_dict_iter_init(GnxDictIter *iter,
                   GnxDict *dict)
{
    iter->bootstrap = TRUE;
    iter->dict = dict;
    iter->i = 0;
    iter->j = 0;
}

/**
 * @brief Iterate to the next element of a dictionary.
 *
 * The order in which we iterate over the elements of a dictionary is not the
 * same as the order in which the elements were inserted into the dictionary.
 *
 * @param iter An iterator that has been initialized via the function
 *        gnx_dict_iter_init().
 * @param key This will hold the key of the next element in the dictionary.
 *        If @c NULL, then we will ignore the key of the next element.
 * @param value This will hold the value of the next element in the dictionary.
 *        If @c NULL, then we will ignore the value of the next element.
 * @return Nonzero if we have not yet reached the end of the dictionary; zero
 *         otherwise.  If zero, then we have reached the end of the dictionary
 *         and the iterator is now invalid.  We also return zero if the
 *         dictionary is empty.
 */
int
gnx_dict_iter_next(GnxDictIter *iter,
                   gnxptr *key,
                   gnxptr *value)
{
    GnxArray *bucket;
    GnxNode *node;
    unsigned int i;

    g_return_val_if_fail(iter, GNX_FAILURE);
    gnx_i_check_dict(iter->dict);

    /* If we are bootstrapping the process, then we look for the first element
     * to which we can iterate.
     */
    if (iter->bootstrap) {
        /* The dictionary is empty. */
        if (!(iter->dict->size))
            return GNX_FAILURE;

        /* The index of the first bucket that has at least one entry. */
        i = UINT_MAX;
        do {
            i++;
            bucket = (GnxArray *)(iter->dict->bucket[i]);
        } while (!bucket);
        iter->i = i;

        /* The first entry within the first non-empty bucket. */
        g_assert(bucket->size);
        iter->j = 0;
        node = (GnxNode *)(bucket->cell[iter->j]);
        if (key)
            *key = node->key;
        if (value)
            *value = node->value;

        iter->bootstrap = FALSE;
        return GNX_SUCCESS;
    }

    /* We have already iterated over the first element of the dictionary.
     * Start from the current element and iterate to the next element of the
     * dictionary.
     */
    bucket = (GnxArray *)(iter->dict->bucket[iter->i]);
    for ((iter->j)++; iter->j < bucket->size; (iter->j)++) {
        node = (GnxNode *)(bucket->cell[iter->j]);
        if (key)
            *key = node->key;
        if (value)
            *value = node->value;
        return GNX_SUCCESS;
    }

    /* We have exhausted the current bucket.  Find the next bucket that has at
     * least one entry.
     */
    (iter->i)++;
    while (iter->i < (iter->dict->capacity)) {
        bucket = (GnxArray *)(iter->dict->bucket[iter->i]);
        if (!bucket) {
            (iter->i)++;
            continue;
        }

        g_assert(bucket->size);
        iter->j = 0;
        node = (GnxNode *)(bucket->cell[iter->j]);
        if (key)
            *key = node->key;
        if (value)
            *value = node->value;
        return GNX_SUCCESS;
    }

    /* We have exhausted all buckets. */
    return GNX_FAILURE;
}

/**
 * @brief Initializes a new dictionary.
 *
 * The dictionary is initialized with default settings.  In particular, the
 * dictionary will not release the memory of its key/value pairs.  It is your
 * responsibility to ensure that the memory of the keys and values are released
 * as appropriate.
 *
 * @sa gnx_init_dict_full() This function gives you full control over how a
 *     dictionary will be initialized.
 *
 * @return See the return values of gnx_init_dict_full().
 */
GnxDict*
gnx_init_dict(void)
{
    return gnx_init_dict_full(GNX_DONT_FREE_KEYS, GNX_DONT_FREE_VALUES);
}

/**
 * @brief Initializes a new dictionary.
 *
 * This function gives you full control over how a dictionary will be
 * initialized.
 *
 * @sa gnx_init_dict() This function initializes a dictionary with default
 *     settings.
 *
 * @param free_key Whether to release the memory of each key in the dictionary.
 *        The possible values are #GNX_FREE_KEYS and #GNX_DONT_FREE_KEYS.  If
 *        #GNX_FREE_KEYS, then the memory of each key will be released when you
 *        destroy the dictionary via gnx_destroy_dict().  This option should
 *        only be used if each key to be inserted has memory that is allocated
 *        on the heap, i.e. via @c calloc(), @c malloc(), or @c realloc().
 *        Using this option with stack memory will result in undefined
 *        behavior.  If #GNX_DONT_FREE_KEYS, then you are responsible for
 *        releasing the memory of each key in the dictionary.  You can also use
 *        this option if each key has memory that is allocated on the stack.
 * @param free_value Whether to release the memory of each value in the
 *        dictionary.  The possibilities are #GNX_FREE_VALUES and
 *        #GNX_DONT_FREE_VALUES.  If #GNX_FREE_VALUES, then the memory of each
 *        value will be released when you destroy the dictionary via
 *        gnx_destroy_dict().  This option should only be used if each value to
 *        be inserted has memory that is allocated on the heap, i.e. via
 *        @c calloc(), @c malloc(), or @c realloc().  Using this option with
 *        stack memory will result in undefined behavior.  If
 *        #GNX_DONT_FREE_VALUES, then you are responsible for releasing the
 *        memory of each value in the dictionary.  You can also use this option
 *        if each value has memory that is allocated on the stack.
 * @return An initialized dictionary.  When you no longer need the dictionary,
 *         you must destroy the dictionary via the function gnx_destroy_dict().
 *         If we are unable to allocate memory, then we set @c errno to
 *         @c ENOMEM and return @c NULL.
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
    dict->size = 0;                           /* Dictionary initially empty. */

    /* The dictionary is represented as an array of buckets.  Each array
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
