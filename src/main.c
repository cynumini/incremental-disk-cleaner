#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(const char *name) {
  char *base = strrchr(name, '/');
  printf("usage: %s [options]\n", (base ? base + 1 : name));
  printf("Options\n"
         "\t-h, --help\tDisplay this help and exit.\n"
         "\t-c, --count\tFind a file by descending into the most populated "
         "branch of the directory tree. At each level, the dir with the "
         "largest number of files is chosen until reaching a dir that contains "
         "only files. The first file in that dir is returned.\n"
         "\t-s, --size\tFind the single largest file by size within the entire "
         "directory tree.\n");
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  int c;

  static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                         {"count", no_argument, 0, 'c'},
                                         {"size", no_argument, 0, 's'},
                                         {0, 0, 0, 0}};

  while (1) {
    int option_index = 0;

    if ((c = getopt_long(argc, argv, "hcs", long_options, &option_index)) == -1)
      break;

    switch (c) {
    case 'h':
      usage(argv[0]);
      break;
    case 'c':
      printf("incremental-disk-cleaner\n");
      break;
    case 's':
      printf("incremental-disk-cleaner\n");
      break;
    default:
      usage(argv[0]);
    }
  }

  return EXIT_SUCCESS;
}
