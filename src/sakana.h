#ifndef SAKANA_H
#define SAKANA_H

#include <stddef.h>

// Section - types

typedef size_t usize;
typedef int bool;
#define false 0
#define true 1

// Section - mem

#define mem_alloc(type, len) (type *)_mem_alloc(sizeof(type) * len, __FILE__, __LINE__)
#define mem_create(type) (type *)_mem_alloc(sizeof(type), __FILE__, __LINE__)
#define mem_free(address) _mem_free((void *)address)
void *_mem_alloc(usize size, const char *file, usize line);
void _mem_free(void *address);
void mem_check();

// Section - str

const char *str_join(const char **strings, usize len);

// Section - misc

const char *get_home();

#endif
