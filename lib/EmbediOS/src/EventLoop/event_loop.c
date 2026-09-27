#include "event_loop.h"

#define cmp_retstr(cond) cond ? "true" : "false"

event_loop* create_event_loop()
{
    event_loop* loop = (event_loop*) malloc(sizeof(event_loop));
    if (loop == NULL)
        return NULL;

    loop->tasks = create_task_vector();
    if (loop->tasks == NULL)
    {
        free(loop);
        return NULL;
    }

    loop->events = create_event_queue();
    if (loop->events == NULL)
    {
        free_task_vector(loop->tasks);
        free(loop);
        return NULL;
    }

    loop->running = false;
    loop->current_task = NULL;
    loop->current_index = 0;

    return loop;
}

void free_event_loop(event_loop* loop)
{
    free_task_vector(loop->tasks);
    free_event_queue(loop->events);
    free(loop);
}

bool submit_task_to_loop(task* t, event_loop* loop)
{
    if (loop->tasks->size == 0)
    {
        return add_task(t, loop->tasks);
    }
    else
    {
        // Insertion sort into the right priority index
        for (size_t i = loop->tasks->size - 1;; i--)
        {
            task* ctask = get_task(i, loop->tasks);

            if (ctask->priority >= t->priority)
            {
                return insert_task(t, i, loop->tasks);
            }

            if (i == 0)
            {
                break;
            }
        }

        // if we were not able to find a place then just add the task
        return add_task(t, loop->tasks);
    }
}

bool remove_task_from_loop(const char* task_name, event_loop* loop)
{
    if (task_name == NULL)
    {
        if (loop->current_task == NULL)
            return false;

        bool res = remove_task(loop->current_index, loop->tasks);
        loop->current_index--;

        return res;
    }
    else
    {
        for (size_t i = 0; i < loop->tasks->size; i++)
        {
            task* ctask = get_task(i, loop->tasks);
            if (strcmp(ctask->name, task_name) == 0)
            {
                loop->current_index--;
                return remove_task(i, loop->tasks);
            }
        }
    }

    return false;
}

bool push_event_to_loop(event_type type, const char* refer, event_loop* loop)
{
    return push_event(make_event(type, refer), loop->events);
}

size_t __time_millis(size_t start)
{
    return millis() - start;
}

void loop_check_event(event_loop* loop)
{
    event e = pop_event(loop->events);
    if (e.type == NO_EVENT)
        return;

    switch (e.type)
    {
        case DELETE_TASK: {
            remove_task_from_loop(e.refer, loop);
        } break;

        case SKIP_NEXT_EXEC: {
            loop->current_task->skip = true;
        } break;

        case PAUSE_TASK: {
            if (e.refer == NULL) {
                loop->current_task->pause = true;
            } else {
                task* ctask = get_task_by_name(e.refer, loop->tasks);
                ctask->pause = true;
            }
        } break;

        case RESUME_TASK: {
            if (e.refer != NULL) {
                task* ctask = get_task_by_name(e.refer, loop->tasks);
                ctask->pause = false;
            }
        } break;

        case EXIT_LOOP: {
            loop->running = false;
        } break;
    }
}

void loop_start(event_loop* loop)
{
    loop->running = true;

    while (loop->running)
    {
        for (loop->current_index = 0; loop->current_index < loop->tasks->size; loop->current_index++)
        {
            loop->current_task = get_task(loop->current_index, loop->tasks);
            bool task_ran = false;   // A counter to check if the current task ran

            if (__time_millis(loop->current_task->start) >= loop->current_task->time)
            {
                if (!loop->current_task->skip && !loop->current_task->pause)
                {
                    loop->current_task->run(loop->current_task->parameters);
                    task_ran = true;
                }
    
                loop->current_task->start = __time_millis(loop->current_task->start);
                if (loop->current_task->skip) loop->current_task->skip = false;

                if (task_ran)
                    loop_check_event(loop);
            }
        }

        // inbuilt function provided by Espressif for accurate timings
        ets_delay_us(1000);
    }
}

bool end_loop(event_loop* loop)
{
    return push_event_to_loop(EXIT_LOOP, NULL, loop);
}

bool del_task(const char* refer, event_loop* loop)
{
    return push_event_to_loop(DELETE_TASK, refer, loop);
}

bool skip_next_iter(const char* refer, event_loop* loop)
{
    return push_event_to_loop(SKIP_NEXT_EXEC, refer, loop);
}

bool pause_task(const char* refer, event_loop* loop)
{
    return push_event_to_loop(PAUSE_TASK, refer, loop);
}

bool resume_task(const char* refer, event_loop* loop)
{
    return push_event_to_loop(RESUME_TASK, refer, loop);
}

#undef cmp_retstr