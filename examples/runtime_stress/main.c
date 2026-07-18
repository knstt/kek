#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "runtime/event.h"
#include "runtime/runtime.h"
#include "runtime/standard_io.h"
#include "runtime/stream.h"
#include "runtime/timer.h"
#include "runtime_stress_support.h"

#define STRESS_CHECK(label, condition)                 \
    do {                                               \
        if (!(condition)) {                            \
            fprintf(stderr, "runtime stress: %s\n", label); \
            goto done;                                 \
        }                                              \
    } while (0)

enum {
    STRESS_AGENT_COUNT = 96,
    STRESS_PACKET_COUNT = 20,
    STRESS_CYCLES = 120,
    STRESS_BATCH_SIZE = 8
};

typedef struct AgentStep {
    float dx;
    float dy;
    double energy_delta;
    uint64_t flags;
} AgentStep;

typedef struct PacketStep {
    uint64_t seq;
    uint32_t bytes;
    uint64_t checksum;
} PacketStep;

typedef struct ClockStep {
    uint64_t tick;
    int32_t phase;
} ClockStep;

typedef struct AuditBump {
    uint64_t batch_delta;
    uint64_t error_delta;
    uint64_t eof_delta;
} AuditBump;

static void update_agent_step(void* draft, void* context) {
    Agent* agent = (Agent*)draft;
    const AgentStep* step = (const AgentStep*)context;
    if (!agent || !step) {
        return;
    }
    agent->x += step->dx;
    agent->y += step->dy;
    agent->energy += step->energy_delta;
    agent->flags ^= step->flags;
    if (agent->energy < 5.0) {
        agent->energy = 5.0;
    }
    if (agent->energy > 950.0) {
        agent->energy = 950.0;
    }
}

static void update_packet_step(void* draft, void* context) {
    Packet* packet = (Packet*)draft;
    const PacketStep* step = (const PacketStep*)context;
    if (!packet || !step) {
        return;
    }
    packet->seq = step->seq;
    packet->bytes = step->bytes;
    packet->checksum = step->checksum;
    for (size_t i = 0; i < 16; i++) {
        packet->samples[i] = (int32_t)((step->seq + i) % 997u);
    }
}

static void update_clock_step(void* draft, void* context) {
    SimulationClock* clock = (SimulationClock*)draft;
    const ClockStep* step = (const ClockStep*)context;
    if (!clock || !step) {
        return;
    }
    clock->tick = step->tick;
    clock->phase = step->phase;
    clock->dt = 0.016f;
    clock->enabled = true;
}

static void update_audit_bump(void* draft, void* context) {
    AuditLog* audit = (AuditLog*)draft;
    const AuditBump* bump = (const AuditBump*)context;
    if (!audit || !bump) {
        return;
    }
    audit->last_event++;
    audit->batch_count += bump->batch_delta;
    audit->error_count += bump->error_delta;
    audit->eof_count += bump->eof_delta;
}

static int count_event(const KekEvent* event, void* context) {
    RuntimeStressApp* app = (RuntimeStressApp*)context;
    if (!event || !app || event->type < 0 || event->type >= KEK_EVENT_TYPE_COUNT) {
        return 0;
    }
    app->subscriber_events[event->type]++;
    if (event->type == KEK_EVENT_STATE_BATCH_CHANGED && app->generated) {
        AuditBump bump = {1, 0, 0};
        return runtime_stress_state_update_audit(
            app->generated, update_audit_bump, &bump,
            KEK_STATE_TYPE_AUDIT_LOG_FIELD_LAST_EVENT |
                KEK_STATE_TYPE_AUDIT_LOG_FIELD_BATCH_COUNT);
    }
    return 1;
}

