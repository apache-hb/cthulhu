// SPDX-License-Identifier: LGPL-3.0-or-later
#include "os_common.h"

#include "base/panic.h"

static void *thread_fn(void *arg)
{
    os_thread_t *thread = arg;

    return (void *)(uintptr_t)thread->fn(thread->arg);
}

USE_DECL
os_error_t os_thread_init(
    os_thread_t *thread,
    const char *name,
    os_thread_fn_t fn,
    void *arg)
{
    CTASSERT(thread != NULL);
    CTASSERT(name != NULL);
    CTASSERT(fn != NULL);

    thread->name = name;
    thread->fn = fn;
    thread->arg = arg;

    if (pthread_create(&thread->impl, NULL, thread_fn, thread) != 0)
    {
        return errno;
    }

    thread->id = thread->impl;

    return eOsSuccess;
}

USE_DECL
os_error_t os_thread_join(os_thread_t *thread, os_status_t *status)
{
    CTASSERT(thread != NULL);
    CTASSERT(status != NULL);

    void *result = NULL;

    if (pthread_join(thread->impl, &result) != 0)
    {
        return errno;
    }

    *status = (os_status_t)(uintptr_t)result;

    return eOsSuccess;
}

os_thread_id_t os_get_thread_id(void)
{
    return pthread_self();
}
