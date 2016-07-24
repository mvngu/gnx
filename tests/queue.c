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

/* append */
static void append_empty(void);
static void append_maximum(void);
static void append_no_memory(void);
static void append_random(void);
static void append_resize(void);

/* new: create and destroy */
static void new_capacity_512(void);
static void new_default_capacity(void);
static void new_free(void);
static void new_minimum_capacity(void);
static void new_no_memory(void);

/* peek */
static void peek_empty(void);
static void peek_one(void);
static void peek_random(void);

/* pop */
static void pop_empty(void);
static void pop_maximum(void);
static void pop_one(void);
static void pop_random(void);
static void pop_resize_wrap(void);
static void pop_wrap(void);

/**************************************************************************
 * append
 *************************************************************************/

static void
append(void)
{
    append_empty();
    append_maximum();
    append_no_memory();
    append_random();
    append_resize();
}

/* Append an element to an empty queue.
 */
static void
append_empty(void)
{
    GnxQueue *queue;
    unsigned int elem = (unsigned int)g_random_int();

    queue = gnx_init_queue();
    assert(gnx_queue_append(queue, &elem));
    assert(1 == queue->size);
    assert(0 == queue->i);
    assert(0 == queue->j);
    assert(elem == *((unsigned int *)(queue->cell[0])));

    gnx_destroy_queue(queue);
}

/* Fill a queue to capacity without triggering a resize.
 */
static void
append_maximum(void)
{
    GnxQueue *queue;
    unsigned int i, *list;
    const unsigned int size = GNX_DEFAULT_ALLOC_SIZE;

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    queue = gnx_init_queue();

    for (i = 0; i < size; i++) {
        list[i] = (unsigned int)g_random_int();
        assert(gnx_queue_append(queue, &(list[i])));
    }
    assert(size == queue->size);
    assert(0 == queue->i);
    assert((size - 1) == queue->j);

    for (i = 0; i < size; i++)
        assert(list[i] == *((unsigned int *)(queue->cell[i])));

    free(list);
    gnx_destroy_queue(queue);
}

/* Test the function gnx_queue_append() in low-memory scenarios.
 */
static void
append_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxQueue *queue;
    int alloc_size;
    unsigned int *elem, i;
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    const unsigned int size = GNX_DEFAULT_ALLOC_SIZE;

    /* Populate a queue to capacity. */
    queue = gnx_init_queue_full(&capacity, GNX_FREE_ELEMENTS);
    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_queue_append(queue, elem));
    }
    assert(size == queue->size);
    assert(queue->size == queue->capacity);

    /* Cannot allocate memory to resize the queue. */
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = (unsigned int)g_random_int();
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_queue_append(queue, elem));
    assert(ENOMEM == errno);
    assert(size == queue->size);
    assert(queue->size == queue->capacity);

    free(elem);
    gnx_destroy_queue(queue);
    gnx_alloc_reset_limit();
#endif
}

/* Fill a queue with a random number of elements.
 */
static void
append_random(void)
{
    GnxQueue *queue;
    unsigned int *elem, i;
    const int high = (int)GNX_DEFAULT_ALLOC_SIZE;
    const int low = 2;
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    const unsigned int size = (unsigned int)g_random_int_range(low, high);

    assert(size < GNX_DEFAULT_ALLOC_SIZE);
    queue = gnx_init_queue_full(&capacity, GNX_FREE_ELEMENTS);

    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_queue_append(queue, elem));
    }
    assert(size == queue->size);
    assert(0 == queue->i);
    assert((size - 1) == queue->j);

    gnx_destroy_queue(queue);
}

/* Trigger a resize of a queue.
 */
static void
append_resize(void)
{
    GnxQueue *queue;
    unsigned int i, *list;
    const unsigned int size = GNX_DEFAULT_ALLOC_SIZE + 1;

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    queue = gnx_init_queue();

    for (i = 0; i < size; i++) {
        list[i] = (unsigned int)g_random_int();
        assert(gnx_queue_append(queue, &(list[i])));
    }
    assert(size == queue->size);
    assert((GNX_DEFAULT_ALLOC_SIZE << 1) == queue->capacity);
    assert(0 == queue->i);
    assert((size - 1) == queue->j);

    for (i = 0; i < size; i++)
        assert(list[i] == *((unsigned int *)(queue->cell[i])));

    free(list);
    gnx_destroy_queue(queue);
}

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
 * peek
 *************************************************************************/

