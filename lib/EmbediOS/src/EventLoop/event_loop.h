#ifndef __DEVENT_LOOP_H__
#define __DEVENT_LOOP_H__

#include <Arduino.h>

#include <stdbool.h>

#include "event_queue.h"
#include "tasks.h"

/**
 * @brief event loop struct which will schedule all of our tasks according to its priority and time interval
 */
typedef struct event_loop_t
{
    event_queue* events;
    task_vector* tasks;
    bool running;
    size_t current_index;
    task* current_task;
} event_loop;

/**
 * @brief Creates an instance of the event loop on the heap and returns it
 * @return A new instance of event loop by pointer, if fails returns NULL
 */
event_loop* create_event_loop();

/**
 * @brief Deallocates an instance of the event loop allocated on the heap
 */
void free_event_loop(event_loop* loop);

/**
 * @brief Inserts a task in the task vector of the event loop sorted by priority
 * , sorting technique used is insertion sort
 * @return true if operation was successful else false
 */
bool submit_task_to_loop(task* t, event_loop* loop);

/**
 * @brief Removes a task from the task vector of loop specified by its name
 * @return true if operation was successful else false
 */
bool remove_task_from_loop(const char* t, event_loop* loop);

/**
 * @brief Pushes a new event to the event loop event queue
 * @return true if operation was successful else false
 */
bool push_event_to_loop(event_type type, const char* refer, event_loop* loop);

/**
 * @brief Calculates the time passed from a `start` in milli-seconds
 * @return Returns the Calculated time in milli-seconds
 */
size_t __time_millis(size_t start);

/**
 * @warning Not to be used by user
 */
void loop_check_event(event_loop* loop);

/**
 * @brief Starts the event loop, does not return until `EXIT_LOOP` event is passed
 */
void loop_start(event_loop* loop);

// Helper functions

/**
 * @brief prepares and send the `EXIT_LOOP` event
 * @return returns true if successful else false
 */
bool end_loop(event_loop* loop);

/**
 * @brief prepares and send the `DELETE_TASK` event  
 * deletes the currently running task after its execution if `refer` is NULL
 * @return returns true if successful else false
 */
bool del_task(const char* refer, event_loop* loop);

/**
 * @brief prepares and send the `SKIP_NEXT_EXEC` event  
 * skips the next iteration of the currently running task after its execution if `refer` is NULL
 * @return returns true if successful else false
 */
bool skip_next_iter(const char* refer, event_loop* loop);

/**
 * @brief prepares and send the `PAUSE_TASK` event  
 * pauses the currently running task after its execution if `refer` is NULL
 * @return returns true if successful else false
 */
bool paise_task(const char* refer, event_loop* loop);

/**
 * @brief prepares and send the `RESUME_TASK` event
 * @return returns true if successful else false
 */
bool resume_task(const char* refer, event_loop* loop);

#endif