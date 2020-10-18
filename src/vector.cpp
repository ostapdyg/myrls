
#include <vector/vector.h>
// #include <stdio.h>

// Basic vararray/vector implementation (nothing fancy here)

vector vector_new(size_t msize)
{
    // printf("Create: size=%d ", msize);

    vector self = (vector)malloc(sizeof(vector_t));
    self->size = 0;
    self->data = (void **)malloc(sizeof(*(self->data)) * msize);
    return self;
}

void *vector_get(vector self, size_t i)
{
    if (i < self->size)
    {
        return (self->data)[i];
    }
    return 0;
}

int vector_push(vector self, void *data)
{
    // printf("Push: size=%d, data=%x ", self->size, data);
    self->data = (void **)realloc(self->data, sizeof(void **) * (self->size + 1));
    if (!self->data)
    {
        return -1;
    }
    self->data[self->size] = data;
    self->size += 1;
    return 0;
}

const void **vector_data(vector self)
{
    return (const void **)self->data;
}

size_t vector_size(vector self)
{
    return self->size;
}

int vector_free(vector self)
{
    free(self->data);
    free(self);
    return 0;
}