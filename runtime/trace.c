#define _POSIX_C_SOURCE 200809L

#include "trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "event.h"
#include "hook.h"
#include "runtime.h"

static const char* const trace_runtime_metric_names[] = {
    [KEK_TRACE_METRIC_EVENT_PUBLISH] = "event_publish",
    [KEK_TRACE_METRIC_EVENT_DISPATCH_PENDING] = "event_dispatch_pending",
    [KEK_TRACE_METRIC_EVENT_SUBSCRIBER_DISPATCH] = "event_subscriber_dispatch",
    [KEK_TRACE_METRIC_RUNTIME_STATE_PREPARE] = "runtime_state_prepare",
    [KEK_TRACE_METRIC_RUNTIME_STATE_READY] = "runtime_state_ready",
    [KEK_TRACE_METRIC_RUNTIME_SELECT_WAIT] = "runtime_select_wait",
    [KEK_TRACE_METRIC_RUNTIME_MALLOC] = "runtime_malloc",
    [KEK_TRACE_METRIC_RUNTIME_FREE] = "runtime_free",
    [KEK_TRACE_METRIC_STATE_STORE_INIT_COPY] = "state_store_init_copy",
    [KEK_TRACE_METRIC_STATE_STORE_DRAFT_COPY] = "state_store_draft_copy",
    [KEK_TRACE_METRIC_STATE_STORE_UPDATE_CALLBACK] =
        "state_store_update_callback",
    [KEK_TRACE_METRIC_STATE_STORE_VALIDATION] = "state_store_validation",
    [KEK_TRACE_METRIC_STATE_STORE_TRANSACTION_COPY] =
        "state_store_transaction_copy",
    [KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY] = "state_store_rollback_copy",
    [KEK_TRACE_METRIC_HOOK_PARALLEL_WAVE] = "hook_parallel_wave",
    [KEK_TRACE_METRIC_HOOK_PARALLEL_READONLY_WAVE] =
        "hook_parallel_readonly_wave",
    [KEK_TRACE_METRIC_HOOK_PARALLEL_WRITE_WAVE] = "hook_parallel_write_wave",
    [KEK_TRACE_METRIC_HOOK_FIELD_MERGE_WAVE] = "hook_field_merge_wave",
    [KEK_TRACE_METRIC_HOOK_SERIAL_DESCRIPTOR] = "hook_serial_descriptor",
    [KEK_TRACE_METRIC_HOOK_SERIAL_FALLBACK] = "hook_serial_fallback",
    [KEK_TRACE_METRIC_HOOK_WORKER_CLONE] = "hook_worker_clone",
    [KEK_TRACE_METRIC_HOOK_WORKER_CLONE_BYTES] = "hook_worker_clone_bytes",
    [KEK_TRACE_METRIC_HOOK_WORKER_APPLY] = "hook_worker_apply",
    [KEK_TRACE_METRIC_HOOK_WORKER_EVENT_REPLAY] = "hook_worker_event_replay",
    [KEK_TRACE_METRIC_STATE_ARENA_HIGH_WATER] = "state_arena_high_water",
    [KEK_TRACE_METRIC_STATE_ARENA_RESET] = "state_arena_reset",
    [KEK_TRACE_METRIC_HOOK_OVERLAY_DRAFT_BYTES] = "hook_overlay_draft_bytes",
    [KEK_TRACE_METRIC_HOOK_OVERLAY_ENTRY] = "hook_overlay_entry",
};

uint64_t kek_trace_now_ns(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        return 0;
    }
    return (uint64_t)now.tv_sec * 1000000000ull + (uint64_t)now.tv_nsec;
}

static void trace_copy_path(char target[KEK_TRACE_PATH_CAPACITY],
                            const char* source) {
    if (!target) {
        return;
    }
    target[0] = '\0';
    if (!source || source[0] == '\0') {
        return;
    }
    snprintf(target, KEK_TRACE_PATH_CAPACITY, "%s", source);
}

