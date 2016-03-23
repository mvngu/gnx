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
 * start here
 *************************************************************************/

int
main(int argc,
     char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/stack/new", new);

    return g_test_run();
}
