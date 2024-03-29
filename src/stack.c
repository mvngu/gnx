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

#include <glib.h>

#include "array.h"
#include "sanity.h"
#include "stack.h"

/**
 * @file stack.h
 * @brief A stack of integers.
 *
 * A stack does not make its own copy of any element that is pushed onto it.
 * Rather, the stack copies the pointer that is passed in and stores that
 * pointer.  It is your responsibility to ensure that any element that is
 * pushed onto the stack exists for the duration of the stack itself.  For
 * further details on the stack data structure, see the
 * <a href="https://en.wikipedia.org/wiki/Stack_(abstract_data_type)">Wikipedia article</a>.
 *
 * Use either of the functions gnx_init_stack() or gnx_init_stack_full() to
 * initialize a new stack.  Destroy a stack via the function
 * gnx_destroy_stack().
 */

/**
 * @brief Destroys a stack.
 *
 * @param stack The stack to destroy.  The stack must have been initialized via
 *        gnx_init_stack() or gnx_init_stack_full().
 */
void
gnx_destroy_stack(GnxStack *stack)
{
    if (!stack)
        return;
    gnx_destroy_array(stack->array);
    free(stack);
    stack = NULL;
}

/**
 * @brief Initializes a stack of integers with default settings.
 *
 * The stack is initialized with a default capacity.  Furthermore, the stack is
 * set to not release the memory of each of its elements.  Thus, it is your
 * responsibility to ensure that the memory of each element pushed onto the
 * stack is released.
 *
 * @sa gnx_init_stack_full() Initializes a stack with full control over its
 *     settings.
 *
 * @return An initialized stack with zero elements and a default capacity of
 *         #GNX_DEFAULT_ALLOC_SIZE.  When you no longer need the stack, you
 *         must destroy the stack via the function gnx_destroy_stack().  See
 *         gnx_init_stack_full() for further details on the return value.
 */
GnxStack*
gnx_init_stack(void)
{
    const unsigned int capacity = GNX_DEFAULT_ALLOC_SIZE;
    return gnx_init_stack_full(&capacity, GNX_DONT_FREE_ELEMENTS);
}

/**
 * @brief Initializes a stack of integers.
 *
 * @param capacity The initial capacity of the stack.  This is assumed to be a
 *        positive power of two, but must not exceed #GNX_MAXIMUM_ELEMENTS.  A
 *        large initial capacity ensures that we do not need to constantly
 *        resize the stack when we push a large number of elements.
 * @param destroy Whether to release the memory of each element as part of the
 *        destroy procedure.  Possible values are #GNX_FREE_ELEMENTS or
 *        #GNX_DONT_FREE_ELEMENTS.  If #GNX_FREE_ELEMENTS, then calling the
 *        function gnx_destroy_stack() will release the memory of each element
 *        of the stack, in addition to destroying the stack itself.  This
 *        option should only be used if each element to be pushed has memory
 *        that is allocated on the heap, i.e. via @c calloc(), @c malloc(), or
 *        @c realloc().  Using this option with stack memory will result in
 *        undefined behavior. If #GNX_DONT_FREE_ELEMENTS, then it is your
 *        responsibility to release the memory of each element in the stack.
 *        You can also use this option if each element has memory that is
 *        allocated on the stack.
 * @return An initialized stack with the given capacity and zero elements.
 *         When you no longer need the stack, you must destroy the stack via
 *         the function gnx_destroy_stack().  If we are unable to allocate
 *         memory, then @c errno is set to @c ENOMEM and we return @c NULL.
 */
GnxStack*
gnx_init_stack_full(const unsigned int *capacity,
                    const GnxBool destroy)
{
    GnxStack *stack;

    errno = 0;
    stack = (GnxStack *)malloc(sizeof(GnxStack));
    if (!stack)
        goto cleanup;

    /* The given parameters will be sanity checked when we initialize the
     * array.  Thus we do not need to sanity check the arguments here.
     */
    stack->array = gnx_init_array_full(capacity, destroy, GNX_UINT);
    if (!stack->array)
        goto cleanup;

    stack->size = 0;
    return stack;

cleanup:
    errno = ENOMEM;
    gnx_destroy_stack(stack);
    return NULL;
}

/**
 * @brief Peeks at the content at the top of a stack.
 *
 * @param stack We want to peek at the top of this stack.
 * @return The element at the top of the given stack.  If the stack is empty,
 *         then we return @c NULL.
 */
unsigned int*
gnx_stack_peek(const GnxStack *stack)
{
    gnx_i_check_stack(stack);
    if (!stack->size)
        return NULL;

    return (unsigned int *)(stack->array->cell[stack->size - 1]);
}

/**
 * @brief Pops an element off a stack.
 *
 * @param stack We want to pop an element off this stack.  The stack must not
 *        be empty.
 * @return The element at the top of the stack.  It is your responsibility to
 *         ensure that when you no longer need the element, you release the
 *         memory of the element as appropriate.  If the stack is empty, then
 *         we return @c NULL.
 */
unsigned int*
gnx_stack_pop(GnxStack *stack)
{
    unsigned int *elem;

    gnx_i_check_stack(stack);
    if (!stack->size)
        return NULL;

    (stack->size)--;
    elem = (unsigned int *)(stack->array->cell[stack->size]);
    stack->array->cell[stack->size] = NULL;
    (stack->array->size)--;
    g_assert(stack->size == stack->array->size);

    return elem;
}

/**
 * @brief Pushes an element on top of a stack.
 *
 * @param stack We want to push an element onto this stack.
 * @param elem Push this element onto the given stack.  It is your
 *        responsibility to ensure that this element exists for the duration of
 *        the stack.  With this element, the size of the stack must not exceed
 *        #GNX_MAXIMUM_ELEMENTS.  In other words, we assume that the current
 *        size of the stack is less than #GNX_MAXIMUM_ELEMENTS.
 * @return Nonzero if we successfully pushed the given element onto the stack;
 *         zero otherwise.  If we are unable to allocate memory, then we set
 *         @c errno to @c ENOMEM and return zero.
 */
int
gnx_stack_push(GnxStack *stack,
               unsigned int *elem)
{
    errno = 0;
    gnx_i_check_stack(stack);
    g_return_val_if_fail(elem, GNX_FAILURE);
    g_return_val_if_fail(stack->size < GNX_MAXIMUM_ELEMENTS, GNX_FAILURE);

    if (!gnx_array_append(stack->array, elem)) {
        errno = ENOMEM;
        return GNX_FAILURE;
    }

    (stack->size)++;
    g_assert(stack->size == stack->array->size);
    return GNX_SUCCESS;
}
