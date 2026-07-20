#include "runtime_stress_support.h"

int OnBenchmarkMergeLeft(KekHookContext* context) {
    RuntimeStressApp* app = context ? (RuntimeStressApp*)context->app_context : 0;
    const Runtime_stress_stateStateSlots* slots =
        app ? runtime_stress_state_get_slots_const(app->generated) : 0;
    OnBenchmarkMergeLeftAccess access =
        on_benchmark_merge_left_access(context, slots);
    const WriteBenchmarkTrigger* trigger =
        on_benchmark_merge_left_read_benchmark_trigger(&access);
    if (!trigger) {
        return 0;
    }

    uint64_t value = trigger->merge_tick * 3u + 1u;
    runtime_stress_note_write_benchmark(context, 1,
                                        value ^ 0xF1E1D1C1B1A19181ull);
    return on_benchmark_merge_left_set_benchmark_merge_left_value(&access,
                                                                  value);
}
