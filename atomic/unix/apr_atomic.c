/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2003 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#include "apr.h"
#include "apr_atomic.h"
#include "apr_thread_mutex.h"

#if defined(WIN32)

#define apr_atomic_t LONG /* not used */

APR_DECLARE(apr_status_t) apr_atomic_init(apr_pool_t *p)
{
    return APR_SUCCESS;
}

APR_DECLARE(void *) apr_atomic_casptr(volatile void **mem, void *with, const void *cmp)
{
    return InterlockedCompareExchangePointer(mem, with, cmp);
}

/* 
 * Remapping function pointer type to accept apr_uint32_t's type-safely
 * as the arguments for as our apr_atomic_foo32 Functions
 */
typedef WINBASEAPI apr_uint32_t (WINAPI * apr_atomic_win32_ptr_fn)
                                           (apr_uint32_t volatile *);
typedef WINBASEAPI apr_uint32_t (WINAPI * apr_atomic_win32_ptr_val_fn)
                                           (apr_uint32_t volatile *, 
                                            apr_uint32_t);
typedef WINBASEAPI apr_uint32_t (WINAPI * apr_atomic_win32_ptr_val_val_fn)
                                           (apr_uint32_t volatile *, 
                                            apr_uint32_t, apr_uint32_t);

APR_DECLARE(void) apr_atomic_add32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    ((apr_atomic_win32_ptr_val_fn)InterlockedExchangeAdd)(mem, val);
}
#define APR_OVERRIDE_ATOMIC_ADD32

APR_DECLARE(void) apr_atomic_sub32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    ((apr_atomic_win32_ptr_val_fn)InterlockedExchangeAdd)(mem, -val);
}
#define APR_OVERRIDE_ATOMIC_SUB32

APR_DECLARE(void) apr_atomic_inc32(volatile apr_uint32_t *mem)
{
    ((apr_atomic_win32_ptr_fn)InterlockedIncrement)(mem);
}
#define APR_OVERRIDE_ATOMIC_INC32

APR_DECLARE(int) apr_atomic_dec32(volatile apr_uint32_t *mem)
{
    return ((apr_atomic_win32_ptr_fn)InterlockedDecrement)(mem);
}
#define APR_OVERRIDE_ATOMIC_DEC32

APR_DECLARE(void) apr_atomic_set32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    ((apr_atomic_win32_ptr_val_fn)InterlockedExchange)(mem, val);
}
#define APR_OVERRIDE_ATOMIC_SET32

APR_DECLARE(apr_uint32_t) apr_atomic_cas32(volatile apr_uint32_t *mem, apr_uint32_t with,
                                           apr_uint32_t cmp)
{
    return ((apr_atomic_win32_ptr_val_val_fn)InterlockedCompareExchange)(mem, with, cmp);
}
#define APR_OVERRIDE_ATOMIC_CAS32

APR_DECLARE(apr_uint32_t) apr_atomic_xchg32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    return ((apr_atomic_win32_ptr_val_fn)InterlockedExchange)(mem, val);
}
#define APR_OVERRIDE_ATOMIC_XCHG32

#endif /* WIN32 */

#if defined(__FreeBSD__) && !defined(__i386__) && !APR_FORCE_ATOMIC_GENERIC

#include <machine/atomic.h>

#define apr_atomic_t apr_uint32_t /* unused */

APR_DECLARE(void) apr_atomic_add32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    atomic_add_int(mem, val);
}
#define APR_OVERRIDE_ATOMIC_ADD32

APR_DECLARE(int) apr_atomic_dec32(volatile apr_uint32_t *mem)
{
    return atomic_subtract_int(mem, 1);
}
#define APR_OVERRIDE_ATOMIC_DEC32

APR_DECLARE(void) apr_atomic_inc32(volatile apr_uint32_t *mem)
{
    atomic_add_int(mem, 1);
}
#define APR_OVERRIDE_ATOMIC_INC32

APR_DECLARE(void) apr_atomic_set32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    atomic_set_int(mem, val);
}
#define APR_OVERRIDE_ATOMIC_SET32

#endif /* __FreeBSD__ && !__i386__ */

#if (defined(__linux__) || defined(__EMX__) || defined(__FreeBSD__)) \
        && defined(__i386__) && !APR_FORCE_ATOMIC_GENERIC

/* #define apr_atomic_t apr_uint32_t UNUSED */

APR_DECLARE(apr_uint32_t) apr_atomic_cas32(volatile apr_uint32_t *mem, 
                                           apr_uint32_t with,
                                           apr_uint32_t cmp)
{
    apr_uint32_t prev;

    asm volatile ("lock; cmpxchgl %1, %2"             
                  : "=a" (prev)               
                  : "r" (with), "m" (*(mem)), "0"(cmp) 
                  : "memory");
    return prev;
}
#define APR_OVERRIDE_ATOMIC_CAS32

