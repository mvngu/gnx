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

#include <glib.h>

#include "array.h"
#include "sanity.h"

/**
 * @file array.h
 * @brief Array of pointers.
 *
 * The array stores any pointer that is inserted.  All pointers in the array
 * must reference the same data type.  Any pointer that is inserted into the
 * array must exist for at least as long as the array itself.
 *
 * Use the function gnx_init_array_full() to create a new array with a given
 * capacity.  You can use the convenience function gnx_init_array() to create
 * an array with a default capacity.  Destroy an array via the function
 * gnx_destroy_array().
 *
 * We do not make assumptions about the data type that will be in the array of
 * pointers.  You can use the function gnx_init_array_full() to explicitly set
 * the data type of each array element.  Otherwise, the function
 * gnx_init_array() will set each array element to the default of #GNX_POINTER.
 */

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

static void gnx_i_quicksort(GnxArray *array,
                            const unsigned int low,
                            const unsigned int high);
static unsigned int gnx_i_quicksort_partition(GnxArray *array,
                                              const unsigned int low,
                                              const unsigned int high);

/**************************************************************************
 * internal helper functions
 *************************************************************************/

/**
 * @brief Sort the elements of an array via the algorithm of quicksort.
 *
 * @param array Sort the elements of this array in non-decreasing order.
 *        Each array element must be a pointer to an <tt>unsigned int</tt>.
 * @param low Begin the sort at the element that has this index.
 * @param high End the sort at the element that has this index.  In effect, we
 *        want to sort the elements that start from index @c low up to and
 *        including the element at index @c high.
 */
static void
gnx_i_quicksort(GnxArray *array,
                const unsigned int low,
                const unsigned int high)
{
    unsigned int mid;

    g_assert(high < array->size);
    if (low >= high)
        return;

    mid = gnx_i_quicksort_partition(array, low, high);
    gnx_i_quicksort(array, low, mid);
    gnx_i_quicksort(array, mid + 1, high);
}

/**
 * @brief Partition the array that is to be sorted by quicksort.
 *
 * @param array Partition this array and sort the elements of a partition
 *        in non-decreasing order.  Each array element must be a pointer to
 *        an <tt>unsigned int</tt>.
 * @param low Begin the sort at the element that has this index.
 * @param high End the sort at the element that has this index.  In effect, we
 *        want to sort the elements that start from index @c low up to and
 *        including the element at index @c high.
 */
static unsigned int
gnx_i_quicksort_partition(GnxArray *array,
                          const unsigned int low,
                          const unsigned int high)
{
    unsigned int elem, i, k, *tmp;
    const unsigned int pivot = *((unsigned int *)(array->cell[low]));

    /* Use two indices that start at both ends of the partition.  Then move
     * the indices toward each other until we detect an inversion.
     */
    i = low - 1;
    k = high + 1;
    for (;;) {
        do {
            i++;
            elem = *((unsigned int *)(array->cell[i]));
        } while (elem < pivot);

        do {
            k--;
            elem = *((unsigned int *)(array->cell[k]));
        } while (elem > pivot);

        if (i >= k)
            return k;

        /* Swap A[i] with A[k]. */
        tmp = (unsigned int *)(array->cell[i]);
        array->cell[i] = array->cell[k];
        array->cell[k] = tmp;
    }
}

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Appends an element to an array of pointers.
 *
 * The array might possibly be resized to accommodate the element.  Even after
 * a resize, the capacity of the array must not exceed #GNX_MAXIMUM_ELEMENTS.
 * Furthermore, we assume that the current number of elements in the array is
 * less than #GNX_MAXIMUM_ELEMENTS.
 *
 * @param array We want to append an element to this array of pointers.
 * @param elem Append this element to the given array.  It is your
 *        responsibility to ensure that the element exists at least for the
 *        duration of the array itself.
 * @return Nonzero if the given element is successfully appended to the array;
 *         zero otherwise.  If we are unable to allocate memory, then we set
 *         @c errno to @c ENOMEM and return zero.
 */
int
gnx_array_append(GnxArray *array,
                 gnxptr elem)
{
    gnxptr *new_cell;
    unsigned int new_capacity;

    errno = 0;
    gnx_i_check_array(array);
    g_return_val_if_fail(elem, GNX_FAILURE);
    g_return_val_if_fail(array->size < GNX_MAXIMUM_ELEMENTS, GNX_FAILURE);

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
 * @brief Removes an element from an array of pointers.
 *
 * After a successful removal of the element at index @f$i@f$, all elements
 * from index @f$i + 1@f$ upward will be shifted down by one position.
 *
 * @sa gnx_array_delete_tail() Removes the tail element of an array.
 *
 * @param array We want to remove an element from this array of pointers.
 * @param i Remove from the array the element at this index.  The index must be
 *        less than the size of the array.  If you initialized the array with
 *        #GNX_DONT_FREE_ELEMENTS, then only the pointer is removed.  We do not
 *        remove the memory that the pointer references.  It is your
 *        responsibility to ensure that the memory that the pointer references
 *        is released when no longer needed.  If the array was initialized with
 *        #GNX_FREE_ELEMENTS, then we will release the memory that the element
 *        references.
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
    if (array->free_elem)
        free(array->cell[*i]);
    ncell = size - 1 - (*i);
    (void)memmove(&(array->cell[*i]), &(array->cell[*i + 1]),
                  sizeof(gnxptr) * ncell);
    (array->size)--;

    return GNX_SUCCESS;
}

