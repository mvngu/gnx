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

/* Test the functions in the module src/stack.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <glib.h>
#include <gnx.h>

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

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
static void pop_one(void);
static void pop_random(void);

/* push */
static void push_empty(void);
static void push_no_memory(void);
static void push_random(void);

/**************************************************************************
 * new: create and destroy
 *************************************************************************/

static void
new(void)
{
    new_capacity_512();
    new_default_capacity();
    new_free();
    new_minimum_capacity();
    new_no_memory();
}

/* A stack with the capacity to hold 512 elements.
 */
static void
new_capacity_512(void)
{
    GnxStack *stack;
    const unsigned int capacity = 512;

    stack = gnx_init_stack_full(&capacity, GNX_DONT_FREE_ELEMENTS);
    assert(stack);
    assert(stack->array);
    assert(GNX_DONT_FREE_ELEMENTS == stack->array->free_elem);
    assert(0 == stack->size);
    assert(capacity == stack->array->capacity);

    gnx_destroy_stack(stack);
}

/* A stack with the default capacity.
 */
static void
new_default_capacity(void)
{
    GnxStack *stack;

    stack = gnx_init_stack();
    assert(stack);
    assert(stack->array);
    assert(GNX_DONT_FREE_ELEMENTS == stack->array->free_elem);
    assert(0 == stack->size);
    assert(GNX_DEFAULT_ALLOC_SIZE == stack->array->capacity);

    gnx_destroy_stack(stack);
}

/* Set the stack to release the memory of its elements.
 */
static void
new_free(void)
{
    GnxStack *stack;
    const unsigned int capacity = 4;

    stack = gnx_init_stack_full(&capacity, GNX_FREE_ELEMENTS);
    assert(stack);
    assert(stack->array);
    assert(GNX_FREE_ELEMENTS == stack->array->free_elem);
    assert(0 == stack->size);
    assert(capacity == stack->array->capacity);

    gnx_destroy_stack(stack);
}

/* A stack with the minimum capacity.
 */
static void
new_minimum_capacity(void)
{
    GnxStack *stack;
    const unsigned int capacity = 2;

    stack = gnx_init_stack_full(&capacity, GNX_DONT_FREE_ELEMENTS);
    assert(stack);
    assert(stack->array);
    assert(GNX_DONT_FREE_ELEMENTS == stack->array->free_elem);
    assert(0 == stack->size);
    assert(capacity == stack->array->capacity);

    gnx_destroy_stack(stack);
}

/* Test the function gnx_init_stack_full() under low-memory scenarios.
 */
static void
new_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    int alloc_size;

    /* Cannot allocate memory for the stack. */
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(!gnx_init_stack());
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

/* Peek at the top of an empty stack.
 */
static void
peek_empty(void)
{
    GnxStack *stack;

    stack = gnx_init_stack();
    assert(0 == stack->size);
    assert(!gnx_stack_peek(stack));
    assert(0 == stack->size);

    gnx_destroy_stack(stack);
}

/* Peek at a stack that has one element.
 */
static void
peek_one(void)
{
    GnxStack *stack;
    unsigned int a = (unsigned int)g_random_int();
    unsigned int *elem;

    stack = gnx_init_stack();
    assert(gnx_stack_push(stack, &a));
    assert(1 == stack->size);

    elem = gnx_stack_peek(stack);
    assert(a == *elem);
    assert(1 == stack->size);

    gnx_destroy_stack(stack);
}

/* Peek at a stack that has a random number of elements.
 */
static void
peek_random(void)
{
    GnxStack *stack;
    unsigned int *elem, i, *list;
    const unsigned int size = (unsigned int)g_random_int_range(2, 21);

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    stack = gnx_init_stack();

    for (i = 0; i < size; i++) {
        list[i] = (unsigned int)g_random_int();
        assert(gnx_stack_push(stack, &(list[i])));
    }
    assert(size == stack->size);

    elem = gnx_stack_peek(stack);
    assert(elem);
    assert(*elem == list[size - 1]);
    assert(size == stack->size);

    free(list);
    gnx_destroy_stack(stack);
}

