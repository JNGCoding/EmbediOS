#ifndef __DEVENT_TASKS_H__
#define __DEVENT_TASKS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Use this macro to easily define a runnable task
// name = name of the function
// param = name of the void*
#define define_task(name, param) void name(const void* param)

typedef void(*runnable)(const void*);

typedef enum task_priority_t
{
    TOP_PRIORITY,
    MID_PRIORITY,
    BOT_PRIORITY
} task_priority;

typedef struct task_t
{
    // Name of the task
    const char* name;

    // Function parameters
    runnable run;
    const void* parameters;

    // Priority of the task
    task_priority priority;

    // Timing variables of the task
    size_t time;
    size_t start;

    // State variables of the task
    bool skip;
    bool pause;
} task;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns an instance of the task struct allocated on the stack
 */
task make_task(
    const char* name,
    runnable func,
    const void* param,
    size_t interval,
    task_priority priority
);

#define TASK_VECTOR_MULTIPLY_FACTOR (1.5f)
#define TASK_VECTOR_INITIAL_SIZE (32)

/**
 * @brief Vector data structure for holding all of our tasks
 */
typedef struct task_vector_t {
    task** vector;
    size_t size, capacity;
} task_vector;

/**
 * @brief Initializes a new task vector on the heap and returns it
 * @return returns the instance of task vector by pointer, if fails then returns NULL 
 */
task_vector* create_task_vector();

/**
 * @brief Deallocates an instance of task vector allocated on the heap
 */
void free_task_vector(task_vector* tvec);

/**
 * @brief Appends a task* to the ending of the vector
 * @return true if operation was successful else false
 */
bool add_task(task* t, task_vector* tvec);

/**
 * @brief Removes a task* from the vector
 * @return true if operation was successful else false
 */
bool remove_task(size_t ind, task_vector* tvec);

/**
 * @brief Inserts a task* in the vector at `ind`
 * @return true if operation was successful else false
 */
bool insert_task(task* t, size_t ind, task_vector* tvec);

/**
 * @brief Retrieves a task* from the vector at `ind`
 * @return task* if operation was successful else NULL
 */
task* get_task(size_t ind, task_vector* tvec);

/**
 * @brief Searches and returns a task* from the vector specified by `name`
 * @return task* if operation was successful else NULL
 */
task* get_task_by_name(const char* name, task_vector* tvec);

/**
 * @brief Prints a formatted output of `tvec`
 */
void print_task_list(task_vector* tvec);

#ifdef __cplusplus
}
#endif

#endif