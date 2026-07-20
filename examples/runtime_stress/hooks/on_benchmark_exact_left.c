#include "runtime_stress_support.h"

typedef struct BenchmarkTargetPatch {
    uint64_t tick;
    uint64_t salt;
} BenchmarkTargetPatch;

static void update_exact_target(void* draft, void* context) {
    WriteBenchmarkTarget* target = (WriteBenchmarkTarget*)draft;
    const BenchmarkTargetPatch* patch = (const BenchmarkTargetPatch*)context;
    if (!target || !patch) {
        return;
    }

    target->exact_hits++;
    target->checksum ^= patch->tick + patch->salt + target->exact_hits;
}

int OnBenchmarkExactLeft(KekHookContext* context) {
    RuntimeStressApp* app = context ? (RuntimeStressApp*)context->app_context : 0;
    const Runtime_stress_stateStateSlots* slots =
        app ? runtime_stress_state_get_slots_const(app->generated) : 0;
    OnBenchmarkExactLeftAccess access =
        on_benchmark_exact_left_access(context, slots);
    const WriteBenchmarkTrigger* trigger =
        on_benchmark_exact_left_read_benchmark_trigger(&access);
    if (!trigger) {
        return 0;
    }

    BenchmarkTargetPatch patch = {trigger->exact_tick, 0xB100C1E1ull};
    runtime_stress_note_write_benchmark(context, 0, patch.tick ^ patch.salt);
    return on_benchmark_exact_left_update_benchmark_left(
        &access, update_exact_target, &patch,
        KEK_STATE_TYPE_WRITE_BENCHMARK_TARGET_FIELD_EXACT_HITS |
            KEK_STATE_TYPE_WRITE_BENCHMARK_TARGET_FIELD_CHECKSUM);
}