void kek_trace_init(struct KekRuntime* runtime) {
    if (!runtime) {
        return;
    }

    memset(&runtime->trace, 0, sizeof(runtime->trace));
    const char* runtime_path = getenv("KEK_TRACE_RUNTIME_CSV");
    const char* hooks_path = getenv("KEK_TRACE_HOOKS_CSV");
    const char* subscriber_timing = getenv("KEK_TRACE_SUBSCRIBER_TIMING");
    trace_copy_path(runtime->trace.runtime_csv_path, runtime_path);
    trace_copy_path(runtime->trace.hooks_csv_path, hooks_path);
    runtime->trace.runtime_csv_enabled =
        runtime->trace.runtime_csv_path[0] != '\0';
    runtime->trace.hooks_csv_enabled = runtime->trace.hooks_csv_path[0] != '\0';
    runtime->trace.enabled = runtime->trace.runtime_csv_enabled ||
                             runtime->trace.hooks_csv_enabled;
    runtime->trace.subscriber_timing_enabled =
        subscriber_timing && subscriber_timing[0] != '\0' &&
        strcmp(subscriber_timing, "0") != 0;
    if (runtime->trace.runtime_csv_enabled) {
        for (size_t i = 0; i < KEK_TRACE_RUNTIME_METRIC_ID_COUNT; i++) {
            runtime->trace.runtime_metrics[i].name = trace_runtime_metric_names[i];
        }
        runtime->trace.runtime_metric_count = KEK_TRACE_RUNTIME_METRIC_ID_COUNT;
    }
}

int kek_trace_enabled(const struct KekRuntime* runtime) {
    return runtime && runtime->trace.enabled;
}

static KekTraceRuntimeMetric* trace_runtime_metric(struct KekRuntime* runtime,
                                                   const char* name) {
    if (!runtime || !name || !runtime->trace.runtime_csv_enabled) {
        return NULL;
    }

    for (size_t i = 0; i < runtime->trace.runtime_metric_count; i++) {
        KekTraceRuntimeMetric* metric = &runtime->trace.runtime_metrics[i];
        if (metric->name == name ||
            (metric->name && strcmp(metric->name, name) == 0)) {
            return metric;
        }
    }

    if (runtime->trace.runtime_metric_count >= KEK_TRACE_MAX_RUNTIME_METRICS) {
        return NULL;
    }

    KekTraceRuntimeMetric* metric =
        &runtime->trace.runtime_metrics[runtime->trace.runtime_metric_count++];
    memset(metric, 0, sizeof(*metric));
    metric->name = name;
    return metric;
}

void kek_trace_record_runtime(struct KekRuntime* runtime, const char* name,
                              uint64_t duration_ns) {
    KekTraceRuntimeMetric* metric = trace_runtime_metric(runtime, name);
    if (!metric) {
        return;
    }

    metric->count++;
    metric->total_ns += duration_ns;
    if (metric->count == 1 || duration_ns < metric->min_ns) {
        metric->min_ns = duration_ns;
    }
    if (duration_ns > metric->max_ns) {
        metric->max_ns = duration_ns;
    }
}

void kek_trace_record_runtime_metric(struct KekRuntime* runtime,
                                     KekTraceRuntimeMetricId metric_id,
                                     uint64_t duration_ns) {
    if (!runtime || !runtime->trace.runtime_csv_enabled ||
        metric_id < 0 || metric_id >= KEK_TRACE_RUNTIME_METRIC_ID_COUNT) {
        return;
    }

    KekTraceRuntimeMetric* metric = &runtime->trace.runtime_metrics[metric_id];
    metric->count++;
    metric->total_ns += duration_ns;
    if (metric->count == 1 || duration_ns < metric->min_ns) {
        metric->min_ns = duration_ns;
    }
    if (duration_ns > metric->max_ns) {
        metric->max_ns = duration_ns;
    }
}

void kek_trace_count_runtime_metric(struct KekRuntime* runtime,
                                    KekTraceRuntimeMetricId metric_id) {
    if (!runtime || !runtime->trace.runtime_csv_enabled ||
        metric_id < 0 || metric_id >= KEK_TRACE_RUNTIME_METRIC_ID_COUNT) {
        return;
    }

    runtime->trace.runtime_metrics[metric_id].count++;
}

