#ifndef KEK_RUNTIME_EVENT_H
#define KEK_RUNTIME_EVENT_H

#include <stddef.h>
#include <stdint.h>

#define KEK_EVENT_MAX_SUBSCRIBERS 32
#define KEK_EVENT_QUEUE_CAPACITY 256
#define KEK_EVENT_DATA_CAPACITY 1024
#define KEK_EVENT_TYPE_COUNT 6

typedef enum KekEventType {
    KEK_EVENT_STREAM_DATA = 0,
    KEK_EVENT_STREAM_EOF = 1,
    KEK_EVENT_STREAM_ERROR = 2,
    KEK_EVENT_STATE_CHANGED = 3,
    KEK_EVENT_STATE_CREATED = 4,
    KEK_EVENT_STATE_DELETED = 5
} KekEventType;

typedef struct KekEvent {
    KekEventType type;
    void* source;
    size_t state_type_id;
    size_t state_slot_id;
    uint64_t state_version;
    char data[KEK_EVENT_DATA_CAPACITY];
    size_t data_len;
    int error_code;
} KekEvent;

typedef void (*KekEventHandler)(const KekEvent* event, void* context);

typedef struct KekEventSubscriber {
    KekEventHandler handler;
    void* context;
    int active;
} KekEventSubscriber;

typedef struct KekEventSubscriberList {
    KekEventSubscriber subscribers[KEK_EVENT_MAX_SUBSCRIBERS];
    size_t count;
} KekEventSubscriberList;

typedef struct KekEventDispatcher {
    KekEventSubscriberList subscriber_lists[KEK_EVENT_TYPE_COUNT];
    KekEvent queue[KEK_EVENT_QUEUE_CAPACITY];
    size_t queue_start;
    size_t queue_end;
    size_t queue_size;
} KekEventDispatcher;

void kek_event_dispatcher_init(KekEventDispatcher* dispatcher);
void kek_event_subscribe(KekEventDispatcher* dispatcher, KekEventType type,
                         KekEventHandler handler, void* context);
void kek_event_unsubscribe(KekEventDispatcher* dispatcher, KekEventType type,
                           KekEventHandler handler, void* context);
int kek_event_publish(KekEventDispatcher* dispatcher, const KekEvent* event);
void kek_event_dispatch_pending(KekEventDispatcher* dispatcher);
int kek_event_has_pending(const KekEventDispatcher* dispatcher);

#endif
