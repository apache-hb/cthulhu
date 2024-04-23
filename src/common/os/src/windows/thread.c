// SPDX-License-Identifier: LGPL-3.0-or-later
#include "os_common.h"

#include "base/panic.h"

// TODO: naming threads requires some pretty arcane win32 calls

static DWORD WINAPI thread_fn(LPVOID param)
{
    os_thread_t *thread = param;

    return thread->fn(thread->arg);
}

STA_DECL
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
    thread->impl = NULL;

    DWORD id = 0;

    HANDLE handle = CreateThread(NULL, 0, thread_fn, thread, 0, &id);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    thread->impl = handle;
    thread->id = id;

    return eOsSuccess;
}

STA_DECL
os_error_t os_thread_join(os_thread_t *thread, os_status_t *status)
{
    CTASSERT(thread != NULL);
    CTASSERT(status != NULL);

    if (WaitForSingleObject(thread->impl, INFINITE) != WAIT_OBJECT_0)
    {
        return GetLastError();
    }

    DWORD result = 0;

    if (GetExitCodeThread(thread->impl, &result) == 0)
    {
        return GetLastError();
    }

    *status = result;

    return eOsSuccess;
}

os_thread_id_t os_get_thread_id(void)
{
    return GetCurrentThreadId();
}
