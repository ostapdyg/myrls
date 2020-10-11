#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <stdlib.h>

#include <ctype.h>

struct vector_t{
    size_t size;
    void** data;
};

typedef struct vector_t* vector;

vector vector_new(size_t size);

void* vector_get(vector self, size_t i);

int vector_push(vector self, void* data);

const void** vector_data(vector self);

size_t vector_size(vector self);

int vector_free(vector self);


#endif 