/**************************************************************************
 * pop
 *************************************************************************/

static void
pop(void)
{
    pop_empty();
    pop_one();
    pop_random();
}

/* Pop an element off an empty stack.
 */
static void
pop_empty(void)
{
    GnxStack *stack;

    stack = gnx_init_stack();
    assert(0 == stack->size);
    assert(!gnx_stack_pop(stack));
    assert(0 == stack->size);

    gnx_destroy_stack(stack);
}

/* Pop an element off a stack that has exactly one element.
 */
static void
pop_one(void)
{
    GnxStack *stack;
    unsigned int a, *b;

    a = (unsigned int)g_random_int();
    stack = gnx_init_stack();
    assert(gnx_stack_push(stack, &a));
    assert(1 == stack->size);

    b = gnx_stack_pop(stack);
    assert(a == *b);
    assert(0 == stack->size);

    gnx_destroy_stack(stack);
}

/* Pop an element off a stack that has a random number of elements.
 */
static void
pop_random(void)
{
    GnxStack *stack;
    unsigned int *elem, i, *list;
    const unsigned int capacity = 32;
    const unsigned int size = (unsigned int)g_random_int_range(32, 51);

    list = (unsigned int *)malloc(sizeof(unsigned int) * size);
    stack = gnx_init_stack_full(&capacity, GNX_DONT_FREE_ELEMENTS);

    for (i = 0; i < size; i++) {
        list[i] = (unsigned int)g_random_int();
        assert(gnx_stack_push(stack, &(list[i])));
    }
    assert(size == stack->size);

    i = size - 1;
    while (stack->size) {
        elem = gnx_stack_pop(stack);
        assert(*elem == list[i]);
        i--;
    }
    assert(0 == stack->size);

    free(list);
    gnx_destroy_stack(stack);
}

/**************************************************************************
 * push
 *************************************************************************/

static void
push(void)
{
    push_empty();
    push_no_memory();
    push_random();
}

/* Push an element onto an empty stack.
 */
static void
push_empty(void)
{
    GnxStack *stack;
    unsigned int a = (unsigned int)g_random_int();

    stack = gnx_init_stack();
    assert(0 == stack->size);
    assert(gnx_stack_push(stack, &a));
    assert(1 == stack->size);

    gnx_destroy_stack(stack);
}

/* Test the function gnx_stack_push() under low-memory scenarios.
 */
static void
push_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    GnxStack *stack;
    int alloc_size;
    unsigned int *elem, i;
    const unsigned int capacity = 16;
    const unsigned int size = capacity;

    stack = gnx_init_stack_full(&capacity, GNX_FREE_ELEMENTS);

    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_stack_push(stack, elem));
    }
    assert(size == stack->size);

    /* Cannot allocate memory for a stack element. */
    elem = (unsigned int *)malloc(sizeof(unsigned int));
    *elem = (unsigned int)g_random_int();
    alloc_size = 0;
    gnx_alloc_set_limit(alloc_size);
    assert(size == stack->size);
    assert(!gnx_stack_push(stack, elem));
    assert(ENOMEM == errno);
    assert(size == stack->size);

    free(elem);
    gnx_destroy_stack(stack);
    gnx_alloc_reset_limit();
#endif
}

/* Push a random number of elements onto a stack.
 */
static void
push_random(void)
{
    GnxStack *stack;
    unsigned int *elem, i;
    const unsigned int capacity = 32;
    const unsigned int size = (unsigned int)g_random_int_range(2, 33);

    assert(size <= capacity);
    stack = gnx_init_stack_full(&capacity, GNX_FREE_ELEMENTS);

    for (i = 0; i < size; i++) {
        elem = (unsigned int *)malloc(sizeof(unsigned int));
        *elem = (unsigned int)g_random_int();
        assert(gnx_stack_push(stack, elem));
    }
    assert(size == stack->size);

    gnx_destroy_stack(stack);
}

/**************************************************************************
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/stack/new", new);
    g_test_add_func("/stack/peek", peek);
    g_test_add_func("/stack/pop", pop);
    g_test_add_func("/stack/push", push);

    return g_test_run();
}
