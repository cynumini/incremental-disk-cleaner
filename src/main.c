#include "sakana.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  printf("incremental-disk-cleaner\n");
  const char *home = get_home();
  DIR *home_dir = opendir(home); // home_dir
  struct dirent *d;
  while ((d = readdir(home_dir)) != NULL) {
    const char *path = str_join((const char *[]){home, "/", d->d_name}, 3); // path
    printf("%s %i\n", path, d->d_type);
    mem_free(path); // !path
  }
  closedir(home_dir); // !home_dir
  mem_check();
  return EXIT_SUCCESS;
}