APR_DECLARE(void) apr_atomic_add32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    asm volatile ("lock; addl %1, %0"                              
                  :                                                           
                  : "m" (*(mem)), "r" (val)                                   
                  : "memory");
}
#define APR_OVERRIDE_ATOMIC_ADD32

APR_DECLARE(void) apr_atomic_sub32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    asm volatile ("lock; subl %1, %0"
                  :
                  : "m" (*(mem)), "r" (val)
                  : "memory");
}
#define APR_OVERRIDE_ATOMIC_SUB32

APR_DECLARE(int) apr_atomic_dec32(volatile apr_uint32_t *mem)
{
    int prev;

    asm volatile ("mov $0, %%eax;\n\t"
                  "lock; decl %1;\n\t"
                  "setnz %%al;\n\t"
                  "mov %%eax, %0"
                  : "=r" (prev)
                  : "m" (*(mem))
                  : "memory", "%eax");
    return prev;
}
#define APR_OVERRIDE_ATOMIC_DEC32

APR_DECLARE(void) apr_atomic_inc32(volatile apr_uint32_t *mem)
{
    asm volatile ("lock; incl %0"
                  :
                  : "m" (*(mem))
                  : "memory");
}
#define APR_OVERRIDE_ATOMIC_INC32

APR_DECLARE(void) apr_atomic_set32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    *mem = val;
}
#define APR_OVERRIDE_ATOMIC_SET32

APR_DECLARE(apr_uint32_t) apr_atomic_xchg32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    apr_uint32_t prev = val;

    asm volatile ("lock; xchgl %0, %1"
                  : "=r" (prev)
                  : "m" (*(mem)), "0"(prev)
                  : "memory");
    return prev;
}
#define APR_OVERRIDE_ATOMIC_XCHG32

/*#define apr_atomic_init(pool)        APR_SUCCESS*/

#endif /* (__linux__ || __EMX__ || __FreeBSD__) && __i386__ */

#if !defined(apr_atomic_init) && !defined(APR_OVERRIDE_ATOMIC_INIT)

#if APR_HAS_THREADS
#define NUM_ATOMIC_HASH 7
/* shift by 2 to get rid of alignment issues */
#define ATOMIC_HASH(x) (unsigned int)(((unsigned long)(x)>>2)%(unsigned int)NUM_ATOMIC_HASH)
static apr_thread_mutex_t **hash_mutex;
#endif /* APR_HAS_THREADS */

apr_status_t apr_atomic_init(apr_pool_t *p)
{
#if APR_HAS_THREADS
    int i;
    apr_status_t rv;
    hash_mutex = apr_palloc(p, sizeof(apr_thread_mutex_t*) * NUM_ATOMIC_HASH);

    for (i = 0; i < NUM_ATOMIC_HASH; i++) {
        rv = apr_thread_mutex_create(&(hash_mutex[i]),
                                     APR_THREAD_MUTEX_DEFAULT, p);
        if (rv != APR_SUCCESS) {
           return rv;
        }
    }
#endif /* APR_HAS_THREADS */
    return APR_SUCCESS;
}
#endif /*!defined(apr_atomic_init) && !defined(APR_OVERRIDE_ATOMIC_INIT) */

#if !defined(apr_atomic_add32) && !defined(APR_OVERRIDE_ATOMIC_ADD32)
void apr_atomic_add32(volatile apr_uint32_t *mem, apr_uint32_t val) 
{
#if APR_HAS_THREADS
    apr_thread_mutex_t *lock = hash_mutex[ATOMIC_HASH(mem)];
       
    if (apr_thread_mutex_lock(lock) == APR_SUCCESS) {
        *mem += val;
        apr_thread_mutex_unlock(lock);
    }
#else
    *mem += val;
#endif /* APR_HAS_THREADS */
}
#endif /*!defined(apr_atomic_sub32) && !defined(APR_OVERRIDE_ATOMIC_SUB32) */

#if !defined(apr_atomic_sub32) && !defined(APR_OVERRIDE_ATOMIC_SUB32)
void apr_atomic_sub32(volatile apr_uint32_t *mem, apr_uint32_t val) 
{
#if APR_HAS_THREADS
    apr_thread_mutex_t *lock = hash_mutex[ATOMIC_HASH(mem)];
       
    if (apr_thread_mutex_lock(lock) == APR_SUCCESS) {
        *mem -= val;
        apr_thread_mutex_unlock(lock);
    }
#else
    *mem -= val;
#endif /* APR_HAS_THREADS */
}
#endif /*!defined(apr_atomic_sub32) && !defined(APR_OVERRIDE_ATOMIC_SUB32) */

