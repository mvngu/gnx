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

#include <errno.h>
#include <stdlib.h>
#include <string.h>  /* memmove */

#include "array.h"
#include "sanity.h"

/**
 * @file array.h
 * @brief Array of integers.
 *
 * The array does not make a copy of any integer that is inserted.  It only
 * stores a pointer to an integer.  Thus any integer that is added to the array
 * must exist for at least as long as the array itself.
 *
 * Use the function gnx_init_array_full() to create a new array with a given
 * capacity.  You can use the convenience function gnx_init_array() to create
 * an array with a default capacity.  Destroy an array via the function
 * gnx_destroy_array().
 */

/**
 * @brief Appends an element to an array.
 *
 * The array might possibly be resized to accommodate the element.  Even after
 * a resize, the capacity of the array must not exceed #GNX_MAXIMUM_ELEMENTS.
 *
 * @param array We want to append an element to this array.
 * @param elem Append this element to the given array.  We do not make a copy
 *        of this element, but only copy the pointer.  Thus it is your
 *        responsibility to ensure that the element exists at least for the
 *        duration of the array itself.
 * @return Nonzero if the given element is successfully appended to the array;
 *         zero otherwise.  If we are unable to allocate memory, then we set
 *         @c errno to @c ENOMEM and return zero.
 */
int
gnx_array_append(GnxArray *array,
                 unsigned int *elem)
{
    gnxptr *new_cell;
    unsigned int new_capacity;

    errno = 0;
    gnx_i_check_array(array);
    g_return_val_if_fail(elem, GNX_FAILURE);

    /* Possibly resize the array by doubling the current capacity. */
    if (array->size >= array->capacity) {
        new_capacity = array->capacity << 1;
        g_assert(new_capacity <= GNX_MAXIMUM_ELEMENTS);

        new_cell
            = (gnxptr *)realloc(array->cell, new_capacity * sizeof(gnxptr));
        if (!new_cell) {
            errno = ENOMEM;
            return GNX_FAILURE;
        }

        array->capacity = new_capacity;
        array->cell = new_cell;
    }

    /* Append the element to the array. */
    array->cell[array->size] = elem;
    (array->size)++;

    return GNX_SUCCESS;
}

/**
 * @brief Removes an element from an array.
 *
 * After a successful removal of the element at index @f$i@f$, all elements
 * from index @f$i + 1@f$ upward will be shifted down by one position.
 *
 * @sa gnx_array_delete_tail() Removes the tail element of an array.
 *
 * @param array We want to remove an element from this array.
 * @param i Remove from the array the element at this index.  The index must be
 *        less than the size of the array.  If you initialized the array with
 *        #GNX_DONT_FREE_ELEMENTS, then only the pointer is removed.  We do not
 *        remove the memory that the pointer references.  It is your
 *        responsibility to ensure that the memory that the pointer references
 *        is released when no longer needed.  If the array was initialized with
 *        #GNX_FREE_ELEMENTS, then we will release the memory that the element
 *        occupies.
 * @return Nonzero if we successfully removed the element at the given index;
 *         zero otherwise.  If the array is empty, then we return zero.
 */
int
gnx_array_delete(GnxArray *array,
                 const unsigned int *i)
{
    unsigned int ncell, size;

    gnx_i_check_array(array);
    g_return_val_if_fail(i, GNX_FAILURE);

    size = array->size;
    if (!size)
        return GNX_FAILURE;

    g_return_val_if_fail(*i < size, GNX_FAILURE);
    if ((1 == size) || (*i == (size - 1)))
        return gnx_array_delete_tail(array);

    /* The number of cells to shift downward can be computed as the number of
     * cells from index i + 1 to the index of the last cell (which is the size
     * of the array minus one).  Hence the formula:
     *
     * #cells to shift down = (index of last cell) - (index of target cell)
     */
    if (GNX_FREE_ELEMENTS & array->free_elem)
        free(array->cell[*i]);
    ncell = size - 1 - (*i);
    (void)memmove(&(array->cell[*i]), &(array->cell[*i + 1]),
                  sizeof(gnxptr) * ncell);
    (array->size)--;

    return GNX_SUCCESS;
}

/**
 * @brief Removes the last element of an array.
 *
 * @sa gnx_array_delete() Removes an array element given a position index.
 *
 * @param array We want to remove the last element of this array.  If you
 *        initialized the array with #GNX_DONT_FREE_ELEMENTS, then only the
 *        pointer is removed.  We do not release the memory that the pointer
 *        references.  It is your responsibility to ensure that the memory that
 *        the pointer references is released when no longer needed.  If the
 *        array was initialized with #GNX_FREE_ELEMENTS, then we will release
 *        the memory that the element occupies.
 * @return Nonzero if we successfully removed the last element of the array;
 *         zero otherwise.  If the array is empty, then we return zero.
 */
int
gnx_array_delete_tail(GnxArray *array)
{
    gnx_i_check_array(array);
    if (!array->size)
        return GNX_FAILURE;

    (array->size)--;
    if (GNX_FREE_ELEMENTS & array->free_elem)
        free(array->cell[array->size]);
    array->cell[array->size] = NULL;

    return GNX_SUCCESS;
}

