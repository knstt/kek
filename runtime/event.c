#include "event.h"

#include <stdio.h>
#include <string.h>

void kek_event_dispatcher_init(KekEventDispatcher* dispatcher) {
    memset(dispatcher, 0, sizeof(*dispatcher));
}

void kek_event_subscribe(KekEventDispatcher* dispatcher, KekEventType type,
                         KekEventHandler handler, void* context) {
    if (!dispatcher || type < 0 || type >= KEK_EVENT_TYPE_COUNT || !handler) {
        return;
    }

    KekEventSubscriberList* list = &dispatcher->subscriber_lists[type];
    if (list->count >= KEK_EVENT_MAX_SUBSCRIBERS) {
        fprintf(stderr, "event subscriber list full\n");
        return;
    }

    list->subscribers[list->count].handler = handler;
    list->subscribers[list->count].context = context;
    list->subscribers[list->count].active = 1;
    list->count++;
}

void kek_event_unsubscribe(KekEventDispatcher* dispatcher, KekEventType type,
                           KekEventHandler handler, void* context) {
    if (!dispatcher || type < 0 || type >= KEK_EVENT_TYPE_COUNT || !handler) {
        return;
    }

    KekEventSubscriberList* list = &dispatcher->subscriber_lists[type];
    for (size_t i = 0; i < list->count; i++) {
        KekEventSubscriber* subscriber = &list->subscribers[i];
        if (subscriber->handler == handler && subscriber->context == context) {
            subscriber->active = 0;
        }
    }
}

int kek_event_publish(KekEventDispatcher* dispatcher, const KekEvent* event) {
    if (!dispatcher || !event) {
        return 0;
    }
    if (dispatcher->queue_size >= KEK_EVENT_QUEUE_CAPACITY) {
        fprintf(stderr, "event queue full, event dropped\n");
        return 0;
    }

    dispatcher->queue[dispatcher->queue_end] = *event;
    dispatcher->queue_end = (dispatcher->queue_end + 1) % KEK_EVENT_QUEUE_CAPACITY;
    dispatcher->queue_size++;
    return 1;
}

void kek_event_dispatch_pending(KekEventDispatcher* dispatcher) {
    if (!dispatcher) {
        return;
    }

    while (dispatcher->queue_size > 0) {
        KekEvent event = dispatcher->queue[dispatcher->queue_start];
        dispatcher->queue_start = (dispatcher->queue_start + 1) % KEK_EVENT_QUEUE_CAPACITY;
        dispatcher->queue_size--;

        if (event.type < 0 || event.type >= KEK_EVENT_TYPE_COUNT) {
            continue;
        }

        KekEventSubscriberList* list = &dispatcher->subscriber_lists[event.type];
        for (size_t i = 0; i < list->count; i++) {
            KekEventSubscriber* subscriber = &list->subscribers[i];
            if (subscriber->active) {
                subscriber->handler(&event, subscriber->context);
            }
        }
    }
}

int kek_event_has_pending(const KekEventDispatcher* dispatcher) {
    return dispatcher && dispatcher->queue_size > 0;
}
