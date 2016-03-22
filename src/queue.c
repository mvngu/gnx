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
#include <string.h>  /* memcpy */

#include "queue.h"
#include "sanity.h"

/**
 * @file queue.h
 * @brief Queue of integers.
 *
 * A queue does not make its own copy of any element that is appended to it.
 * Rather, the queue copies the pointer that is passed in and stores that
 * pointer.  It is your responsibility to ensure that any element that is
 * appended to the queue exists for the duration of the queue itself.  For
 * further details on the queue data structure, see the
 * <a href="https://en.wikipedia.org/wiki/Queue_(abstract_data_type)">Wikipedia article</a>.
 *
 * Use the function gnx_init_queue() or gnx_init_queue_full() to initialize a
 * new queue of integers.  Destroy a queue via the function gnx_destroy_queue().
 */

/**
 * @brief Destroys a queue of integers.
 *
 * @param queue We want to destroy this queue.  The queue must have been
 *        initialized via gnx_init_queue() or gnx_init_queue_full().
 */
void
gnx_destroy_queue(GnxQueue *queue)
{
    unsigned int i;

    if (!queue)
        return;
    if (queue->cell) {
        if (GNX_FREE_ELEMENTS & queue->free_elem) {
            for (i = 0; i < queue->size; i++) {
                if (queue->cell[i]) {
                    free(queue->cell[i]);
                    queue->cell[i] = NULL;
                }
            }
        }
        free(queue->cell);
        queue->cell = NULL;
    }
    free(queue);
    queue = NULL;
}

/**
 * @brief Initializes a queue of integers.
 *
 * The queue is initialized with default settings.  In particular, the queue is
 * set to not release the memory of its elements when you destroy the queue via
 * the function gnx_destroy_queue().  It is your responsibility to ensure that
 * any element appended to the queue exists for the duration of the queue and
 * that you must release the memory of the elements as necessary.
 *
 * @return An initialized queue.  Note that you are responsible for releasing
 *         the memory of elements in the queue.  See the return value of
 *         gnx_init_queue_full().
 */
GnxQueue*
gnx_init_queue(void)
{
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    return gnx_init_queue_full(&capacity, GNX_DONT_FREE_ELEMENTS);
}

/**
 * @brief Initializes a queue of integers.
 *
 * @sa gnx_init_queue() Initializes a queue with default settings.
 *
 * @param capacity The initial capacity of the queue.  This is assumed to be a
 *        positive power of two, but must not exceed #GNX_MAXIMUM_ELEMENTS.  A
 *        large initial capacity ensures that we do not need to constantly
 *        resize the queue when we append a large number of elements.
 * @param destroy Whether to release the memory of each element as part of the
 *        destroy procedure.  Possible values are #GNX_FREE_ELEMENTS or
 *        #GNX_DONT_FREE_ELEMENTS.  If #GNX_FREE_ELEMENTS, then calling the
 *        function gnx_destroy_queue() will release the memory of each element
 *        of the queue, in addition to destroying the queue itself.  This
 *        option should only be used if each element to be appended has memory
 *        that is allocated on the heap, i.e. via @c calloc(), @c malloc(), or
 *        @c realloc().  Using this option with stack memory will result in
 *        undefined behavior. If #GNX_DONT_FREE_ELEMENTS, then it is your
 *        responsibility to release the memory of each element in the queue.
 *        You can also use this option if each element has memory that is
 *        allocated on the stack.
 * @return An initialized queue of integers with zero elements and the given
 *         capacity.  When you no longer need the queue, you must destroy the
 *         queue via the function gnx_destroy_queue(). If we are unable to
 *         allocate memory, then @c errno is set to @c ENOMEM and we return
 *         @c NULL.
 */