/**
 * @brief Whether an array contains a given element.
 *
 * We perform a linear search in the array.  The time complexity is @f$O(n)@f$
 * for an array with @f$n@f$ elements.
 *
 * @param array Search this array for an element.
 * @param elem Whether the given array has this element.
 * @return Nonzero if the given element is in the array; zero otherwise.  If
 *         the array is empty, then we return zero.
 */
int
gnx_array_has(const GnxArray *array,
              const unsigned int *elem)
{
    unsigned int i;

    gnx_i_check_array(array);
    g_return_val_if_fail(elem, GNX_FAILURE);
    if (!array->size)
        return GNX_FAILURE;

    for (i = 0; i < array->size; i++) {
        if (*elem == *((unsigned int *)(array->cell[i])))
            return GNX_SUCCESS;
    }

    return GNX_FAILURE;
}

/**
 * @brief Destroys an array.
 *
 * @param array We want to destroy this array.  The array must have been
 *        initialized via gnx_init_array() or gnx_init_array_full().
 */
void
gnx_destroy_array(GnxArray *array)
{
    unsigned int i;

    if (!array)
        return;
    if (array->cell) {
        if (GNX_FREE_ELEMENTS & array->free_elem) {
            for (i = 0; i < array->size; i++) {
                if (array->cell[i]) {
                    free(array->cell[i]);
                    array->cell[i] = NULL;
                }
            }
        }
        free(array->cell);
        array->cell = NULL;
    }
    free(array);
    array = NULL;
}

/**
 * @brief Initializes an array with default settings.
 *
 * The array is initialized with a default capacity.  Furthermore, the array is
 * set to not release the memory of each of its elements.  Thus, it is your
 * responsibility to ensure that the memory of each element inserted into the
 * array is released.
 *
 * @sa gnx_init_array_full() Initializes an array with full control over its
 *     settings.
 *
 * @return An initialized array of pointers with zero elements and a default
 *         capacity of #GNX_DEFAULT_ALLOC_SIZE.  When you no longer need the
 *         array, you must destroy the array via the function
 *         gnx_destroy_array().  See gnx_init_array_full() for further details
 *         on the return value.
 */
GnxArray*
gnx_init_array(void)
{
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    return gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS);
}

/**
 * @brief Initializes an array.
 *
 * @sa gnx_init_array() Initializes an array with default settings.
 *
 * @param capacity The initial capacity of the array.  This is assumed to be a
 *        positive power of two, but must not exceed #GNX_MAXIMUM_ELEMENTS.  A
 *        large initial capacity ensures that we do not need to constantly
 *        resize the array when we add a large number of elements.
 * @param destroy Whether to release the memory of each element as part of the
 *        destroy procedure.  Possible values are #GNX_FREE_ELEMENTS or
 *        #GNX_DONT_FREE_ELEMENTS.  If #GNX_FREE_ELEMENTS, then calling the
 *        function gnx_destroy_array() will release the memory of each element
 *        of the array, in addition to destroying the array itself.  This
 *        option should only be used if each element to be inserted has memory
 *        that is allocated on the heap, i.e. via @c calloc(), @c malloc(), or
 *        @c realloc().  Using this option with stack memory will result in
 *        undefined behavior. If #GNX_DONT_FREE_ELEMENTS, then it is your
 *        responsibility to release the memory of each element in the array.
 *        You can also use this option if each element of the array has memory
 *        that is allocated on the stack.
 * @return An initialized array with zero elements and the given capacity.
 *         When you no longer need the array, you must destroy the array via
 *         the function gnx_destroy_array(). If we are unable to allocate
 *         memory, then @c errno is set to @c ENOMEM and we return @c NULL.
 */
GnxArray*
gnx_init_array_full(const unsigned int *capacity,
                    const GnxBool destroy)
{
    GnxArray *array;

    errno = 0;
    g_return_val_if_fail(capacity, NULL);
    g_return_val_if_fail((*capacity > 1)
                         && (*capacity <= GNX_MAXIMUM_ELEMENTS),
                         NULL);
    /* If n > 1 is an unsigned integer, then n is a power of two provided that
     *
     * n & (n - 1) == 0
     *
     * If n > 1 is a power of two, then its binary representation has exactly
     * one bit set to 1 at some position k and all other bits are set to 0.
     * Then the integer n - 1 has all bits from position 0 to k - 1 set to 1.
     * Thus the bit-wise AND of n and n - 1 must be zero.
     */
    g_return_val_if_fail(!((*capacity) & (*capacity - 1)), NULL);
    g_return_val_if_fail((GNX_FREE_ELEMENTS & destroy)
                         || (GNX_DONT_FREE_ELEMENTS & destroy),
                         NULL);

    array = (GnxArray *)malloc(sizeof(GnxArray));
    if (!array)
        goto cleanup;

    array->free_elem = destroy;
    array->capacity = *capacity;
    array->size = 0;
    array->cell = (gnxptr *)calloc(array->capacity, sizeof(gnxptr));
    if (!array->cell)
        goto cleanup;

    return array;

cleanup:
    errno = ENOMEM;
    gnx_destroy_array(array);
    return NULL;
}
