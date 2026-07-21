#ifndef KEK_RUNTIME_TRACE_H
#define KEK_RUNTIME_TRACE_H

#include <stddef.h>
#include <stdint.h>

#define KEK_TRACE_MAX_RUNTIME_METRICS 64
#define KEK_TRACE_MAX_HOOK_METRICS 64
#define KEK_TRACE_PATH_CAPACITY 1024

struct KekEvent;
struct KekHookDescriptor;
struct KekRuntime;

typedef struct KekTraceRuntimeMetric {
    const char* name;
    uint64_t count;
    uint64_t total_ns;
    uint64_t min_ns;
    uint64_t max_ns;
} KekTraceRuntimeMetric;

typedef enum KekTraceRuntimeMetricId {
    KEK_TRACE_METRIC_EVENT_PUBLISH = 0,
    KEK_TRACE_METRIC_EVENT_DISPATCH_PENDING,
    KEK_TRACE_METRIC_EVENT_SUBSCRIBER_DISPATCH,
    KEK_TRACE_METRIC_RUNTIME_STATE_PREPARE,
    KEK_TRACE_METRIC_RUNTIME_STATE_READY,
    KEK_TRACE_METRIC_RUNTIME_SELECT_WAIT,
    KEK_TRACE_METRIC_RUNTIME_MALLOC,
    KEK_TRACE_METRIC_RUNTIME_FREE,
    KEK_TRACE_METRIC_STATE_STORE_INIT_COPY,
    KEK_TRACE_METRIC_STATE_STORE_DRAFT_COPY,
    KEK_TRACE_METRIC_STATE_STORE_UPDATE_CALLBACK,
    KEK_TRACE_METRIC_STATE_STORE_VALIDATION,
    KEK_TRACE_METRIC_STATE_STORE_TRANSACTION_COPY,
    KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
    KEK_TRACE_METRIC_HOOK_PARALLEL_WAVE,
    KEK_TRACE_METRIC_HOOK_PARALLEL_READONLY_WAVE,
    KEK_TRACE_METRIC_HOOK_PARALLEL_WRITE_WAVE,
    KEK_TRACE_METRIC_HOOK_FIELD_MERGE_WAVE,
    KEK_TRACE_METRIC_HOOK_SERIAL_DESCRIPTOR,
    KEK_TRACE_METRIC_HOOK_SERIAL_FALLBACK,
    KEK_TRACE_METRIC_HOOK_WORKER_CLONE,
    KEK_TRACE_METRIC_HOOK_WORKER_CLONE_BYTES,
    KEK_TRACE_METRIC_HOOK_WORKER_APPLY,
    KEK_TRACE_METRIC_HOOK_WORKER_EVENT_REPLAY,
    KEK_TRACE_METRIC_STATE_ARENA_HIGH_WATER,
    KEK_TRACE_METRIC_STATE_ARENA_RESET,
    KEK_TRACE_METRIC_HOOK_OVERLAY_DRAFT_BYTES,
    KEK_TRACE_METRIC_HOOK_OVERLAY_ENTRY,
    KEK_TRACE_RUNTIME_METRIC_ID_COUNT
} KekTraceRuntimeMetricId;

typedef struct KekTraceHookMetric {
    const char* hook_name;
    int event_type;
    size_t state_type_id;
    size_t state_slot_id;
    uint64_t call_count;
    uint64_t success_count;
    uint64_t failure_count;
    uint64_t total_wait_ns;
    uint64_t min_wait_ns;
    uint64_t max_wait_ns;
    uint64_t total_run_ns;
    uint64_t min_run_ns;
    uint64_t max_run_ns;
} KekTraceHookMetric;

typedef struct KekRuntimeTrace {
    int enabled;
    int runtime_csv_enabled;
    int hooks_csv_enabled;
    int subscriber_timing_enabled;
    char runtime_csv_path[KEK_TRACE_PATH_CAPACITY];
    char hooks_csv_path[KEK_TRACE_PATH_CAPACITY];
    KekTraceRuntimeMetric runtime_metrics[KEK_TRACE_MAX_RUNTIME_METRICS];
    size_t runtime_metric_count;
    KekTraceHookMetric hook_metrics[KEK_TRACE_MAX_HOOK_METRICS];
    size_t hook_metric_count;
    uint64_t current_bytes;
    uint64_t peak_bytes;
    uint64_t total_allocated_bytes;
    uint64_t total_freed_bytes;
    uint64_t allocation_count;
    uint64_t free_count;
} KekRuntimeTrace;

uint64_t kek_trace_now_ns(void);
void kek_trace_init(struct KekRuntime* runtime);
void kek_trace_destroy(struct KekRuntime* runtime);
int kek_trace_enabled(const struct KekRuntime* runtime);
void kek_trace_record_runtime(struct KekRuntime* runtime, const char* name,
                              uint64_t duration_ns);
void kek_trace_record_runtime_metric(struct KekRuntime* runtime,
                                     KekTraceRuntimeMetricId metric_id,
                                     uint64_t duration_ns);
void kek_trace_count_runtime_metric(struct KekRuntime* runtime,
                                    KekTraceRuntimeMetricId metric_id);
void kek_trace_record_hook(struct KekRuntime* runtime,
                           const struct KekHookDescriptor* descriptor,
                           const struct KekEvent* event,
                           uint64_t wait_ns,
                           uint64_t run_ns,
                           int ok);
void* kek_trace_malloc(struct KekRuntime* runtime, size_t size);
void kek_trace_free(struct KekRuntime* runtime, void* ptr, size_t size);

#endif