#if !defined(apr_atomic_set32) && !defined(APR_OVERRIDE_ATOMIC_SET32)
void apr_atomic_set32(volatile apr_uint32_t *mem, apr_uint32_t val) 
{
#if APR_HAS_THREADS
    apr_thread_mutex_t *lock = hash_mutex[ATOMIC_HASH(mem)];

    if (apr_thread_mutex_lock(lock) == APR_SUCCESS) {
        *mem = val;
        apr_thread_mutex_unlock(lock);
    }
#else
    *mem = val;
#endif /* APR_HAS_THREADS */
}
#endif /*!defined(apr_atomic_set32) && !defined(APR_OVERRIDE_ATOMIC_SET32) */

#if !defined(apr_atomic_inc32) && !defined(APR_OVERRIDE_ATOMIC_INC32)
void apr_atomic_inc32(volatile apr_uint32_t *mem) 
{
#if APR_HAS_THREADS
    apr_thread_mutex_t *lock = hash_mutex[ATOMIC_HASH(mem)];

    if (apr_thread_mutex_lock(lock) == APR_SUCCESS) {
        (*mem)++;
        apr_thread_mutex_unlock(lock);
    }
#else
    (*mem)++;
#endif /* APR_HAS_THREADS */
}
#endif /*!defined(apr_atomic_inc32) && !defined(APR_OVERRIDE_ATOMIC_INC32) */

#if !defined(apr_atomic_dec32) && !defined(APR_OVERRIDE_ATOMIC_DEC32)
int apr_atomic_dec32(volatile apr_uint32_t *mem) 
{
#if APR_HAS_THREADS
    apr_thread_mutex_t *lock = hash_mutex[ATOMIC_HASH(mem)];
    apr_uint32_t new;

    if (apr_thread_mutex_lock(lock) == APR_SUCCESS) {
        (*mem)--;
        new = *mem;
        apr_thread_mutex_unlock(lock);
        return new; 
    }
#else
    (*mem)--;
#endif /* APR_HAS_THREADS */
    return *mem; 
}
#endif /*!defined(apr_atomic_dec32) && !defined(APR_OVERRIDE_ATOMIC_DEC32) */

#if !defined(apr_atomic_cas32) && !defined(APR_OVERRIDE_ATOMIC_CAS32)
apr_uint32_t apr_atomic_cas32(volatile apr_uint32_t *mem, apr_uint32_t with,
			      apr_uint32_t cmp)
{
    apr_uint32_t prev;
#if APR_HAS_THREADS
    apr_thread_mutex_t *lock = hash_mutex[ATOMIC_HASH(mem)];

    if (apr_thread_mutex_lock(lock) == APR_SUCCESS) {
        prev = *mem;
        if (prev == cmp) {
            *mem = with;
        }
        apr_thread_mutex_unlock(lock);
        return prev;
    }
    return *mem;
#else
    prev = *mem;
    if (prev == cmp) {
        *mem = with;
    }
    return prev;
#endif /* APR_HAS_THREADS */
}
#endif /*!defined(apr_atomic_cas32) && !defined(APR_OVERRIDE_ATOMIC_CAS32) */

#if !defined(apr_atomic_xchg32) && !defined(APR_OVERRIDE_ATOMIC_XCHG32)
apr_uint32_t apr_atomic_xchg32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    apr_uint32_t prev;
#if APR_HAS_THREADS
    apr_thread_mutex_t *lock = hash_mutex[ATOMIC_HASH(mem)];

    if (apr_thread_mutex_lock(lock) == APR_SUCCESS) {
        prev = *mem;
        *mem = val;
        apr_thread_mutex_unlock(lock);
        return prev;
    }
    return *mem;
#else
    prev = *mem;
    *mem = val;
    return prev;
#endif /* APR_HAS_THREADS */
}
#endif /*!defined(apr_atomic_xchg32) && !defined(APR_OVERRIDE_ATOMIC_XCHG32) */

#if !defined(apr_atomic_casptr) && !defined(APR_OVERRIDE_ATOMIC_CASPTR)
void *apr_atomic_casptr(volatile void **mem, void *with, const void *cmp)
{
    void *prev;
#if APR_HAS_THREADS
    apr_thread_mutex_t *lock = hash_mutex[ATOMIC_HASH(mem)];

    if (apr_thread_mutex_lock(lock) == APR_SUCCESS) {
        prev = *(void **)mem;
        if (prev == cmp) {
            *mem = with;
        }
        apr_thread_mutex_unlock(lock);
        return prev;
    }
    return *(void **)mem;
#else
    prev = *(void **)mem;
    if (prev == cmp) {
        *mem = with;
    }
    return prev;
#endif /* APR_HAS_THREADS */
}
#endif /*!defined(apr_atomic_casptr) && !defined(APR_OVERRIDE_ATOMIC_CASPTR) */

#if !defined(APR_OVERRIDE_ATOMIC_READ32)
APR_DECLARE(apr_uint32_t) apr_atomic_read32(volatile apr_uint32_t *mem)
{
    return *mem;
}
#endif