static int stream_event_handler(const KekEvent* event, void* context) {
    RuntimeStressApp* app = (RuntimeStressApp*)context;
    if (!event || !app || !app->generated) {
        return 0;
    }

    if (event->type == KEK_EVENT_STREAM_DATA) {
        return app->input_bridge &&
               kek_standard_text_bridge_append(app->input_bridge, event->data,
                                               event->data_len);
    }

    if (event->type == KEK_EVENT_STREAM_EOF || event->type == KEK_EVENT_STREAM_ERROR) {
        AuditBump bump = {0, event->type == KEK_EVENT_STREAM_ERROR ? 1 : 0,
                          event->type == KEK_EVENT_STREAM_EOF ? 1 : 0};
        return runtime_stress_state_update_audit(
            app->generated, update_audit_bump, &bump,
            KEK_STATE_TYPE_AUDIT_LOG_FIELD_LAST_EVENT |
                (event->type == KEK_EVENT_STREAM_ERROR
                     ? KEK_STATE_TYPE_AUDIT_LOG_FIELD_ERROR_COUNT
                     : KEK_STATE_TYPE_AUDIT_LOG_FIELD_EOF_COUNT));
    }

    return 1;
}

static int subscribe_runtime_events(Runtime_stress_stateRuntime* stress,
                                    RuntimeStressApp* app) {
    KekEventDispatcher* events =
        kek_runtime_events(runtime_stress_state_get_runtime(stress));
    for (int type = 0; type < KEK_EVENT_TYPE_COUNT; type++) {
        if (!kek_event_subscribe(events, (KekEventType)type, count_event, app)) {
            return 0;
        }
    }
    if (!kek_event_subscribe(events, KEK_EVENT_STATE_CHANGED, count_event, app)) {
        return 0;
    }
    if (!kek_event_unsubscribe(events, KEK_EVENT_STREAM_ERROR, count_event, app)) {
        return 0;
    }
    if (!kek_event_subscribe(events, KEK_EVENT_STREAM_ERROR, count_event, app)) {
        return 0;
    }
    if (!kek_event_subscribe(events, KEK_EVENT_STREAM_DATA, stream_event_handler,
                             app) ||
        !kek_event_subscribe(events, KEK_EVENT_STREAM_EOF, stream_event_handler,
                             app) ||
        !kek_event_subscribe(events, KEK_EVENT_STREAM_ERROR, stream_event_handler,
                             app)) {
        return 0;
    }
    return 1;
}

static int create_dynamic_slots(Runtime_stress_stateRuntime* stress,
                                size_t agents[STRESS_AGENT_COUNT],
                                size_t packets[STRESS_PACKET_COUNT]) {
    for (size_t i = 0; i < STRESS_AGENT_COUNT; i++) {
        Agent agent = (i % 3 == 0) ? Agent_scout() : Agent_guard();
        agent.id = (uint32_t)(1000u + i);
        agent.x = (float)((int)i - 48);
        agent.y = (float)((int)(i % 17) - 8);
        agent.energy = 50.0 + (double)(i % 40);
        agents[i] = runtime_stress_state_create_agent_with(stress, &agent);
        if (agents[i] == KEK_STATE_INVALID_ID) {
            return 0;
        }
    }

    for (size_t i = 0; i < STRESS_PACKET_COUNT; i++) {
        Packet packet = Packet_telemetry();
        packet.seq = i + 10;
        packet.bytes = (uint32_t)(32u + i);
        packet.checksum = 0xC0FFEEu + i;
        packets[i] = runtime_stress_state_create_packet_with(stress, &packet);
        if (packets[i] == KEK_STATE_INVALID_ID) {
            return 0;
        }
    }
    return 1;
}