/**
 * @brief Removes the last element of an array of pointers.
 *
 * @sa gnx_array_delete() Removes an array element given a position index.
 *
 * @param array We want to remove the last element of this array of pointers.
 *        If you initialized the array with #GNX_DONT_FREE_ELEMENTS, then only
 *        the pointer is removed.  We do not release the memory that the pointer
 *        references.  It is your responsibility to ensure that the memory that
 *        the pointer references is released when no longer needed.  If the
 *        array was initialized with #GNX_FREE_ELEMENTS, then we will release
 *        the memory that the element references.
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
    if (array->free_elem)
        free(array->cell[array->size]);
    array->cell[array->size] = NULL;

    return GNX_SUCCESS;
}

/**
 * @brief Sort the array elements in non-decreasing order.
 *
 * We can only sort elements if there is a specific method to compare elements.
 *
 * @param array Sort the elements of this array.  The elements of the array
 *        must have been configured to be of type #GNX_UINT.
 * @return Nonzero if the sort was successful; zero otherwise.  We also return
 *         zero if the array is empty.
 */
int
gnx_array_sort(GnxArray *array)
{
    unsigned int low, high;

    gnx_i_check_array(array);
    if (!(GNX_UINT & array->datatype))
        return GNX_FAILURE;
    if (!array->size)
        return GNX_FAILURE;

    low = 0;
    high = array->size - 1;
    gnx_i_quicksort(array, low, high);

    return GNX_SUCCESS;
}

/**
 * @brief Destroys an array of pointers.
 *
 * @param array We want to destroy this array of pointers.  The array must have
 *        been initialized via gnx_init_array() or gnx_init_array_full().
 */
void
gnx_destroy_array(GnxArray *array)
{
    unsigned int i;

    if (!array)
        return;
    if (array->cell) {
        for (i = 0; i < array->size; i++) {
            if (array->cell[i]) {
                if (array->free_elem)
                    free(array->cell[i]);

                array->cell[i] = NULL;
            }
        }
        free(array->cell);
        array->cell = NULL;
    }
    free(array);
    array = NULL;
}

/**
 * @brief Initializes an array of pointers with default settings.
 *
 * The array is initialized with a default capacity.  Furthermore, the array is
 * set to not release the memory of each of its elements.  It is your
 * responsibility to ensure that the memory of each element inserted into the
 * array is released.  By default, each element is assumed to be of the general
 * type #GNX_POINTER.
 *
 * @sa gnx_init_array_full() Initializes an array of pointers with full control
 *     over its settings.
 *
 * @return An initialized array of pointers with zero elements, a default
 *         capacity of #GNX_DEFAULT_ALLOC_SIZE, and each element has the
 *         general type #GNX_POINTER.  When you no longer need the array, you
 *         must destroy the array via the function gnx_destroy_array().
 *         See gnx_init_array_full() for further details on the return value.
 */
GnxArray*
gnx_init_array(void)
{
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    return gnx_init_array_full(&capacity, GNX_DONT_FREE_ELEMENTS, GNX_POINTER);
}

/**
 * @brief Initializes an array of pointers.
 *
 * @sa gnx_init_array() Initializes an array of pointers with default settings.
 *
 * @param capacity The initial capacity of the array.  This is assumed to be a
 *        positive power of two, but must not exceed #GNX_MAXIMUM_ELEMENTS.  A
 *        large initial capacity ensures that we do not need to constantly
 *        resize the array when we add a large number of elements.
 * @param destroy Whether to release the memory of each element as part of the
 *        destroy procedure.  Possible values are:
 *        <ul>
 *        <li>#GNX_FREE_ELEMENTS: The memory of elements will be released by
 *            the function gnx_destroy_array().  This option should only be
 *            used if each element to be inserted has memory that is allocated
 *            on the heap, i.e. memory was allocated via @c calloc(),
 *            @c malloc(), or @c realloc().  Using this option with stack
 *            memory will result in undefined behavior.</li>
 *        <li>#GNX_DONT_FREE_ELEMENTS: The memory of elements will not be
 *            released by the function gnx_destroy_array().  It is your
 *            responsibility to release the memory of each element in the array.
 *            You can also use this option if each element of the array has
 *            memory that is allocated on the stack.</li>
 *        </ul>
 * @param datatype The data type of each element in the array.  Each array
 *        element should be of the same data type.  Possible values are:
 *        <ul>
 *        <li>#GNX_UINT: Each array element is a pointer to an
 *            <tt>unsigned int</tt>.  Having this distinction is important
 *            because it helps us to determine whether two elements can be
 *            compared as integers.</li>
 *        <li>#GNX_POINTER: Each array element is an untyped pointer.  Use
 *            this option if you do not know the type of each element.</li>
 *        </ul>
 * @return An initialized array of pointers with zero elements and the given
 *         capacity.  When you no longer need the array, you must destroy the
 *         array via the function gnx_destroy_array().  If we are unable to
 *         allocate memory, then @c errno is set to @c ENOMEM and we return
 *         @c NULL.
 */
GnxArray*
gnx_init_array_full(const unsigned int *capacity,
                    const GnxBool destroy,
                    const GnxBool datatype)
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
    gnx_i_check_destroy_type(destroy);
    gnx_i_check_data_type(datatype);

    array = (GnxArray *)malloc(sizeof(GnxArray));
    if (!array)
        goto cleanup;

    array->datatype = datatype;
    array->free_elem = GNX_FREE_ELEMENTS & destroy;
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