GnxQueue*
gnx_init_queue_full(const unsigned int *capacity,
                    const GnxBool destroy)
{
    GnxQueue *queue;

    errno = 0;
    g_return_if_fail(capacity);
    g_return_if_fail((*capacity > 1) && (*capacity <= GNX_MAXIMUM_ELEMENTS));
    /* If n > 1 is an unsigned integer, then n is a power of two provided that
     *
     * n & (n - 1) == 0
     *
     * If n > 1 is a power of two, then its binary representation has exactly
     * one bit set to 1 at some position k and all other bits are set to 0.
     * Then the integer n - 1 has all bits from position 0 to k - 1 set to 1.
     * Thus the bit-wise AND of n and n - 1 must be zero.
     */
    g_return_if_fail(!((*capacity) & (*capacity - 1)));
    g_return_if_fail((GNX_FREE_ELEMENTS & destroy)
                     || (GNX_DONT_FREE_ELEMENTS & destroy));

    queue = (GnxQueue *)malloc(sizeof(GnxQueue));
    if (!queue)
        goto cleanup;

    queue->free_elem = destroy;
    queue->i = 0;
    queue->j = 0;
    queue->size = 0;
    queue->capacity = *capacity;
    queue->cell
        = (gnxintptr *)malloc(sizeof(gnxintptr) * queue->capacity);
    if (!queue->cell)
        goto cleanup;

    return queue;

cleanup:
    errno = ENOMEM;
    gnx_destroy_queue(queue);
    return NULL;
}

/**
 * @brief Appends an element to a queue.
 *
 * We assume that the current number of elements in the queue is less than
 * #GNX_MAXIMUM_ELEMENTS.
 *
 * @param queue We want to append an element to this queue.
 * @param elem Append this element to the given queue.  It is your
 *        responsibility to ensure that this element exists for the duration of
 *        the queue.
 * @return Nonzero if we successfully appended the element to the queue; zero
 *         otherwise.  If we are unable to allocate memory, then we set
 *         @c errno to @c ENOMEM and return zero.
 */
int
gnx_queue_append(GnxQueue *queue,
                 int *elem)
{
    gnxintptr *new_cell;
    unsigned int ncell, new_capacity;

    gnx_i_check_queue(queue);
    g_return_if_fail(elem);
    g_return_if_fail(queue->size < GNX_MAXIMUM_ELEMENTS);

    /* The queue is empty.  The given element is now the head of the queue. */
    if (!queue->size) {
        queue->cell[0] = elem;
        queue->i = 0;  /* Index of the head. */
        queue->j = 0;  /* Index of the tail. */
        queue->size = 1;
        return GNX_SUCCESS;
    }

    /* Possibly resize the queue by doubling the current capacity. */
    if (queue->size >= queue->capacity) {
        new_capacity = queue->capacity << 1;
        g_assert(new_capacity <= GNX_MAXIMUM_ELEMENTS);
        new_cell = (gnxintptr *)malloc(sizeof(gnxintptr) * new_capacity);
        if (!new_cell) {
            errno = ENOMEM;
            return GNX_FAILURE;
        }

        /* Copy elements over to the new array.  Let i be the index of the head
         * of the queue and let j be the index of the tail of the queue.  Let s
         * be the size of the queue and let c be the capacity of the queue.
         */
        if (queue->i <= queue->j) {
            /* If i <= j, then the queue has not wrapped around the array.
             * Thus we only need to copy the block of memory between the
             * indices i and j, inclusive.  Then i <= j and s >= c if and only
             * if i = 0 and j = s - 1.  We do not need to reset i and j after
             * the copy operation.
             */
            g_assert(!queue->i);
            g_assert(queue->j == (queue->size - 1));
            (void)memcpy(&(new_cell[0]), &(queue->cell[0]),
                         sizeof(gnxintptr) * (queue->size));
        } else {
            /* If i > j, then the queue has wrapped around the array and so we
             * need to copy two blocks of memory.  The first block of memory is
             * from i to the index of the last element in the array.  The
             * second block of memory is from 0 to j.  With s and c as above,
             * we have i > j and s >= c if and only if j = i - 1.  After the
             * copy operation, we must set i = 0 and j = s - 1.
             */
            g_assert(queue->j == (queue->i - 1));
            ncell = queue->size - queue->i;
            (void)memcpy(&(new_cell[0]), &(queue->cell[queue->i]),
                         sizeof(gnxintptr) * ncell);
            (void)memcpy(&(new_cell[ncell]), &(queue->cell[0]),
                         sizeof(gnxintptr) * (queue->j + 1));
        }

        free(queue->cell);
        queue->cell = new_cell;
        queue->capacity = new_capacity;
        queue->i = 0;
        queue->j = queue->size - 1;
    }

    /* Now append the element to the queue. */
    if (queue->i <= queue->j) {
        /* If i <= j, then we need to consider two cases. */
        if (queue->j < (queue->capacity - 1)) {
            /* The first case is that j < c - 1.  In this case, we have not yet
             * reached the end of the array.  The new element is simply
             * inserted at position j + 1.
             */
            (queue->j)++;
        } else {
            /* In the second case, we have j = c - 1 and thus we have reached
             * the end of the array.  Here, we need to wrap around and insert
             * the new element at position 0.  Of course we assume that 0 < i.
             */
            g_assert(queue->i > 0);
            queue->j = 0;
        }
    } else {
        /* If i > j, then the array has already wrapped around.  In this case,
         * we have j < i - 1.  The new element is inserted at position j + 1.
         */
        g_assert(queue->j < (queue->i - 1));
        (queue->j)++;
    }
    queue->cell[queue->j] = elem;
    (queue->size)++;

    return GNX_SUCCESS;
}

