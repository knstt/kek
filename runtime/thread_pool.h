#ifndef KEK_RUNTIME_THREAD_POOL_H
#define KEK_RUNTIME_THREAD_POOL_H

#include <stddef.h>

#include <pthread.h>

#define KEK_THREAD_POOL_MAX_WORKERS 32

typedef void (*KekThreadPoolJobFn)(void* context);

typedef struct KekThreadPoolJob {
    KekThreadPoolJobFn run;
    void* context;
} KekThreadPoolJob;

typedef struct KekThreadPool {
    pthread_t workers[KEK_THREAD_POOL_MAX_WORKERS];
    size_t worker_count;
    pthread_mutex_t mutex;
    pthread_cond_t work_available;
    pthread_cond_t work_finished;
    KekThreadPoolJob* jobs;
    size_t job_count;
    size_t next_job;
    size_t active_jobs;
    int stop;
    int initialized;
} KekThreadPool;

void kek_thread_pool_init(KekThreadPool* pool);
void kek_thread_pool_destroy(KekThreadPool* pool);
size_t kek_thread_pool_worker_count(const KekThreadPool* pool);
int kek_thread_pool_run(KekThreadPool* pool, KekThreadPoolJob* jobs,
                        size_t job_count);

#endif