static KekTraceHookMetric* trace_hook_metric(
    struct KekRuntime* runtime, const struct KekHookDescriptor* descriptor) {
    if (!runtime || !descriptor || !runtime->trace.hooks_csv_enabled) {
        return NULL;
    }

    const char* name = descriptor->name ? descriptor->name : "";
    for (size_t i = 0; i < runtime->trace.hook_metric_count; i++) {
        KekTraceHookMetric* metric = &runtime->trace.hook_metrics[i];
        if (metric->event_type == (int)descriptor->event_type &&
            metric->state_type_id == descriptor->state_type_id &&
            metric->state_slot_id == descriptor->state_slot_id &&
            (metric->hook_name == name ||
             (metric->hook_name && strcmp(metric->hook_name, name) == 0))) {
            return metric;
        }
    }

    if (runtime->trace.hook_metric_count >= KEK_TRACE_MAX_HOOK_METRICS) {
        return NULL;
    }

    KekTraceHookMetric* metric =
        &runtime->trace.hook_metrics[runtime->trace.hook_metric_count++];
    memset(metric, 0, sizeof(*metric));
    metric->hook_name = name;
    metric->event_type = descriptor->event_type;
    metric->state_type_id = descriptor->state_type_id;
    metric->state_slot_id = descriptor->state_slot_id;
    return metric;
}

void kek_trace_record_hook(struct KekRuntime* runtime,
                           const struct KekHookDescriptor* descriptor,
                           const struct KekEvent* event,
                           uint64_t wait_ns,
                           uint64_t run_ns,
                           int ok) {
    (void)event;
    KekTraceHookMetric* metric = trace_hook_metric(runtime, descriptor);
    if (!metric) {
        return;
    }

    metric->call_count++;
    if (ok) {
        metric->success_count++;
    } else {
        metric->failure_count++;
    }
    metric->total_wait_ns += wait_ns;
    if (metric->call_count == 1 || wait_ns < metric->min_wait_ns) {
        metric->min_wait_ns = wait_ns;
    }
    if (wait_ns > metric->max_wait_ns) {
        metric->max_wait_ns = wait_ns;
    }
    metric->total_run_ns += run_ns;
    if (metric->call_count == 1 || run_ns < metric->min_run_ns) {
        metric->min_run_ns = run_ns;
    }
    if (run_ns > metric->max_run_ns) {
        metric->max_run_ns = run_ns;
    }
}

void* kek_trace_malloc(struct KekRuntime* runtime, size_t size) {
    uint64_t start = kek_trace_enabled(runtime) ? kek_trace_now_ns() : 0;
    void* ptr = malloc(size);
    if (kek_trace_enabled(runtime)) {
        uint64_t end = kek_trace_now_ns();
        kek_trace_record_runtime_metric(runtime, KEK_TRACE_METRIC_RUNTIME_MALLOC,
                                        end - start);
        if (ptr) {
            runtime->trace.allocation_count++;
            runtime->trace.current_bytes += size;
            runtime->trace.total_allocated_bytes += size;
            if (runtime->trace.current_bytes > runtime->trace.peak_bytes) {
                runtime->trace.peak_bytes = runtime->trace.current_bytes;
            }
        }
    }
    return ptr;
}

void kek_trace_free(struct KekRuntime* runtime, void* ptr, size_t size) {
    if (!ptr) {
        return;
    }

    uint64_t start = kek_trace_enabled(runtime) ? kek_trace_now_ns() : 0;
    free(ptr);
    if (kek_trace_enabled(runtime)) {
        uint64_t end = kek_trace_now_ns();
        kek_trace_record_runtime_metric(runtime, KEK_TRACE_METRIC_RUNTIME_FREE,
                                        end - start);
        runtime->trace.free_count++;
        runtime->trace.total_freed_bytes += size;
        if (runtime->trace.current_bytes >= size) {
            runtime->trace.current_bytes -= size;
        } else {
            runtime->trace.current_bytes = 0;
        }
    }
}

