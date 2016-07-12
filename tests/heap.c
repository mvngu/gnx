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

/* Test the functions in the module src/heap.c. */

#undef G_DISABLE_ASSERT

#include <assert.h>
#include <errno.h>

#include <gnx.h>

/**************************************************************************
 * prototypes for helper functions
 *************************************************************************/

/* new: create and destroy */
static void new_heap(void);
static void new_no_memory(void);

/**************************************************************************
 * new: create and destroy
 *************************************************************************/

static void
new(void)
{
    new_heap();
    new_no_memory();
}

/* Initialize a heap. */
static void
new_heap(void)
{
    GnxHeap *heap;

    heap = gnx_init_heap();
    assert(0 == heap->size);
    gnx_destroy_heap(heap);

    gnx_destroy_heap(NULL);
}

/* Test the function gnx_init_heap() under low-memory scenarios. */
static void
new_no_memory(void)
{
#ifdef GNX_ALLOC_TEST
    /* Cannot allocate memory for the heap. */
    gnx_alloc_set_limit(0);
    assert(!gnx_init_heap());
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

    g_test_add_func("/heap/new", new);

    return g_test_run();
}
