#ifndef KEK_RUNTIME_STANDARD_IO_H
#define KEK_RUNTIME_STANDARD_IO_H

#include <stddef.h>

#include "state_storage.h"
#include "stream.h"

typedef int (*KekStandardTextSetter)(KekStateStore* store, size_t slot_id,
                                     const char* data, size_t len);

typedef struct KekStandardTextBridge {
    KekStateStore* store;
    size_t slot_id;
    char* buffer;
    size_t capacity;
    size_t len;
    KekStandardTextSetter set_text;
} KekStandardTextBridge;

void kek_standard_text_bridge_init(KekStandardTextBridge* bridge,
                                   KekStateStore* store,
                                   size_t slot_id,
                                   char* buffer,
                                   size_t capacity,
                                   KekStandardTextSetter set_text);
int kek_standard_text_bridge_append(KekStandardTextBridge* bridge,
                                    const char* data, size_t len);
int kek_standard_text_bridge_track_output(KekStandardTextBridge* bridge,
                                          const char* data, size_t len);
size_t kek_standard_output_write(KekStream* stream,
                                 KekStandardTextBridge* bridge,
                                 const char* data, size_t len);

#endif
