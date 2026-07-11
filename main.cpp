#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define STREAM_BUFFER_SIZE 1024
#define MAX_EVENT_SUBSCRIBERS 32
#define MAX_EVENT_QUEUE 256
#define MAX_REGISTERED_STREAMS 64

// ============================================================================
// Event System - Observer Pattern
// ============================================================================

enum EventType {
    EVENT_STREAM_DATA_AVAILABLE,
    EVENT_STREAM_EOF,
    EVENT_STREAM_ERROR,
    EVENT_QUIT
};

struct Event {
    enum EventType type;
    void* source;  // Stream pointer
    char data[STREAM_BUFFER_SIZE];
    size_t dataLength;
};

// Event handler callback
typedef void (*EventHandler)(struct Event* event, void* context);

// Event subscriber entry
struct Subscriber {
    EventHandler handler;
    void* context;
    int active;
};

// Event subscriber list for a specific event type
struct SubscriberList {
    struct Subscriber subscribers[MAX_EVENT_SUBSCRIBERS];
    size_t count;
    pthread_mutex_t mutex;
};

// Central event dispatcher
struct EventDispatcher {
    struct SubscriberList subscriberLists[4];  // One per event type
    struct Event eventQueue[MAX_EVENT_QUEUE];
    size_t queueStart;
    size_t queueEnd;
    size_t queueSize;
    pthread_mutex_t queueMutex;
    sem_t queueSemaphore;
    int running;
    pthread_t dispatcherThread;
};

// ============================================================================
// Event Dispatcher Functions
// ============================================================================

struct EventDispatcher* EventDispatcherCreate() {
    struct EventDispatcher* dispatcher = 
        (struct EventDispatcher*)malloc(sizeof(struct EventDispatcher));
    memset(dispatcher, 0, sizeof(struct EventDispatcher));
    
    for (int i = 0; i < 4; i++) {
        pthread_mutex_init(&dispatcher->subscriberLists[i].mutex, NULL);
    }
    
    pthread_mutex_init(&dispatcher->queueMutex, NULL);
    sem_init(&dispatcher->queueSemaphore, 0, 0);
    dispatcher->running = 1;
    
    return dispatcher;
}

void EventDispatcherSubscribe(struct EventDispatcher* dispatcher, 
                               enum EventType type,
                               EventHandler handler, 
                               void* context) {
    struct SubscriberList* list = &dispatcher->subscriberLists[type];
    pthread_mutex_lock(&list->mutex);
    
    if (list->count < MAX_EVENT_SUBSCRIBERS) {
        list->subscribers[list->count].handler = handler;
        list->subscribers[list->count].context = context;
        list->subscribers[list->count].active = 1;
        list->count++;
    }
    
    pthread_mutex_unlock(&list->mutex);
}

void EventDispatcherUnsubscribe(struct EventDispatcher* dispatcher,
                                 enum EventType type,
                                 EventHandler handler) {
    struct SubscriberList* list = &dispatcher->subscriberLists[type];
    pthread_mutex_lock(&list->mutex);
    
    for (size_t i = 0; i < list->count; i++) {
        if (list->subscribers[i].handler == handler) {
            list->subscribers[i].active = 0;
        }
    }
    
    pthread_mutex_unlock(&list->mutex);
}

void EventDispatcherPublish(struct EventDispatcher* dispatcher, 
                             struct Event* event) {
    pthread_mutex_lock(&dispatcher->queueMutex);
    
    if (dispatcher->queueSize < MAX_EVENT_QUEUE) {
        memcpy(&dispatcher->eventQueue[dispatcher->queueEnd], event, 
               sizeof(struct Event));
        dispatcher->queueEnd = (dispatcher->queueEnd + 1) % MAX_EVENT_QUEUE;
        dispatcher->queueSize++;
        
        sem_post(&dispatcher->queueSemaphore);
    } else {
        fprintf(stderr, "Event queue full, event dropped\n");
    }
    
    pthread_mutex_unlock(&dispatcher->queueMutex);
}

// Dispatcher thread - processes events and notifies subscribers
void* dispatcherThreadFunction(void* arg) {
    struct EventDispatcher* dispatcher = (struct EventDispatcher*)arg;
    
    while (dispatcher->running) {
        sem_wait(&dispatcher->queueSemaphore);
        
        if (!dispatcher->running) {
            break;
        }
        
        pthread_mutex_lock(&dispatcher->queueMutex);
        
        if (dispatcher->queueSize > 0) {
            struct Event event;
            memcpy(&event, &dispatcher->eventQueue[dispatcher->queueStart],
                   sizeof(struct Event));
            dispatcher->queueStart = (dispatcher->queueStart + 1) % MAX_EVENT_QUEUE;
            dispatcher->queueSize--;
            
            pthread_mutex_unlock(&dispatcher->queueMutex);
            
            // Dispatch event to all subscribers
            struct SubscriberList* list = &dispatcher->subscriberLists[event.type];
            pthread_mutex_lock(&list->mutex);
            
            for (size_t i = 0; i < list->count; i++) {
                if (list->subscribers[i].active) {
                    list->subscribers[i].handler(&event, 
                                                  list->subscribers[i].context);
                }
            }
            
            pthread_mutex_unlock(&list->mutex);
        } else {
            pthread_mutex_unlock(&dispatcher->queueMutex);
        }
    }
    
    return NULL;
}