static void trace_write_runtime_csv(struct KekRuntime* runtime) {
    if (!runtime || !runtime->trace.runtime_csv_enabled) {
        return;
    }

    FILE* file = fopen(runtime->trace.runtime_csv_path, "w");
    if (!file) {
        return;
    }

    fprintf(file,
            "metric,count,total_ns,avg_ns,min_ns,max_ns,current_bytes,"
            "peak_bytes,total_allocated_bytes,total_freed_bytes\n");
    for (size_t i = 0; i < runtime->trace.runtime_metric_count; i++) {
        const KekTraceRuntimeMetric* metric = &runtime->trace.runtime_metrics[i];
        uint64_t avg = metric->count ? metric->total_ns / metric->count : 0;
        fprintf(file,
                "%s,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n",
                metric->name ? metric->name : "",
                (unsigned long long)metric->count,
                (unsigned long long)metric->total_ns,
                (unsigned long long)avg,
                (unsigned long long)metric->min_ns,
                (unsigned long long)metric->max_ns,
                (unsigned long long)runtime->trace.current_bytes,
                (unsigned long long)runtime->trace.peak_bytes,
                (unsigned long long)runtime->trace.total_allocated_bytes,
                (unsigned long long)runtime->trace.total_freed_bytes);
    }
    fprintf(file, "runtime_memory,%llu,0,0,0,0,%llu,%llu,%llu,%llu\n",
            (unsigned long long)runtime->trace.allocation_count,
            (unsigned long long)runtime->trace.current_bytes,
            (unsigned long long)runtime->trace.peak_bytes,
            (unsigned long long)runtime->trace.total_allocated_bytes,
            (unsigned long long)runtime->trace.total_freed_bytes);
    fclose(file);
}

static void trace_write_csv_field(FILE* file, const char* value) {
    fputc('"', file);
    if (value) {
        for (const char* cursor = value; *cursor; cursor++) {
            if (*cursor == '"') {
                fputc('"', file);
            }
            fputc(*cursor, file);
        }
    }
    fputc('"', file);
}

static void trace_write_hooks_csv(struct KekRuntime* runtime) {
    if (!runtime || !runtime->trace.hooks_csv_enabled) {
        return;
    }

    FILE* file = fopen(runtime->trace.hooks_csv_path, "w");
    if (!file) {
        return;
    }

    fprintf(file,
            "hook,event_type,state_type_id,state_slot_id,call_count,"
            "success_count,failure_count,total_wait_ns,avg_wait_ns,"
            "min_wait_ns,max_wait_ns,total_run_ns,avg_run_ns,min_run_ns,"
            "max_run_ns\n");
    for (size_t i = 0; i < runtime->trace.hook_metric_count; i++) {
        const KekTraceHookMetric* metric = &runtime->trace.hook_metrics[i];
        uint64_t avg_wait =
            metric->call_count ? metric->total_wait_ns / metric->call_count : 0;
        uint64_t avg_run =
            metric->call_count ? metric->total_run_ns / metric->call_count : 0;
        trace_write_csv_field(file, metric->hook_name);
        fprintf(file,
                ",%d,%zu,%zu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,"
                "%llu,%llu,%llu,%llu\n",
                metric->event_type,
                metric->state_type_id,
                metric->state_slot_id,
                (unsigned long long)metric->call_count,
                (unsigned long long)metric->success_count,
                (unsigned long long)metric->failure_count,
                (unsigned long long)metric->total_wait_ns,
                (unsigned long long)avg_wait,
                (unsigned long long)metric->min_wait_ns,
                (unsigned long long)metric->max_wait_ns,
                (unsigned long long)metric->total_run_ns,
                (unsigned long long)avg_run,
                (unsigned long long)metric->min_run_ns,
                (unsigned long long)metric->max_run_ns);
    }
    fclose(file);
}

void kek_trace_destroy(struct KekRuntime* runtime) {
    if (!kek_trace_enabled(runtime)) {
        return;
    }

    trace_write_runtime_csv(runtime);
    trace_write_hooks_csv(runtime);
    memset(&runtime->trace, 0, sizeof(runtime->trace));
}