static int run_batched_cycles(Runtime_stress_stateRuntime* stress,
                              const size_t agents[STRESS_AGENT_COUNT],
                              const size_t packets[STRESS_PACKET_COUNT]) {
    for (size_t cycle = 0; cycle < STRESS_CYCLES; cycle++) {
        Runtime_stress_stateUpdateItem updates[STRESS_BATCH_SIZE];
        AgentStep agent_steps[4];
        PacketStep packet_steps[2];
        ClockStep clock_step = {cycle + 1, (int32_t)(cycle % 12u)};

        updates[0] = runtime_stress_state_clock_update_item(
            stress, update_clock_step, &clock_step,
            KEK_STATE_TYPE_SIMULATION_CLOCK_FIELD_TICK |
                KEK_STATE_TYPE_SIMULATION_CLOCK_FIELD_PHASE);

        for (size_t i = 0; i < 4; i++) {
            size_t agent_index = (cycle * 7u + i * 13u) % STRESS_AGENT_COUNT;
            agent_steps[i].dx = (float)((int)(cycle % 5u) - 2) * 0.25f;
            agent_steps[i].dy = (float)((int)(i % 3u) - 1) * 0.5f;
            agent_steps[i].energy_delta = (i % 2u == 0) ? -0.5 : 0.25;
            agent_steps[i].flags = 1ull << (i % 8u);
            updates[i + 1] = runtime_stress_state_agent_slot_update_item(
                agents[agent_index], update_agent_step, &agent_steps[i],
                KEK_STATE_TYPE_AGENT_FIELD_X | KEK_STATE_TYPE_AGENT_FIELD_Y |
                    KEK_STATE_TYPE_AGENT_FIELD_ENERGY |
                    KEK_STATE_TYPE_AGENT_FIELD_FLAGS);
        }

        for (size_t i = 0; i < 2; i++) {
            size_t packet_index = (cycle * 5u + i * 3u) % STRESS_PACKET_COUNT;
            packet_steps[i].seq = cycle * 100u + i;
            packet_steps[i].bytes = (uint32_t)(64u + ((cycle + i) % 256u));
            packet_steps[i].checksum = packet_steps[i].seq ^ packet_steps[i].bytes;
            updates[i + 5] = runtime_stress_state_packet_slot_update_item(
                packets[packet_index], update_packet_step, &packet_steps[i],
                KEK_STATE_TYPE_PACKET_FIELD_SEQ |
                    KEK_STATE_TYPE_PACKET_FIELD_BYTES |
                    KEK_STATE_TYPE_PACKET_FIELD_CHECKSUM |
                    KEK_STATE_TYPE_PACKET_FIELD_SAMPLES);
        }

        PacketStep main_packet_step = {cycle + 5000u, 128u,
                                       (cycle + 5000u) ^ 128u};
        updates[7] = runtime_stress_state_main_packet_update_item(
            stress, update_packet_step, &main_packet_step,
            KEK_STATE_TYPE_PACKET_FIELD_SEQ | KEK_STATE_TYPE_PACKET_FIELD_BYTES |
                KEK_STATE_TYPE_PACKET_FIELD_CHECKSUM);

        if (!runtime_stress_state_update_many(stress, updates, STRESS_BATCH_SIZE) ||
            !runtime_stress_state_dispatch(stress)) {
            return 0;
        }

        if (cycle % 15u == 0) {
            char text[64];
            int len = snprintf(text, sizeof(text), "cycle-%03zu", cycle);
            if (len < 0 ||
                !runtime_stress_state_control_panel_set_label(
                    runtime_stress_state_get_store(stress),
                    runtime_stress_state_control_slot_id(stress), text,
                    (size_t)len) ||
                !runtime_stress_state_packet_set_payload(
                    runtime_stress_state_get_store(stress),
                    runtime_stress_state_main_packet_slot_id(stress), text,
                    (size_t)len) ||
                !runtime_stress_state_dispatch(stress)) {
                return 0;
            }
        }
    }
    return 1;
}

static int exercise_rollbacks(Runtime_stress_stateRuntime* stress,
                              RuntimeStressApp* app) {
    const Agent* before =
        runtime_stress_state_primary_agent_current_const(stress);
    if (!before) {
        return 0;
    }
    double original_energy = before->energy;
    if (runtime_stress_state_set_primary_agent_energy(stress, 2000.0)) {
        return 0;
    }
    const Agent* after =
        runtime_stress_state_primary_agent_current_const(stress);
    if (!after || after->energy != original_energy) {
        return 0;
    }

    const AuditLog* audit_before =
        runtime_stress_state_audit_current_const(stress);
    if (!audit_before) {
        return 0;
    }
    uint64_t original_audit_last_event = audit_before->last_event;
    app->force_clock_phase_failure = 1;
    if (!runtime_stress_state_set_clock_phase(stress, 13) ||
        runtime_stress_state_dispatch(stress)) {
        return 0;
    }
    app->force_clock_phase_failure = 0;
    const AuditLog* audit_after =
        runtime_stress_state_audit_current_const(stress);
    if (!audit_after || audit_after->last_event != original_audit_last_event ||
        app->hook_failures == 0) {
        return 0;
    }

    app->check_forbidden_write = 1;
    if (!runtime_stress_state_control_panel_set_label(
            runtime_stress_state_get_store(stress),
            runtime_stress_state_control_slot_id(stress), "forbidden-check",
            strlen("forbidden-check")) ||
        !runtime_stress_state_dispatch(stress)) {
        return 0;
    }
    app->check_forbidden_write = 0;
    return app->forbidden_write_checks > 0;
}