void EventDispatcherStart(struct EventDispatcher* dispatcher) {
    pthread_create(&dispatcher->dispatcherThread, NULL, 
                   dispatcherThreadFunction, dispatcher);
}

void EventDispatcherStop(struct EventDispatcher* dispatcher) {
    dispatcher->running = 0;
    sem_post(&dispatcher->queueSemaphore);
    pthread_join(dispatcher->dispatcherThread, NULL);
}

void EventDispatcherDestroy(struct EventDispatcher* dispatcher) {
    for (int i = 0; i < 4; i++) {
        pthread_mutex_destroy(&dispatcher->subscriberLists[i].mutex);
    }
    pthread_mutex_destroy(&dispatcher->queueMutex);
    sem_destroy(&dispatcher->queueSemaphore);
    free(dispatcher);
}

// ============================================================================
// Stream Management (simplified for events)
// ============================================================================

enum StreamMode { STREAM_READ, STREAM_WRITE };

struct Stream {
    int fd;
    enum StreamMode mode;
    char buffer[STREAM_BUFFER_SIZE];
    size_t length;
    pthread_mutex_t mutex;
    int running;
    pthread_t thread;
    struct EventDispatcher* dispatcher;
};

void StreamInit(struct Stream* stream, int fd, enum StreamMode mode,
                struct EventDispatcher* dispatcher) {
    memset(stream, 0, sizeof(struct Stream));
    stream->fd = fd;
    stream->mode = mode;
    stream->running = 1;
    stream->dispatcher = dispatcher;
    pthread_mutex_init(&stream->mutex, NULL);
}

void StreamDestroy(struct Stream* stream) {
    pthread_mutex_destroy(&stream->mutex);
}

// Get data from a read stream
char PopChar(struct Stream* stream) {
    if (stream->length > 0) {
        char ch = stream->buffer[0];
        memmove(stream->buffer, stream->buffer + 1, stream->length - 1);
        stream->length--;
        return ch;
    } else {
        return '\0';
    }
}

// Read all available data from a read stream
int StreamReadAll(struct Stream* stream, char* outputBuffer, size_t maxLength) {
    if (stream->mode != STREAM_READ) {
        return 0;
    }
    
    pthread_mutex_lock(&stream->mutex);
    size_t i = 0;
    while (i < maxLength - 1 && stream->length > 0) {
        outputBuffer[i++] = PopChar(stream);
    }
    outputBuffer[i] = '\0';
    pthread_mutex_unlock(&stream->mutex);

    return i;
}

// Write data to a write stream
void StreamWrite(struct Stream* stream, const char* data) {
    if (stream->mode != STREAM_WRITE) {
        return;
    }
    
    pthread_mutex_lock(&stream->mutex);
    
    size_t dataLen = strlen(data);
    size_t available = sizeof(stream->buffer) - stream->length - 1;
    size_t toCopy = (dataLen < available) ? dataLen : available;
    
    memcpy(stream->buffer + stream->length, data, toCopy);
    stream->length += toCopy;
    stream->buffer[stream->length] = '\0';
    
    pthread_mutex_unlock(&stream->mutex);
}

// Write raw data (without null terminator requirement)
void StreamWriteRaw(struct Stream* stream, const char* data, size_t len) {
    if (stream->mode != STREAM_WRITE) {
        return;
    }
    
    pthread_mutex_lock(&stream->mutex);
    
    size_t available = sizeof(stream->buffer) - stream->length - 1;
    size_t toCopy = (len < available) ? len : available;
    
    memcpy(stream->buffer + stream->length, data, toCopy);
    stream->length += toCopy;
    stream->buffer[stream->length] = '\0';
    
    pthread_mutex_unlock(&stream->mutex);
}

