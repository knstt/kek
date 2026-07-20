#include "runtime_stress_support.h"

int OnBenchmarkMergeRight(KekHookContext* context) {
    RuntimeStressApp* app = context ? (RuntimeStressApp*)context->app_context : 0;
    const Runtime_stress_stateStateSlots* slots =
        app ? runtime_stress_state_get_slots_const(app->generated) : 0;
    OnBenchmarkMergeRightAccess access =
        on_benchmark_merge_right_access(context, slots);
    const WriteBenchmarkTrigger* trigger =
        on_benchmark_merge_right_read_benchmark_trigger(&access);
    if (!trigger) {
        return 0;
    }

    uint64_t value = trigger->merge_tick * 5u + 2u;
    runtime_stress_note_write_benchmark(context, 1,
                                        value ^ 0x8171615141312111ull);
    return on_benchmark_merge_right_set_benchmark_merge_right_value(&access,
                                                                    value);
}
