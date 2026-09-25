#ifndef __DEVENT_LIB_H__
#define __DEVENT_LIB_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum event_type_t
{
    NO_EVENT,
    DELETE_TASK,
    SKIP_NEXT_EXEC,
    PAUSE_TASK,
    RESUME_TASK,
    EXIT_LOOP
} event_type;

typedef struct event_t
{
    event_type type;
    const char* refer;
} event;

/**
 * @brief Returns an instance of event allocated on the stack
 */
event make_event(event_type type, const char* ref);

#define EVENT_QUEUE_SIZE (4096)

typedef struct event_queue_t
{
    event events[EVENT_QUEUE_SIZE];
    size_t size, readptr, writeptr;
} event_queue;

/**
 * @brief Initializes a new event_queue on the heap and returns it
 * @return a new empty instance of event_queue by pointer, if failed returns NULL
 */
event_queue* create_event_queue();

/**
 * @brief Deallocates an event_queue allocated on the heap
 */
void free_event_queue(event_queue* equeue);

/**
 * @brief Pushes an event onto the queue
 * @return true if the operation was successful else false
 */
bool push_event(event e, event_queue* equeue);

/**
 * @brief pops an event out of the queue
 * @return event if an event was present else NO_EVENT 
 */
event pop_event(event_queue* equeue);

/**
 * @brief prints the entire queue
 */
void print_event_queue(event_queue* equeue);

#endif