// Reader thread function for input streams
void* readerThreadFunction(void* arg) {
    struct Stream* stream = (struct Stream*)arg;
    
    while (stream->running) {
        // Use select() to wait for data with a timeout
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(stream->fd, &readfds);
        
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;  // 100ms timeout
        
        int selectResult = select(stream->fd + 1, &readfds, NULL, NULL, &tv);
        
        if (selectResult > 0 && FD_ISSET(stream->fd, &readfds)) {
            // Data is available
            char temp[STREAM_BUFFER_SIZE];
            ssize_t bytes = read(stream->fd, temp, sizeof(temp) - 1);
            
            if (bytes > 0) {
                // Publish data available event with only the new data
                struct Event event;
                event.type = EVENT_STREAM_DATA_AVAILABLE;
                event.source = stream;
                memcpy(event.data, temp, bytes);
                event.dataLength = bytes;
                EventDispatcherPublish(stream->dispatcher, &event);
                
                // Also add to stream buffer for consumers
                pthread_mutex_lock(&stream->mutex);
                
                size_t available = sizeof(stream->buffer) - stream->length - 1;
                size_t toCopy = (bytes < (ssize_t)available) ? bytes : available;
                memcpy(stream->buffer + stream->length, temp, toCopy);
                stream->length += toCopy;
                stream->buffer[stream->length] = '\0';
                
                pthread_mutex_unlock(&stream->mutex);
            } else if (bytes == 0) {
                // EOF - publish EOF event
                struct Event event;
                event.type = EVENT_STREAM_EOF;
                event.source = stream;
                event.dataLength = 0;
                EventDispatcherPublish(stream->dispatcher, &event);
                stream->running = 0;
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                // Error - publish error event
                struct Event event;
                event.type = EVENT_STREAM_ERROR;
                event.source = stream;
                event.dataLength = 0;
                EventDispatcherPublish(stream->dispatcher, &event);
                perror("read");
                stream->running = 0;
            }
        } else if (selectResult < 0) {
            // Error in select
            struct Event event;
            event.type = EVENT_STREAM_ERROR;
            event.source = stream;
            event.dataLength = 0;
            EventDispatcherPublish(stream->dispatcher, &event);
            perror("select");
            stream->running = 0;
        }
    }
    
    return NULL;
}

// Writer thread function for output streams
void* writerThreadFunction(void* arg) {
    struct Stream* stream = (struct Stream*)arg;
    
    while (stream->running) {
        pthread_mutex_lock(&stream->mutex);
        
        // Write all data in buffer
        if (stream->length > 0) {
            ssize_t written = write(stream->fd, stream->buffer, stream->length);
            
            if (written > 0) {
                // Remove written data from buffer
                memmove(stream->buffer, stream->buffer + written, 
                        stream->length - written);
                stream->length -= written;
                stream->buffer[stream->length] = '\0';
            }
        }
        
        pthread_mutex_unlock(&stream->mutex);
        
        // Small sleep to prevent busy waiting
        usleep(10000);  // 10ms
    }
    
    return NULL;
}

// Start a reader stream
void StreamStartReader(struct Stream* stream) {
    if (stream->mode != STREAM_READ) {
        return;
    }
    pthread_create(&stream->thread, NULL, readerThreadFunction, stream);
}

// Start a writer stream
void StreamStartWriter(struct Stream* stream) {
    if (stream->mode != STREAM_WRITE) {
        return;
    }
    pthread_create(&stream->thread, NULL, writerThreadFunction, stream);
}

// Stop a stream and wait for its thread to finish
void StreamStop(struct Stream* stream) {
    stream->running = 0;
    pthread_join(stream->thread, NULL);
}

// ============================================================================
// Event Handlers - Application Logic
// ============================================================================

void logToFileEventHandler(struct Event* event, void* context) {
    if (event->type != EVENT_STREAM_DATA_AVAILABLE) {
        return;
    }
    
    struct Stream* fileStream = (struct Stream*)context;
    
    // Write received data to file
    if (event->dataLength > 0) {
        StreamWrite(fileStream, (const char*)event->data);
    }
}

void displayKeyPressEventHandler(struct Event* event, void* context) {
    if (event->type != EVENT_STREAM_DATA_AVAILABLE) {
        return;
    }
    
    struct Stream* outputStream = (struct Stream*)context;
    
    // Display what was pressed
    for (size_t i = 0; i < event->dataLength; i++) {
        if (event->data[i] == '\n') {
            StreamWrite(outputStream, "[ENTER]\n");
        } else if (event->data[i] == '\x1b') {
            StreamWrite(outputStream, "[ESC]");
        } else if (event->data[i] < 32) {
            char special[32];
            snprintf(special, sizeof(special), "[CTRL+%c]", event->data[i] + 64);
            StreamWrite(outputStream, special);
        } else {
            StreamWriteRaw(outputStream, &event->data[i], 1);
        }
    }
}

