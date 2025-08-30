#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <uchar.h>

typedef struct {
  char *path;
  size_t size;
  size_t parent_id;
  size_t child_id;
  size_t elements;
  bool root;
} Entry;

Entry *entries = NULL;
size_t entries_len = 0;
size_t entries_capacity = 1024;

void walk(size_t root_id) {
  DIR *root_dir = opendir(entries[root_id].path);
  if (root_dir == NULL)
    return;
  struct dirent *d;
  while ((d = readdir(root_dir)) != NULL) {
    if (strcmp(d->d_name, "..") == 0 || strcmp(d->d_name, ".") == 0)
      continue;
    char *path = malloc(strlen(entries[root_id].path) + strlen(d->d_name) + 2);
    strcpy(path, entries[root_id].path);
    strcat(path, "/");
    strcat(path, d->d_name);
    struct stat stat;
    lstat(path, &stat);
    if (entries_len == entries_capacity) {
      entries_capacity *= 1.5;
      entries = realloc(entries, sizeof(Entry) * entries_capacity);
    }
    size_t child_id = entries_len++;
    entries[child_id] =
        (Entry){.path = path, .size = stat.st_size, .parent_id = root_id, .elements = 0, .child_id = 0};
    if (d->d_type == 4) {
      walk(child_id);
    }
    entries[root_id].elements += entries[child_id].elements + 1;
    entries[root_id].size += entries[child_id].size;
  }
  closedir(root_dir);
}

void select(size_t offset, size_t parent_id) {
  size_t best_match_value = 0;
  size_t best_match_index = offset;
  Entry *entry = entries + offset;
  while (offset < entries_len) {
    if (entry->parent_id == parent_id) {
      if (entry->elements > best_match_value) {
        best_match_value = entry->elements;
        best_match_index = offset;
      }
    }
    offset++;
    entry++;
  }
  entry = entries + best_match_index;
  if (entry->elements > 0) {
    select(best_match_index + 1, best_match_index);
  } else {
    size_t child_id = 0;
    size_t current = best_match_index;
    while (true) {
      entry->child_id = child_id;
      child_id = current;
      current = entry->parent_id;
      entry = entries + current;
      if (entry->root == true) {
        entry->child_id = child_id;
        break;
      }
    }
    printf("%s - %zu - %zu\n", entry->path, entry->elements, entry->child_id);
    while (entry->child_id) {
      entry = entries + entry->child_id;
      if (entry->elements == 0) {
        printf("%s\n", entry->path);
      } else {
        printf("%s - %zu\n", entry->path, entry->elements);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  entries = malloc(sizeof(Entry) * entries_capacity);
  char *home_path = strdup(getenv("HOME"));
  entries[entries_len++] =
      (Entry){.path = home_path, .size = 0, .parent_id = 0, .elements = 0, .child_id = 0, .root = true};
  walk(0);
  select(1, 0);
  free(entries);
  return EXIT_SUCCESS;
}
