#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "ushout.h"

FILE * mklog(char * filename) {
  FILE * log = fopen(filename, "a");
  fclose(log);
  if (chmod("/var/log/ushoutd.log", 0600) < 0) {
    remove(filename);
    puts("Failed to write permissions. You have to run this program as root.\n");
    exit(1);
  }
  return log;
}
