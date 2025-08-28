#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct Entry Entry;

struct Entry {
  char *path;
  bool path_owned;
  Entry *parent;
  Entry *children;
  size_t children_capacity;
  size_t children_index;
  size_t children_len;
  size_t elements;
  size_t size;
};

Entry entry_create(char *path, bool path_owned, Entry *parent, size_t size) {
  size_t capacity = 8;
  return (Entry){
      .path = path,
      .path_owned = path_owned,
      .parent = parent,
      .children = malloc(sizeof(Entry) * capacity),
      .children_capacity = capacity,
      .children_len = 0,
      .elements = 0,
      .size = size,

  };
}

void entry_destroy(Entry *entry) {
  for (size_t i = 0; i < entry->children_len; i++) {
    entry_destroy(entry->children + i);
  }
  if (entry->path_owned) {
    free(entry->path);
  }
  free(entry->children);
}

void add_element(Entry *self) {
  if (self->parent != NULL) {
    add_element(self->parent);
  }
  self->elements++;
}

void add_child(Entry *self, Entry *child) {
  if (self->children_len == self->children_capacity) {
    self->children_capacity *= 1.5;
    self->children = realloc(self->children, sizeof(Entry) * self->children_capacity);
  }
  memcpy(self->children + self->children_len, child, sizeof(Entry));
  self->children_len++;
  self->size += child->size;
  add_element(self);
}

void print_entry(Entry *entry, bool a) {
  printf("%s %zu %f\n", entry->path, entry->elements, entry->size / 1024 / 1024.0);
  for (size_t i = 0; i < entry->children_len; i++) {
    if (a)
      print_entry(entry->children + i, false);
  }
}

int cmp_element(const void *a, const void *b) { return ((Entry *)b)->elements - ((Entry *)a)->elements; }

int cmp_size(const void *a, const void *b) {
  long result = ((Entry *)b)->size - ((Entry *)a)->size;
  if (result < 0) {
    return -1;
  } else if (result > 0) {
    return 1;
  }
  return 0;
}

void sort_entry(Entry *entry, bool sort_by_size) {
  if (sort_by_size) {
    qsort(entry->children, entry->children_len, sizeof(Entry), cmp_size);
  } else {
    qsort(entry->children, entry->children_len, sizeof(Entry), cmp_element);
  }
  for (size_t i = 0; i < entry->children_len; i++) {
    sort_entry(entry->children + i, sort_by_size);
    (entry->children + i)->parent = entry; // TODO: check if it's work
  }
}

void walk(Entry *parent) {
  DIR *root_dir = opendir(parent->path); // home_dir
  if (root_dir == NULL) {
    return;
  }
  struct dirent *d;
  while ((d = readdir(root_dir)) != NULL) {
    // ignore .. and .
    if (strcmp(d->d_name, "..") == 0 || strcmp(d->d_name, ".") == 0) {
      continue;
    }
    char *path = malloc(strlen(parent->path) + strlen(d->d_name) + 2);
    strcpy(path, parent->path);
    strcat(path, "/");
    strcat(path, d->d_name);
    struct stat stat;
    lstat(path, &stat);
    Entry entry = entry_create(path, true, parent, stat.st_size);
    if (d->d_type == 4) {
      walk(&entry);
    }
    add_child(parent, &entry);
  }
  closedir(root_dir);
}

int main(int argc, char *argv[]) {
  printf("incremental-disk-cleaner\n");

  char *home = getenv("HOME");
  Entry root_entry = entry_create(home, false, NULL, 0);
  walk(&root_entry);
  sort_entry(&root_entry, true);
  print_entry(&root_entry, true);
  entry_destroy(&root_entry);

  return EXIT_SUCCESS;
}