static void
peek(void)
{
    peek_empty();
    peek_one();
    peek_random();
}

/* Peek at an empty queue.
 */
static void
peek_empty(void)
{
    GnxQueue *queue;
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;

    queue = gnx_init_queue();
    assert(0 == queue->size);
    assert(capacity == queue->capacity);
    assert(!gnx_queue_peek(queue));
    assert(0 == queue->size);
    assert(capacity == queue->capacity);

    gnx_destroy_queue(queue);
}

/* Peek at a queue that has one element.
 */
static void
peek_one(void)
{
    GnxQueue *queue;
    unsigned int elem = (unsigned int)g_random_int();
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;

    queue = gnx_init_queue();
    assert(gnx_queue_append(queue, &elem));
    assert(1 == queue->size);
    assert(capacity == queue->capacity);
    assert(elem == *gnx_queue_peek(queue));
    assert(1 == queue->size);
    assert(capacity == queue->capacity);

    gnx_destroy_queue(queue);
}

/* Peek at a queue that has a random number of elements.
 */
static void
peek_random(void)
{
    GnxQueue *queue;
    unsigned int *elem, *first, i;
    const unsigned int capacity = 32;
    const unsigned int size = (unsigned int)g_random_int_range(2, 33);

    assert(size <= capacity);
    queue = gnx_init_queue_full(&capacity, GNX_FREE_ELEMENTS);

    first = (unsigned int *)malloc(sizeof(unsigned int));
    *first = (unsigned int)g_random_int();
    assert(gnx_queue_append(queue, first));

    for (i = 1; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_queue_append(queue, elem));
    }
    assert(size == queue->size);
    assert(capacity == queue->capacity);

    assert(*first == *gnx_queue_peek(queue));
    assert(size == queue->size);
    assert(capacity == queue->capacity);

    gnx_destroy_queue(queue);
}

/**************************************************************************
 * pop
 *************************************************************************/

static void
pop(void)
{
    pop_empty();
    pop_maximum();
    pop_one();
    pop_random();
    pop_resize_wrap();
    pop_wrap();
}

/* Pop from an empty queue.
 */
static void
pop_empty(void)
{
    GnxQueue *queue;
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;

    queue = gnx_init_queue();
    assert(0 == queue->size);
    assert(capacity == queue->capacity);
    assert(!gnx_queue_pop(queue));
    assert(0 == queue->size);
    assert(capacity == queue->capacity);

    gnx_destroy_queue(queue);
}

/* Pop from a queue that has been populated to capacity.
 */
static void
pop_maximum(void)
{
    GnxQueue *queue;
    unsigned int *elem, i, *list;
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    const unsigned int size = capacity;

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    queue = gnx_init_queue();

    /* Populate the queue to capacity. */
    for (i = 0; i < size; i++) {
        list[i] = (unsigned int)g_random_int();
        assert(gnx_queue_append(queue, &(list[i])));
    }
    assert(size == queue->size);
    assert(capacity == queue->capacity);
    assert(0 == queue->i);
    assert((size - 1) == queue->j);

    /* Pop until empty. */
    i = 0;
    while (queue->size) {
        elem = gnx_queue_pop(queue);
        assert(elem);
        assert(*elem == list[i]);
        i++;
    }
    assert(0 == queue->size);
    assert(capacity == queue->capacity);

    free(list);
    gnx_destroy_queue(queue);
}

/* Pop from a queue that has one element.
 */
static void
pop_one(void)
{
    GnxQueue *queue;
    unsigned int *elem;
    unsigned int a = (unsigned int)g_random_int();

    queue = gnx_init_queue();
    assert(gnx_queue_append(queue, &a));
    assert(1 == queue->size);

    elem = gnx_queue_pop(queue);
    assert(a == *elem);
    assert(0 == queue->size);
    assert(0 == queue->i);
    assert(0 == queue->j);

    gnx_destroy_queue(queue);
}

