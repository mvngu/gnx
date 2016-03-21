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
