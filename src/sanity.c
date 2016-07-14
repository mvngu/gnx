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

#include "sanity.h"

/**
 * @brief A bunch of sanity checks.
 *
 * This is a collection of internal functions for sanity checks.  These
 * functions should not be visible in the public interface.
 */

/**
 * @brief Some sanity checks on a graph.
 *
 * @param graph The graph to check.
 */
void
gnx_i_check(const GnxGraph *graph)
{
    g_return_if_fail(graph);
    g_return_if_fail(graph->node);
}

/**
 * @brief Some sanity checks on an array.
 *
 * @param array The array to check.
 */
void
gnx_i_check_array(const GnxArray *array)
{
    g_return_if_fail(array);
    g_return_if_fail(array->cell);
    g_return_if_fail(array->capacity > 1);
}

/**
 * @brief Some sanity checks for a dictionary.
 *
 * @param dict Perform some sanity checks on this dictionary.
 */
void
gnx_i_check_dict(const GnxDict *dict)
{
    g_return_if_fail(dict);
    g_return_if_fail(dict->bucket);
}

/**
 * @brief Some sanity checks on a binary heap.
 *
 * @param heap Check this binary heap.
 */
void
gnx_i_check_heap(const GnxHeap *heap)
{
    g_return_if_fail(heap);
    g_return_if_fail(heap->map);
    g_return_if_fail(heap->node);
}

/**
 * @brief Some sanity checks on a queue.
 *
 * @param queue The queue to check.
 */
void
gnx_i_check_queue(const GnxQueue *queue)
{
    g_return_if_fail(queue);
    g_return_if_fail(queue->cell);
    g_return_if_fail(queue->capacity > 1);
}

/**
 * @brief Some sanity checks for a set.
 *
 * @param set Perform some sanity checks on this set.
 */
void
gnx_i_check_set(const GnxSet *set)
{
    g_return_if_fail(set);
    g_return_if_fail(set->bucket);
}

/**
 * @brief Some sanity checks for a stack.
 *
 * @param stack Perform some sanity checks on this stack.
 */
void
gnx_i_check_stack(const GnxStack *stack)
{
    g_return_if_fail(stack);
    gnx_i_check_array(stack->array);
}
