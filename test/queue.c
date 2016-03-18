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

/* Test the functions in the module src/queue.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <gnx.h>

/**************************************************************************
 * prototypes for internal helper functions
 *************************************************************************/

/* new: create and destroy */
static void new_capacity_512(void);
static void new_default_capacity(void);
static void new_free(void);
static void new_minimum_capacity(void);
static void new_no_memory(void);

/**************************************************************************
 * new: create and destroy
 *************************************************************************/

static void
new(void)
{
    new_capacity_512();
    new_default_capacity();
    new_free();
    new_no_memory();
    new_minimum_capacity();
}

/* A queue with the capacity to hold 512 elements.
 */
static void
new_capacity_512(void)
{
    GnxQueue *queue;
    const unsigned int capacity = 512;

    queue = gnx_init_queue_full(&capacity, GNX_DONT_FREE_ELEMENTS);
    assert(queue);
    assert(GNX_DONT_FREE_ELEMENTS == queue->free_elem);
    assert(0 == queue->size);
    assert(0 == queue->i);
    assert(0 == queue->j);
    assert(capacity == queue->capacity);
    assert(queue->cell);

    gnx_destroy_queue(queue);
}

/* A queue with the default capacity.
 */
static void
new_default_capacity(void)
{
    GnxQueue *queue;

    queue = gnx_init_queue();
    assert(queue);
    assert(GNX_DONT_FREE_ELEMENTS == queue->free_elem);
    assert(0 == queue->size);
    assert(0 == queue->i);
    assert(0 == queue->j);
    assert(GNX_DEFAULT_ALLOC_SIZE == queue->capacity);
    assert(queue->cell);

    gnx_destroy_queue(queue);
}

/* Set the queue to release the memory of its elements.
 */
static void
new_free(void)
{
    GnxQueue *queue;
    const unsigned int capacity = 4;

    queue = gnx_init_queue_full(&capacity, GNX_FREE_ELEMENTS);
    assert(queue);
    assert(GNX_FREE_ELEMENTS == queue->free_elem);
    assert(0 == queue->size);
    assert(0 == queue->i);
    assert(0 == queue->j);
    assert(capacity == queue->capacity);
    assert(queue->cell);

    gnx_destroy_queue(queue);
}

/* A queue with the minimum capacity.
 */
static void
new_minimum_capacity(void)
{
    GnxQueue *queue;
    const unsigned int capacity = 2;

    queue = gnx_init_queue_full(&capacity, GNX_DONT_FREE_ELEMENTS);
    assert(queue);
    assert(GNX_DONT_FREE_ELEMENTS == queue->free_elem);
    assert(0 == queue->size);
    assert(0 == queue->i);
    assert(0 == queue->j);
    assert(capacity == queue->capacity);
    assert(queue->cell);

    gnx_destroy_queue(queue);
}

/* Test the function gnx_init_queue_full() under low-memory scenarios.
 */
static void
new_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    int alloc_size;
    const unsigned int capacity = 8;

    /* Cannot allocate memory for a queue. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_init_queue_full(&capacity, GNX_DONT_FREE_ELEMENTS));
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

    g_test_add_func("/queue/new", new);

    return g_test_run();
}