static int exercise_delete_and_reuse(Runtime_stress_stateRuntime* stress,
                                     size_t agents[STRESS_AGENT_COUNT],
                                     size_t packets[STRESS_PACKET_COUNT]) {
    size_t old_agent_slot = agents[3];
    size_t old_packet_slot = packets[2];
    if (!runtime_stress_state_delete_agent(stress, old_agent_slot) ||
        !runtime_stress_state_delete_packet(stress, old_packet_slot) ||
        !runtime_stress_state_dispatch(stress)) {
        return 0;
    }

    Agent replacement_agent = Agent_scout();
    replacement_agent.id = 9001;
    Packet replacement_packet = Packet_telemetry();
    replacement_packet.seq = 9002;
    agents[3] = runtime_stress_state_create_agent_with(stress, &replacement_agent);
    packets[2] =
        runtime_stress_state_create_packet_with(stress, &replacement_packet);
    return agents[3] == old_agent_slot && packets[2] == old_packet_slot &&
           runtime_stress_state_dispatch(stress);
}

static int exercise_streams_and_timers(Runtime_stress_stateRuntime* stress,
                                       RuntimeStressApp* app) {
    int input_pipe[2];
    int output_pipe[2];
    char input_buffer[4096];
    char output_buffer[4096];
    char pipe_buffer[128];
    KekStandardTextBridge input_bridge;
    KekStandardTextBridge output_bridge;

    if (pipe(input_pipe) != 0 || pipe(output_pipe) != 0) {
        return 0;
    }

    KekRuntime* runtime = runtime_stress_state_get_runtime(stress);
    KekStateStore* store = runtime_stress_state_get_store(stress);
    int read_stream_id =
        kek_runtime_register_stream(runtime, input_pipe[0], KEK_STREAM_READ, 1);
    int write_stream_id =
        kek_runtime_register_stream(runtime, output_pipe[1], KEK_STREAM_WRITE, 1);
    int fast_timer_id = kek_runtime_register_timer(
        runtime, store, runtime_stress_state_fast_timer_slot_id(stress), 1);
    int slow_timer_id = kek_runtime_register_timer(
        runtime, store, runtime_stress_state_slow_timer_slot_id(stress), 2);
    if (read_stream_id < 0 || write_stream_id < 0 || fast_timer_id < 0 ||
        slow_timer_id < 0) {
        close(input_pipe[1]);
        close(output_pipe[0]);
        return 0;
    }

    kek_standard_text_bridge_init(
        &input_bridge, store, runtime_stress_state_standard_input_slot_id(stress),
        input_buffer, sizeof(input_buffer), runtime_stress_state_standard_input_set_input);
    kek_standard_text_bridge_init(
        &output_bridge, store, runtime_stress_state_standard_output_slot_id(stress),
        output_buffer, sizeof(output_buffer),
        runtime_stress_state_standard_output_set_output);
    app->input_bridge = &input_bridge;

    KekStream* output_stream = kek_runtime_get_stream(runtime, (size_t)write_stream_id);
    const char* output_text = "runtime-stress-output";
    if (kek_standard_output_write(output_stream, &output_bridge, output_text,
                                  strlen(output_text)) != strlen(output_text)) {
        close(input_pipe[1]);
        close(output_pipe[0]);
        return 0;
    }

    const char* input_text = "runtime-stress-input";
    if (write(input_pipe[1], input_text, strlen(input_text)) < 0) {
        close(input_pipe[1]);
        close(output_pipe[0]);
        return 0;
    }
    close(input_pipe[1]);

    KekEvent manual_error;
    memset(&manual_error, 0, sizeof(manual_error));
    manual_error.type = KEK_EVENT_STREAM_ERROR;
    manual_error.error_code = EIO;
    if (!kek_event_publish(kek_runtime_events(runtime), &manual_error)) {
        close(output_pipe[0]);
        return 0;
    }

    if (kek_runtime_enable_raw_mode(runtime, -1) != 0 ||
        kek_runtime_run(runtime) != 0) {
        close(output_pipe[0]);
        return 0;
    }

    ssize_t bytes = read(output_pipe[0], pipe_buffer, sizeof(pipe_buffer) - 1);
    close(output_pipe[0]);
    app->input_bridge = NULL;
    if (bytes < 0) {
        return 0;
    }
    pipe_buffer[bytes] = '\0';

    const StandardInput* input =
        runtime_stress_state_standard_input_current_const(stress);
    const StandardOutput* output =
        runtime_stress_state_standard_output_current_const(stress);
    const Timer* fast_timer = runtime_stress_state_fast_timer_current_const(stress);
    return input && output && fast_timer && strcmp(input->input.data, input_text) == 0 &&
           strcmp(output->output.data, output_text) == 0 &&
           strcmp(pipe_buffer, output_text) == 0 && fast_timer->tick >= 3;
}