void quitEventHandler(struct Event* event, void* context) {
    if (event->type != EVENT_STREAM_DATA_AVAILABLE) {
        return;
    }
    
    sem_t* doneSemaphore = (sem_t*)context;
    struct Stream* stream = (struct Stream*)event->source;
    
    // Check if 'q' was pressed
    for (size_t i = 0; i < event->dataLength; i++) {
        if (event->data[i] == 'q') {
            // Consume the data
            char buffer[STREAM_BUFFER_SIZE];
            StreamReadAll(stream, buffer, sizeof(buffer));
            
            sem_post(doneSemaphore);
            break;
        }
    }
}

// ============================================================================
// Runtime Management
// ============================================================================

struct AsyncRuntime {
    struct EventDispatcher* dispatcher;
    struct Stream input, output, logFile;
    struct termios originalTermios;
};

struct AsyncRuntime* RuntimeCreate() {
    struct AsyncRuntime* runtime = 
        (struct AsyncRuntime*)malloc(sizeof(struct AsyncRuntime));
    memset(runtime, 0, sizeof(struct AsyncRuntime));
    runtime->dispatcher = EventDispatcherCreate();
    memset(&runtime->originalTermios, 0, sizeof(struct termios));
    return runtime;
}

// Enable raw mode for stdin
void RuntimeEnableRawMode(struct AsyncRuntime* runtime) {
    // Check if stdin is a TTY
    if (!isatty(STDIN_FILENO)) {
        return;
    }
    
    struct termios raw;
    
    // Get current terminal settings
    if (tcgetattr(STDIN_FILENO, &runtime->originalTermios) == -1) {
        perror("tcgetattr");
        return;
    }
    
    raw = runtime->originalTermios;
    
    // Disable canonical mode and echo
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;   // Non-blocking read
    raw.c_cc[VTIME] = 0;  // No timeout
    
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
    }
}

// Restore terminal to original mode
void RuntimeDisableRawMode(struct AsyncRuntime* runtime) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &runtime->originalTermios);
}

void RuntimeDestroy(struct AsyncRuntime* runtime) {
    RuntimeDisableRawMode(runtime);
    EventDispatcherDestroy(runtime->dispatcher);
    free(runtime);
}

// ============================================================================
// Main Program
// ============================================================================

int main(int argc, char** argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;

    // Create runtime
    struct AsyncRuntime* runtime = RuntimeCreate();
    
    // Enable raw mode for immediate key capture
    RuntimeEnableRawMode(runtime);
    
    // Create streams
    StreamInit(&runtime->input, STDIN_FILENO, STREAM_READ, runtime->dispatcher);
    StreamInit(&runtime->output, STDOUT_FILENO, STREAM_WRITE, runtime->dispatcher);
    
    // Open log file for writing
    int logFd = open("keyboard_log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (logFd == -1) {
        perror("Failed to open log file");
        RuntimeDestroy(runtime);
        return 1;
    }
    StreamInit(&runtime->logFile, logFd, STREAM_WRITE, runtime->dispatcher);
    
    // Start the dispatcher
    EventDispatcherStart(runtime->dispatcher);
    
    // Start stream reader/writer threads
    StreamStartReader(&runtime->input);
    StreamStartWriter(&runtime->output);
    StreamStartWriter(&runtime->logFile);
    
    // Write initial message
    StreamWrite(&runtime->output, "Raw keyboard input mode - Press 'q' to quit\n");
    StreamWrite(&runtime->output, "Characters: ");
    
    // Subscribe event handlers
    EventDispatcherSubscribe(runtime->dispatcher, EVENT_STREAM_DATA_AVAILABLE,
                             logToFileEventHandler, &runtime->logFile);
    EventDispatcherSubscribe(runtime->dispatcher, EVENT_STREAM_DATA_AVAILABLE,
                             displayKeyPressEventHandler, &runtime->output);
    
    sem_t doneSemaphore;
    sem_init(&doneSemaphore, 0, 0);
    EventDispatcherSubscribe(runtime->dispatcher, EVENT_STREAM_DATA_AVAILABLE,
                             quitEventHandler, &doneSemaphore);
    
    // Wait until 'q' is pressed
    sem_wait(&doneSemaphore);
    
    // Cleanup
    StreamWrite(&runtime->output, "\n\nQuitting...\n");
    
    // Stop streams BEFORE stopping dispatcher to avoid race conditions
    StreamStop(&runtime->input);
    StreamStop(&runtime->output);
    StreamStop(&runtime->logFile);
    
    // Stop dispatcher after streams are stopped
    EventDispatcherStop(runtime->dispatcher);
    
    // Close log file
    close(logFd);
    
    // Cleanup resources
    StreamDestroy(&runtime->input);
    StreamDestroy(&runtime->output);
    StreamDestroy(&runtime->logFile);
    sem_destroy(&doneSemaphore);
    RuntimeDestroy(runtime);
    
    printf("Keyboard log saved to keyboard_log.txt\n");

    return 0;
}
