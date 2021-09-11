#pragma once

#include <pthread.h>

typedef pthread_mutex_t ctu_critical_t;
#define CTU_CRITICAL_INIT PTHREAD_MUTEX_INITIALIZER
#define CTU_CRITICAL_LOCK(LOCK) pthread_mutex_lock(LOCK)
#define CTU_CRITICAL_UNLOCK(LOCK) pthread_mutex_unlock(LOCK)
#define CTU_CRITICAL(LOCK, ...) do { CTU_CRITICAL_LOCK(LOCK); { __VA_ARGS__ } CTU_CRITICAL_UNLOCK(LOCK); } while(0)
