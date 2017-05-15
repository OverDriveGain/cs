#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

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

/*void closeAllFileDescriptorsExcept(int fd) { */
    /* calculate size of file descriptors table */
    /*int fdTableSize = getdtablesize();
    int excludedFD = fd;

    printf("Table size: %d\n", fdTableSize);
    */
    /* close all file descriptors, except our communication socket	*/
    /* this is done to avoid blocking on tty operations and such.	*/
    /*for (int i=0; i < fdTableSize; i++) {
    	if (i != excludedFD) {
        close(i);
      }
    }
}*/