static int verify_results(const Runtime_stress_stateRuntime* stress,
                          const RuntimeStressApp* app) {
    const Telemetry* telemetry =
        runtime_stress_state_telemetry_current_const(stress);
    const AuditLog* audit = runtime_stress_state_audit_current_const(stress);
    if (!telemetry || !audit || !app) {
        return 0;
    }
    return telemetry->total_events > 1000 && telemetry->hook_hits > 1000 &&
           telemetry->timer_ticks >= 3 && telemetry->stream_bytes > 0 &&
           audit->batch_count > 0 && audit->deleted_count >= 2 &&
           audit->eof_count > 0 && audit->error_count > 0 &&
           app->subscriber_events[KEK_EVENT_STATE_CHANGED] > 1000 &&
           app->subscriber_events[KEK_EVENT_STATE_BATCH_CHANGED] >= STRESS_CYCLES &&
           app->subscriber_events[KEK_EVENT_STREAM_DATA] > 0 &&
           app->subscriber_events[KEK_EVENT_STREAM_EOF] > 0 &&
           app->subscriber_events[KEK_EVENT_STREAM_ERROR] > 0 &&
           app->hook_calls > 1000;
}

int main(void) {
    Runtime_stress_stateRuntime stress;
    RuntimeStressApp app;
    size_t agents[STRESS_AGENT_COUNT];
    size_t packets[STRESS_PACKET_COUNT];
    int ok = 0;

    memset(&app, 0, sizeof(app));
    memset(agents, 0xff, sizeof(agents));
    memset(packets, 0xff, sizeof(packets));

    if (!runtime_stress_state_runtime_init(&stress, &app)) {
        return 1;
    }
    app.generated = &stress;

    STRESS_CHECK("subscribe runtime events", subscribe_runtime_events(&stress, &app));
    STRESS_CHECK("dispatch declared creates", runtime_stress_state_dispatch(&stress));
    STRESS_CHECK("create dynamic slots", create_dynamic_slots(&stress, agents, packets));
    STRESS_CHECK("dispatch dynamic creates", runtime_stress_state_dispatch(&stress));
    STRESS_CHECK("agent count",
                 runtime_stress_state_count_agent(&stress) == STRESS_AGENT_COUNT + 2);
    STRESS_CHECK("packet count",
                 runtime_stress_state_count_packet(&stress) == STRESS_PACKET_COUNT + 1);
    STRESS_CHECK("active agent count",
                 runtime_stress_state_count_active_agent(&stress) > 0);
    STRESS_CHECK("rollback paths", exercise_rollbacks(&stress, &app));
    STRESS_CHECK("batched cycles", run_batched_cycles(&stress, agents, packets));
    STRESS_CHECK("delete and slot reuse",
                 exercise_delete_and_reuse(&stress, agents, packets));
    STRESS_CHECK("streams and timers", exercise_streams_and_timers(&stress, &app));
    STRESS_CHECK("final results", verify_results(&stress, &app));
    ok = 1;

done:
    runtime_stress_state_runtime_destroy(&stress);
    if (ok) {
        printf("runtime stress passed: hooks=%llu state_changed=%llu batches=%llu\n",
               (unsigned long long)app.hook_calls,
               (unsigned long long)app.subscriber_events[KEK_EVENT_STATE_CHANGED],
               (unsigned long long)app.subscriber_events[KEK_EVENT_STATE_BATCH_CHANGED]);
    }
    return ok ? 0 : 1;
}
