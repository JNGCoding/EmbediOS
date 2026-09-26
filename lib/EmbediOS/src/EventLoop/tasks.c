#include "tasks.h"

inline task make_task(
    const char* name,
    runnable func,
    const void* param,
    size_t interval,
    task_priority priority
)
{
    return (task) {name, func, param, priority, interval, 0, false, false};
}

task_vector* create_task_vector()
{
    task_vector* tvec = (task_vector*) malloc(sizeof(task_vector));
    if (tvec == NULL)
        return NULL;

    tvec->capacity = TASK_VECTOR_INITIAL_SIZE;
    tvec->size = 0;
    tvec->vector = (task**) calloc(TASK_VECTOR_INITIAL_SIZE, sizeof(task*));

    return tvec;
}

void free_task_vector(task_vector* tvec)
{
    if (tvec != NULL)
    {
        free(tvec->vector);
        free(tvec);
    }
}

bool add_task(task* t, task_vector* tvec)
{
    if (tvec->size < tvec->capacity)
    {
        tvec->vector[tvec->size] = t;
        tvec->size += 1;
    }
    else
    {
        task** nbuf = (task**) realloc(tvec->vector, tvec->capacity * TASK_VECTOR_MULTIPLY_FACTOR * sizeof(task*));
        if (nbuf == NULL)
            return false;

        tvec->vector = nbuf;
        tvec->capacity *= TASK_VECTOR_MULTIPLY_FACTOR;

        tvec->vector[tvec->size] = t;
        tvec->size += 1;
    }

    return true;
}

bool remove_task(size_t ind, task_vector* tvec)
{
    if (ind >= tvec->size)
        return false;

    for (size_t i = ind; i < tvec->size - 1; i++)
        tvec->vector[i] = tvec->vector[i + 1];

    tvec->size -= 1;

    return true;
}

bool insert_task(task* t, size_t ind, task_vector* tvec)
{
    if (ind > tvec->size)
        return false;

    if (tvec->size + 1 >= tvec->capacity)
    {
        task** nbuf = (task**) realloc(tvec->vector, tvec->capacity * TASK_VECTOR_MULTIPLY_FACTOR * sizeof(task*));
        if (nbuf == NULL)
            return false;

        tvec->vector = nbuf;
        tvec->capacity *= TASK_VECTOR_MULTIPLY_FACTOR;
    }
    
    for (size_t i = tvec->size + 1; i > ind; i--)
    {
        tvec->vector[i] = tvec->vector[i - 1];
    }

    tvec->vector[ind] = t;
    tvec->size++;

    return true;
}

task* get_task(size_t ind, task_vector* tvec)
{
    if (ind >= tvec->size)
        return NULL;
    return tvec->vector[ind];
}

void print_task_list(task_vector* tvec)
{
    printf("task_vector(size = %llu) {", tvec->size);

    for (size_t i = 0; i < tvec->size; i++)
    {
        printf("%s. ", tvec->vector[i]->name);
    }

    printf("}");
}

task* get_task_by_name(const char* name, task_vector* tvec)
{
    for (size_t i = 0; i < tvec->size++; i++)
    {
        if (strcmp(name, tvec->vector[i]->name) == 0)
            return tvec->vector[i];
    }

    return NULL;
}

#undef TASK_VECTOR_MULTIPLY_FACTOR
#undef TASK_VECTOR_INITIAL_SIZE