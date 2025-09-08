#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define global static

typedef int64_t i64;
typedef size_t usize;

typedef struct
{
    char path[PATH_MAX];
    i64 parent_id;
    i64 child_id;
    usize count;
    usize size;
} Entry;

global Entry *entries;
global usize entries_len;
global usize entries_capacity = 1024;
global char **known_paths;
global usize known_paths_len;
global usize known_paths_capacity = 1024;
global bool use_count = true;

bool is_path_in_known_paths(char *path)
{
    for (usize i = 0; i < known_paths_len; i++)
    {
        if (strcmp(known_paths[i], path) == 0)
        {
            return true;
        }
    }
    return false;
}

void add_known_path(char *path)
{
    if (known_paths_len == known_paths_capacity)
    {
        known_paths_capacity *= 2;
        known_paths = realloc(known_paths, sizeof(char *) * known_paths_capacity);
    }
    known_paths[known_paths_len++] = strdup(path);
}

void walk(const char *root_path, i64 root_id)
{
    DIR *root_dir = opendir(root_path);
    if (root_dir == NULL)
        return;
    struct dirent *dirent;
    while ((dirent = readdir(root_dir)) != NULL)
    {
        Entry entry = (Entry){{0}, root_id, -1, 0, 0};
        if (strcmp(dirent->d_name, "..") == 0 || strcmp(dirent->d_name, ".") == 0)
            continue;
        strcpy(entry.path, root_path);
        strcat(entry.path, "/");
        strcat(entry.path, dirent->d_name);
        if (is_path_in_known_paths(entry.path))
            continue;
        struct stat stat;
        lstat(entry.path, &stat);
        entry.size += stat.st_size;
        if (entries_len == entries_capacity)
        {
            entries_capacity *= 2;
            entries = realloc(entries, sizeof(Entry) * entries_capacity);
        }
        usize id = entries_len++;
        entries[id] = entry;
        if (dirent->d_type == 4)
            walk(entry.path, id);
        if (root_id != -1)
        {
            entries[root_id].count += entries[id].count + 1;
            entries[root_id].size += entries[id].size;
        }
    }
    closedir(root_dir);
}

usize select(i64 root_id, bool use_count)
{
    i64 best_id = root_id;
    usize best_value = 0;
    Entry *entry;
    for (usize i = root_id == -1 ? 0 : root_id + 1; i < entries_len; i++)
    {
        entry = entries + i;
        if (entry->parent_id != root_id)
            continue;
        if (use_count)
        {
            if (entry->count >= best_value)
            {
                best_value = entry->count;
                best_id = i;
            }
        }
        else
        {
            if (entry->size >= best_value)
            {
                best_value = entry->size;
                best_id = i;
            }
        }
    }
    entry = entries + best_id;
    if (entry->count > 0)
    {
        entry->child_id = select(best_id, use_count);
    }
    return best_id;
}

void print_help_and_exit(void)
{
    printf("Usage: idc [command]\n\nCommands:\n\n");
    printf("  size\tSelect by size\n");
    printf("  count\tSelect by count\n");
    exit(0);
}

void print_entry(Entry *entry)
{
    if (use_count)
    {
        if (entry->count > 0)
        {
            printf("%s - %zu\n", entry->path, entry->count);
        }
        else
        {
            printf("%s\n", entry->path);
        }
    }
    else if (entry->size < 1024)
        printf("%s - %zuB\n", entry->path, entry->size);
    else if (entry->size < pow(1024, 2))
        printf("%s - %.2fK\n", entry->path, entry->size / 1024.0);
    else if (entry->size < pow(1024, 3))
        printf("%s - %.2fM\n", entry->path, entry->size / pow(1024, 2));
    else if (entry->size < pow(1024, 4))
        printf("%s - %.2fG\n", entry->path, entry->size / pow(1024, 3));
}

int main(int argc, char *argv[])
{
    // arguments
    switch (argc)
    {
    case 1:
        use_count = true;
        break;
    case 2:
        if (strcmp(argv[1], "count") == 0)
            use_count = true;
        else if (strcmp(argv[1], "size") == 0)
            use_count = false;
        else
            print_help_and_exit();
        break;
    default:
        print_help_and_exit();
        break;
    }

    // allocate memmory
    entries = calloc(entries_capacity, sizeof(Entry));
    known_paths = calloc(known_paths_capacity, sizeof(char *));

    // get home path
    char *home_path_from_env = getenv("HOME");

    // load known_paths
    char *data_home_path_from_env = getenv("XDG_DATA_HOME");
    char data_home_path[PATH_MAX];
    if (data_home_path_from_env != NULL)
    {
        strcpy(data_home_path, data_home_path_from_env);
    }
    else
    {
        strcpy(data_home_path, home_path_from_env);
        strcat(data_home_path, "/.local/share");
    }
    strcat(data_home_path, "/idc");
    int result = mkdir(data_home_path, 0755);
    assert(result != -1 || errno == EEXIST);
    char known_paths_path[PATH_MAX];
    strcpy(known_paths_path, data_home_path);
    strcat(known_paths_path, "/known_files.txt");
    FILE *file;
    file = fopen(known_paths_path, "r");
    if (file != NULL)
    {
        fseek(file, 0, SEEK_END);
        usize size = ftell(file);
        rewind(file);
        char *data = calloc(size + 1, sizeof(char));
        fread(data, size, 1, file);
        char *token = strtok(data, "\n");
        while (token != NULL)
        {
            if (strlen(token) > strlen(home_path_from_env))
            {
                add_known_path(token);
            }
            token = strtok(NULL, "\n");
        }
        free(data);
        fclose(file);
    }

    // walk
    walk(home_path_from_env, -1);

    // select
    usize best_id = select(-1, use_count);
    Entry *entry = entries + best_id;

    // print deepest child
    for (;;)
    {
        print_entry(entry);
        if (entry->child_id != -1)
        {
            entry = entries + entry->child_id;
            continue;
        }
        break;
    }

    // user input
    char input[256] = {0};
    bool running = true;
    while (running)
    {
        printf("Add (a) | Up (u) | Down (d) | Quit (q): ");
        scanf("%s", input);
        if (strlen(input) > 1)
            continue;
        switch (input[0])
        {
        case 'a':
            add_known_path(entry->path);
            running = false;
            break;
        case 'q':
            running = false;
            break;
        case 'u':
            if (entry->parent_id != -1)
                entry = entries + entry->parent_id;
            print_entry(entry);
            break;
        case 'd':
            if (entry->child_id != -1)
                entry = entries + entry->child_id;
            print_entry(entry);
            break;
        }
    }

    // free and save
    free(entries);
    file = fopen(known_paths_path, "w");
    assert(file != NULL);
    for (usize i = 0; i < known_paths_len; i++)
    {
        fprintf(file, "%s\n", known_paths[i]);
        free(known_paths[i]);
    }
    fclose(file);
    free(known_paths);

    return EXIT_SUCCESS;
}
