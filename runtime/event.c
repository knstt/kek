#include "event.h"

#include <string.h>

void kek_event_dispatcher_init(KekEventDispatcher* dispatcher) {
    memset(dispatcher, 0, sizeof(*dispatcher));
}

static int subscriber_matches(const KekEventSubscriber* subscriber,
                              KekEventHandler handler, void* context) {
    return subscriber && subscriber->active && subscriber->handler == handler &&
           subscriber->context == context;
}

int kek_event_subscribe(KekEventDispatcher* dispatcher, KekEventType type,
                        KekEventHandler handler, void* context) {
    if (!dispatcher || type < 0 || type >= KEK_EVENT_TYPE_COUNT || !handler) {
        return 0;
    }

    KekEventSubscriberList* list = &dispatcher->subscriber_lists[type];
    for (size_t i = 0; i < list->count; i++) {
        KekEventSubscriber* subscriber = &list->subscribers[i];
        if (subscriber_matches(subscriber, handler, context)) {
            return 1;
        }
        if (!subscriber->active) {
            subscriber->handler = handler;
            subscriber->context = context;
            subscriber->active = 1;
            return 1;
        }
    }

    if (list->count >= KEK_EVENT_MAX_SUBSCRIBERS) {
        return 0;
    }

    list->subscribers[list->count].handler = handler;
    list->subscribers[list->count].context = context;
    list->subscribers[list->count].active = 1;
    list->count++;
    return 1;
}

int kek_event_unsubscribe(KekEventDispatcher* dispatcher, KekEventType type,
                          KekEventHandler handler, void* context) {
    int removed = 0;
    if (!dispatcher || type < 0 || type >= KEK_EVENT_TYPE_COUNT || !handler) {
        return 0;
    }

    KekEventSubscriberList* list = &dispatcher->subscriber_lists[type];
    size_t write_index = 0;
    for (size_t i = 0; i < list->count; i++) {
        KekEventSubscriber* subscriber = &list->subscribers[i];
        if (subscriber_matches(subscriber, handler, context)) {
            removed = 1;
            continue;
        }
        if (subscriber->active) {
            list->subscribers[write_index++] = *subscriber;
        }
    }
    for (size_t i = write_index; i < list->count; i++) {
        memset(&list->subscribers[i], 0, sizeof(list->subscribers[i]));
    }
    list->count = write_index;
    return removed;
}

int kek_event_publish(KekEventDispatcher* dispatcher, const KekEvent* event) {
    if (!dispatcher || !event) {
        return 0;
    }
    if (dispatcher->queue_size >= KEK_EVENT_QUEUE_CAPACITY) {
        return 0;
    }

    dispatcher->queue[dispatcher->queue_end] = *event;
    dispatcher->queue_end = (dispatcher->queue_end + 1) % KEK_EVENT_QUEUE_CAPACITY;
    dispatcher->queue_size++;
    return 1;
}

int kek_event_dispatch_pending(KekEventDispatcher* dispatcher) {
    if (!dispatcher) {
        return 0;
    }

    while (dispatcher->queue_size > 0) {
        KekEvent event = dispatcher->queue[dispatcher->queue_start];
        dispatcher->queue_start = (dispatcher->queue_start + 1) % KEK_EVENT_QUEUE_CAPACITY;
        dispatcher->queue_size--;

        if (event.type < 0 || event.type >= KEK_EVENT_TYPE_COUNT) {
            continue;
        }

        KekEventSubscriberList* list = &dispatcher->subscriber_lists[event.type];
        size_t dispatch_count = list->count;
        for (size_t i = 0; i < dispatch_count; i++) {
            KekEventSubscriber* subscriber = &list->subscribers[i];
            if (subscriber->active) {
                if (!subscriber->handler(&event, subscriber->context)) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int kek_event_has_pending(const KekEventDispatcher* dispatcher) {
    return dispatcher && dispatcher->queue_size > 0;
}

size_t kek_event_capacity_remaining(const KekEventDispatcher* dispatcher) {
    if (!dispatcher || dispatcher->queue_size >= KEK_EVENT_QUEUE_CAPACITY) {
        return 0;
    }
    return KEK_EVENT_QUEUE_CAPACITY - dispatcher->queue_size;
}

int kek_event_transaction_begin(KekEventDispatcher* dispatcher,
                                KekEventTransaction* transaction) {
    if (!dispatcher || !transaction) {
        return 0;
    }
    transaction->dispatcher = dispatcher;
    transaction->queue_start = dispatcher->queue_start;
    transaction->queue_end = dispatcher->queue_end;
    transaction->queue_size = dispatcher->queue_size;
    return 1;
}

void kek_event_transaction_commit(KekEventTransaction* transaction) {
    if (transaction) {
        memset(transaction, 0, sizeof(*transaction));
    }
}

void kek_event_transaction_rollback(KekEventTransaction* transaction) {
    if (!transaction || !transaction->dispatcher) {
        return;
    }
    transaction->dispatcher->queue_start = transaction->queue_start;
    transaction->dispatcher->queue_end = transaction->queue_end;
    transaction->dispatcher->queue_size = transaction->queue_size;
    memset(transaction, 0, sizeof(*transaction));
}
