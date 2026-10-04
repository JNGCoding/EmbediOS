#include "event_queue.h"

inline event make_event(event_type type, const char* ref) {
    return (event) {type, ref};
}

event_queue* create_event_queue()
{
    event_queue* queue = (event_queue*) malloc(sizeof(event_queue));
    if (queue == NULL)
        return NULL;

    queue->size = 0;
    queue->readptr = 0;
    queue->writeptr = 0;

    return queue;
}

void free_event_queue(event_queue* equeue)
{
    if (equeue != NULL)
        free(equeue);
}

bool push_event(event e, event_queue* equeue)
{
    if (equeue->size >= EVENT_QUEUE_SIZE) {
        return false;
    }

    equeue->events[equeue->writeptr] = e;
    equeue->writeptr = (equeue->writeptr + 1) % EVENT_QUEUE_SIZE;
    equeue->size++;

    return true;
}

event pop_event(event_queue* equeue)
{
    if (equeue->size <= 0)
        return (event) {NO_EVENT, NULL};

    event elem = equeue->events[equeue->readptr];
    equeue->readptr = (equeue->readptr + 1) % EVENT_QUEUE_SIZE;
    equeue->size--;

    return elem;
}
