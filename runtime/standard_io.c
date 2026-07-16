#include "standard_io.h"

#include <string.h>

void kek_standard_text_bridge_init(KekStandardTextBridge* bridge,
                                   KekStateStore* store,
                                   size_t slot_id,
                                   char* buffer,
                                   size_t capacity,
                                   KekStandardTextSetter set_text) {
    if (!bridge) {
        return;
    }
    bridge->store = store;
    bridge->slot_id = slot_id;
    bridge->buffer = buffer;
    bridge->capacity = capacity;
    bridge->len = 0;
    bridge->set_text = set_text;
    if (buffer && capacity > 0) {
        buffer[0] = '\0';
    }
}

int kek_standard_text_bridge_append(KekStandardTextBridge* bridge,
                                    const char* data, size_t len) {
    if (!bridge || !bridge->store || !bridge->buffer || bridge->capacity == 0 ||
        bridge->len >= bridge->capacity || !bridge->set_text ||
        (!data && len > 0)) {
        return 0;
    }
    size_t available = bridge->capacity - bridge->len - 1;
    size_t to_copy = len < available ? len : available;
    if (to_copy > 0) {
        memcpy(bridge->buffer + bridge->len, data, to_copy);
        bridge->len += to_copy;
        bridge->buffer[bridge->len] = '\0';
    }
    return bridge->set_text(bridge->store, bridge->slot_id, bridge->buffer, bridge->len);
}

int kek_standard_text_bridge_track_output(KekStandardTextBridge* bridge,
                                          const char* data, size_t len) {
    if (!bridge || !bridge->store || !bridge->buffer || bridge->capacity == 0 ||
        bridge->len >= bridge->capacity || !bridge->set_text ||
        (!data && len > 0)) {
        return 0;
    }
    size_t available = bridge->capacity - bridge->len - 1;
    if (available == 0) {
        bridge->len = 0;
        bridge->buffer[0] = '\0';
        available = bridge->capacity - 1;
    }
    size_t to_copy = len < available ? len : available;
    if (to_copy > 0) {
        memcpy(bridge->buffer + bridge->len, data, to_copy);
        bridge->len += to_copy;
        bridge->buffer[bridge->len] = '\0';
    }
    return bridge->set_text(bridge->store, bridge->slot_id, bridge->buffer, bridge->len);
}

size_t kek_standard_output_write(KekStream* stream,
                                 KekStandardTextBridge* bridge,
                                 const char* data, size_t len) {
    if (!stream || !data) {
        return 0;
    }
    kek_stream_flush(stream);
    size_t written = kek_stream_write_raw(stream, data, len);
    if (written != len) {
        kek_stream_flush(stream);
        written += kek_stream_write_raw(stream, data + written, len - written);
    }
    if (bridge) {
        kek_standard_text_bridge_track_output(bridge, data, written);
    }
    return written;
}