/**
 * @brief Peeks at the head of the queue.
 *
 * Peeking at an element should not modify the element or the queue in any way.
 *
 * @param queue We want to peek at the element at the head of this queue.
 * @return The element at the head of the queue.  If the queue is empty, then
 *         we return @c NULL.
 */
int*
gnx_queue_peek(const GnxQueue *queue)
{
    gnx_i_check_queue(queue);
    if (!queue->size)
        return NULL;

    return queue->cell[queue->i];
}

/**
 * @brief Removes the head of the queue.
 *
 * After an element is removed from the head of the queue, it is your
 * responsibility to ensure that you release the memory of that element (as
 * appropriate).
 *
 * @param queue We want to remove the head of this queue.
 * @return The element that was previously at the head of the queue.  If the
 *         queue is empty, then we return @c NULL.
 */
int*
gnx_queue_pop(GnxQueue *queue)
{
    int *elem, *head;
    unsigned int idx;

    gnx_i_check_queue(queue);
    if (!queue->size)
        return NULL;

    elem = queue->cell[queue->i];
    queue->cell[queue->i] = NULL;

    if (1 == queue->size) {
        queue->i = 0;
        queue->j = 0;
        queue->size = 0;
        return elem;
    }

    g_assert(queue->size > 1);
    /* Let i and j be the head and tail indices, respectively, of the queue.
     * Let s and c be the size and capacity, respectively, of the queue.
     */
    if (queue->i < queue->j) {
        /* If i < j, then we have not wrapped around the array.  In that case,
         * we set the head to NULL and increment i by one.
         */
        (queue->i)++;
    } else {
        /* The index of the last element in the array. */
        idx = queue->capacity - 1;

        /* Note that i =/= j since the queue has at least two elements.  If
         * i > j, then we have wrapped around the array.  Here, we have two
         * cases to consider.
         */
        if (queue->i < idx) {
            /* If i < c - 1, then we simply set the head to NULL and increment
             * i by one.
             */
            (queue->i)++;
        } else {
            /* If i = c - 1, then the head must wrap around to the beginning of
             * the array.  That is, we set the head to NULL and set i = 0.
             */
            queue->i = 0;
        }
    }
    (queue->size)--;

    /* If the size of the queue is now 1, then we move the only remaining
     * element to the start of the array.
     */
    if (1 == queue->size) {
        g_assert(queue->i == queue->j);
        head = queue->cell[queue->i];
        queue->cell[queue->i] = NULL;
        queue->cell[0] = head;
        queue->i = 0;
        queue->j = 0;
    }

    return elem;
}
