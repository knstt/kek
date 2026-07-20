#define _POSIX_C_SOURCE 200809L

#include "thread_pool.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __APPLE__
int sysctlbyname(const char* name, void* oldp, size_t* oldlenp,
                 void* newp, size_t newlen);
#endif

static size_t thread_pool_auto_count(void) {
    const char* setting = getenv("KEK_RUNTIME_THREADS");
    if (setting && setting[0] != '\0' && strcmp(setting, "auto") != 0) {
        char* end = NULL;
        errno = 0;
        unsigned long parsed = strtoul(setting, &end, 10);
        if (errno == 0 && end && *end == '\0' && parsed > 0) {
            if (parsed == 1) {
                return 0;
            }
            if (parsed - 1 > KEK_THREAD_POOL_MAX_WORKERS) {
                return KEK_THREAD_POOL_MAX_WORKERS;
            }
            return (size_t)parsed - 1;
        }
    }

    long cpu_count = 2;
#ifdef _SC_NPROCESSORS_ONLN
    cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(__APPLE__)
    int value = 0;
    size_t value_size = sizeof(value);
    if (sysctlbyname("hw.logicalcpu", &value, &value_size, NULL, 0) == 0 &&
        value > 0) {
        cpu_count = value;
    }
#endif
    if (cpu_count <= 2) {
        return 0;
    }
    size_t workers = (size_t)cpu_count - 1;
    if (workers > 4) {
        workers = 4;
    }
    if (workers > KEK_THREAD_POOL_MAX_WORKERS) {
        workers = KEK_THREAD_POOL_MAX_WORKERS;
    }
    return workers;
}

static void* thread_pool_worker(void* context) {
    KekThreadPool* pool = (KekThreadPool*)context;
    for (;;) {
        pthread_mutex_lock(&pool->mutex);
        while (!pool->stop && pool->next_job >= pool->job_count) {
            pthread_cond_wait(&pool->work_available, &pool->mutex);
        }
        if (pool->stop) {
            pthread_mutex_unlock(&pool->mutex);
            return NULL;
        }

        KekThreadPoolJob job = pool->jobs[pool->next_job++];
        pool->active_jobs++;
        pthread_mutex_unlock(&pool->mutex);

        if (job.run) {
            job.run(job.context);
        }

        pthread_mutex_lock(&pool->mutex);
        pool->active_jobs--;
        if (pool->next_job >= pool->job_count && pool->active_jobs == 0) {
            pthread_cond_signal(&pool->work_finished);
        }
        pthread_mutex_unlock(&pool->mutex);
    }
}

void kek_thread_pool_init(KekThreadPool* pool) {
    if (!pool) {
        return;
    }

    memset(pool, 0, sizeof(*pool));
    if (pthread_mutex_init(&pool->mutex, NULL) != 0) {
        return;
    }
    if (pthread_cond_init(&pool->work_available, NULL) != 0) {
        pthread_mutex_destroy(&pool->mutex);
        return;
    }
    if (pthread_cond_init(&pool->work_finished, NULL) != 0) {
        pthread_cond_destroy(&pool->work_available);
        pthread_mutex_destroy(&pool->mutex);
        return;
    }

    pool->initialized = 1;
    size_t worker_count = thread_pool_auto_count();
    for (size_t i = 0; i < worker_count; i++) {
        if (pthread_create(&pool->workers[i], NULL, thread_pool_worker, pool) != 0) {
            break;
        }
        pool->worker_count++;
    }
}

void kek_thread_pool_destroy(KekThreadPool* pool) {
    if (!pool || !pool->initialized) {
        return;
    }

    pthread_mutex_lock(&pool->mutex);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->work_available);
    pthread_mutex_unlock(&pool->mutex);

    for (size_t i = 0; i < pool->worker_count; i++) {
        pthread_join(pool->workers[i], NULL);
    }

    pthread_cond_destroy(&pool->work_finished);
    pthread_cond_destroy(&pool->work_available);
    pthread_mutex_destroy(&pool->mutex);
    memset(pool, 0, sizeof(*pool));
}

size_t kek_thread_pool_worker_count(const KekThreadPool* pool) {
    return pool ? pool->worker_count : 0;
}

int kek_thread_pool_run(KekThreadPool* pool, KekThreadPoolJob* jobs,
                        size_t job_count) {
    if (!jobs && job_count > 0) {
        return 0;
    }
    if (job_count == 0) {
        return 1;
    }
    if (!pool || !pool->initialized || pool->worker_count == 0 ||
        job_count == 1) {
        for (size_t i = 0; i < job_count; i++) {
            if (jobs[i].run) {
                jobs[i].run(jobs[i].context);
            }
        }
        return 1;
    }

    pthread_mutex_lock(&pool->mutex);
    pool->jobs = jobs;
    pool->job_count = job_count;
    pool->next_job = 0;
    pool->active_jobs = 0;
    pthread_cond_broadcast(&pool->work_available);
    while (pool->next_job < pool->job_count || pool->active_jobs > 0) {
        pthread_cond_wait(&pool->work_finished, &pool->mutex);
    }
    pool->jobs = NULL;
    pool->job_count = 0;
    pool->next_job = 0;
    pthread_mutex_unlock(&pool->mutex);
    return 1;
}