/* Pop from a queue that has a random number of elements.
 */
static void
pop_random(void)
{
    GnxQueue *queue;
    unsigned int i, *list;
    const int high = GNX_DEFAULT_ALLOC_SIZE + 1;
    const int low = 2;
    const unsigned int size = (unsigned int)g_random_int_range(low, high);

    assert(size <= GNX_DEFAULT_ALLOC_SIZE);
    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    queue = gnx_init_queue();

    for (i = 0; i < size; i++) {
        list[i] = (unsigned int)g_random_int();
        assert(gnx_queue_append(queue, &(list[i])));
    }
    assert(size == queue->size);

    /* Pop down to only one element. */
    while (queue->size > 1)
        assert(gnx_queue_pop(queue));
    assert(1 == queue->size);
    assert(0 == queue->i);
    assert(0 == queue->j);

    free(list);
    gnx_destroy_queue(queue);
}

/* Get a queue to wrap around and then trigger a resize.
 */
static void
pop_resize_wrap(void)
{
    GnxQueue *queue;
    unsigned int elem, i, *list;
    const unsigned int pop_size = (unsigned int)g_random_int_range(5, 21);
    const unsigned int size = GNX_DEFAULT_ALLOC_SIZE;

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    queue = gnx_init_queue();

    /* Populate to capacity. */
    for (i = 0; i < size; i++) {
        list[i] = (unsigned int)g_random_int();
        assert(gnx_queue_append(queue, &(list[i])));
    }
    assert(size == queue->size);

    /* Pop a random number of elements. */
    for (i = 0; i < pop_size; i++)
        assert(gnx_queue_pop(queue));

    /* Insert the same number of random elements. */
    for (i = 0; i < pop_size; i++)
        assert(gnx_queue_append(queue, &(list[i])));

    /* Trigger a resize.  After the resize, the head of the queue now has
     * index 0.
     */
    assert(size == queue->size);
    assert(queue->i);
    elem = (unsigned int)g_random_int();
    assert(GNX_DEFAULT_ALLOC_SIZE == queue->capacity);
    assert(gnx_queue_append(queue, &elem));
    assert(0 == queue->i);
    assert(size == queue->j);
    assert((size + 1) == queue->size);
    assert((GNX_DEFAULT_ALLOC_SIZE << 1) == queue->capacity);

    free(list);
    gnx_destroy_queue(queue);
}

/* Get a queue to wrap around.
 */
static void
pop_wrap(void)
{
    GnxQueue *queue;
    unsigned int *elem, i, *list;
    const unsigned int pop_size = (unsigned int)g_random_int_range(1, 21);
    const unsigned int size = GNX_DEFAULT_ALLOC_SIZE;

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    queue = gnx_init_queue();

    /* Populate to capacity. */
    for (i = 0; i < size; i++) {
        list[i] = (unsigned int)g_random_int();
        assert(gnx_queue_append(queue, &(list[i])));
    }
    assert(size == queue->size);

    /* Pop a random number of elements. */
    for (i = 0; i < pop_size; i++)
        assert(gnx_queue_pop(queue));
    assert((size - pop_size) == queue->size);
    assert(pop_size == queue->i);
    assert((size - 1) == queue->j);

    /* Insert the same number of random elements. */
    for (i = 0; i < pop_size; i++)
        assert(gnx_queue_append(queue, &(list[i])));
    assert(size == queue->size);
    assert(pop_size == queue->i);
    assert((queue->i - 1) == queue->j);

    /* Compare elements. */
    i = pop_size;
    while (queue->size) {
        elem = gnx_queue_pop(queue);
        assert(*elem == list[i]);
        i++;
        if (i >= size)
            i = 0;
    }

    free(list);
    gnx_destroy_queue(queue);
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/queue/append", append);
    g_test_add_func("/queue/new", new);
    g_test_add_func("/queue/peek", peek);
    g_test_add_func("/queue/pop", pop);

    return g_test_run();
}
