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

#include <assert.h>
#include <stdlib.h>

#include "alloc.h"

/**
 * @brief Wrappers for library functions that deal with the allocation and
 *        management of dynamic memory.
 *
 * Wrapper functions are used to test the behavior of the gnx library in
 * out-of-memory scenarios.
 */

/**************************************************************************
 * global variables
 *************************************************************************/

/* An upper limit on the possible number of allocations.  This variable is
 * decremented after each allocation.  When the value is zero, no more
 * allocations are allowed.  To disable the limit on allocations, set this
 * variable to a negative integer; the number -1 would suffice.
 */
static int alloc_limit = -1;

/**************************************************************************
 * public interface
 *************************************************************************/

/**
 * @brief Allocates memory for an array and initializes each element to zero.
 *
 * See your system's documentation of calloc().  The general documentation of
 * calloc() can be found at
 * <a href="http://en.cppreference.com/w/c/memory/calloc">cppreference.com</a>.
 * The parameter documentation is adapted from this website, accessed on
 * 2016-03-03.
 *
 * @param nobject How many objects.
 * @param size The size in bytes of each object.
 * @return If successful, return the pointer to the beginning of a newly
 *         allocated block of memory.  The returned pointer must be deallocated
 *         with free() or realloc().  If the allocation failed, return a null
 *         pointer.
 */
void*
__wrap_calloc(size_t nobject,
              size_t size)
{
#ifdef GNX_ALLOC_TEST
    void *ptr;

    assert(nobject);
    assert(size);

    if (!alloc_limit)
        return NULL;

    ptr = __real_calloc(nobject, size);
    if (!ptr)
        return NULL;

    if (alloc_limit > 0)
        alloc_limit--;

    return ptr;
#else
    return __real_calloc(nobject, size);
#endif
}

/**
 * @brief A wrapper of the malloc() function of the standard C library.
 *
 * See your system's documentation of malloc().  The general documentation of
 * malloc() can be found at
 * <a href="http://en.cppreference.com/w/c/memory/malloc">cppreference.com</a>.
 * The parameter documentation is adapted from this website, accessed on
 * 2016-03-05.
 *
 * @param nbyte How many bytes to allocate.  Must be a positive integer.
 * @return A block of memory with the given number of bytes.  If we are
 *         unable to allocate memory, we return @c NULL.
 */
void*
__wrap_malloc(size_t nbyte)
{
#ifdef GNX_ALLOC_TEST
    void *ptr;

    assert(nbyte);

    if (!alloc_limit)
        return NULL;

    ptr = __real_malloc(nbyte);
    if (!ptr)
        return NULL;

    if (alloc_limit > 0)
        alloc_limit--;

    return ptr;
#else
    assert(nbyte);

    return __real_malloc(nbyte);
#endif
}

/**
 * @brief Reallocates the given block of memory.
 *
 * See your system's documentation of realloc().  The general documentation of
 * realloc() can be found at
 * <a href="http://en.cppreference.com/w/c/memory/realloc">cppreference.com</a>.
 * The parameter documentation is adapted from this website, accessed on
 * 2015-11-24.
 *
 * @param ptr Reallocate this block of memory.
 * @param new_size The new size of the array.
 * @return If the reallocation was successful, return the pointer to the
 *         beginning of the newly allocated memory.  The returned pointer must
 *         be deallocated with free() or realloc().  The original pointer
 *         @a ptr is invalidated and any access to it is undefined behavior
 *         (even if reallocation was in-place).  If the reallocation failed,
 *         return a null pointer.  The original pointer @a ptr remains valid
 *         and may need to be deallocated with free() or realloc().
 */
void*
__wrap_realloc(void *ptr,
               size_t new_size)
{
#ifdef GNX_ALLOC_TEST
    void *new_ptr;

    if (!alloc_limit)
        return NULL;

    new_ptr = __real_realloc(ptr, new_size);
    if (!new_ptr)
        return NULL;

    if (alloc_limit > 0)
        alloc_limit--;

    return new_ptr;
#else
    return __real_alloc(ptr, new_size);
#endif
}

/**
 * @brief Reset the limit on the number of possible allocations.
 *
 * This will remove any allocation limits that were previously set.
 */
void
gnx_alloc_reset_limit(void)
{
    alloc_limit = -1;
}

/**
 * @brief Set the limit on the number of possible allocations.
 *
 * @param count Set the allocation limit to this integer.  A positive value
 *        means that we have at most @c count possible allocations.  A value of
 *        zero means that no allocations are possible.  A negative value means
 *        that there is no limit on the number of possible allocations.
 */
void
gnx_alloc_set_limit(int count)
{
    alloc_limit = count;
}
