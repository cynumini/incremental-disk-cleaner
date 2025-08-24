#include "sakana.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Section - mem

typedef struct {
  void *address;
  const char *file;
  usize line;
  bool freed;
} MemoryEntry;

#define MEMORY_ENTRIES_LEN 1024
MemoryEntry memory_entries[MEMORY_ENTRIES_LEN];
usize current_memory_entry_index = 0;

void *_mem_alloc(usize size, const char *file, usize line) {
  void *address = malloc(size);
  assert(current_memory_entry_index < MEMORY_ENTRIES_LEN);
  memory_entries[current_memory_entry_index++] = (MemoryEntry){address, file, line, false};
  return address;
}

void _mem_free(void *address) {
  bool ok = false;
  for (usize i = 0; i < current_memory_entry_index; i++) {
    if (address == memory_entries[i].address) {
      ok = true;
      memory_entries[i].freed = true;
    }
  }
  assert(ok);
  free(address);
}

void mem_check() {
  for (usize i = 0; i < current_memory_entry_index; i++) {
    if (memory_entries[i].freed != true) {
      fprintf(stderr, "%s:%zu: error: memory allocated here was never freed.\n", memory_entries[i].file,
              memory_entries[i].line);
    }
  }
}

// Section - str

const char *str_join(const char **strings, usize len) {
  usize total_len = 0;
  for (usize i = 0; i < len; i++) {
    total_len += strlen(strings[i]);
  }
  char *result = mem_alloc(char, ++total_len);
  char *offset = result;
  for (usize i = 0; i < len; i++) {
    strcpy(offset, strings[i]);
    offset += strlen(strings[i]);
  }
  return result;
}

const char *str_dupe(const char *str) {
  char *result = mem_alloc(char, strlen(str) + 1);
  strcpy(result, str);
  return result;
}

// Section - misc

const char *get_home() { return getenv("HOME"); }
