#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main (int argc, char *argv[]) {
  int port, server_fd, client_fd, err;
  struct sockaddr_in server, client;
  char buf[BUFFER_SIZE];

  if (argc < 2) {
    printf("Usage: %s [port]\n", argv[0]);
    return 1;
  }

  port = atoi(argv[1]);

  // Create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    puts("Could not create socket\n");
    return 1;
  }

  // Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind socket
  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) {
    puts("Could not bind socket\n");
    return 1;
  }

  // Listen
  err = listen(server_fd, 128);
  if (err < 0) {
    puts("Could not listen on socket\n");
    return 1;
  }
  printf("Server is listening on port %d for incomming messages\n", port);

  // Create logfile an add the correct permissions
  FILE * logfile;
  logfile = fopen("/var/log/ushoutd.log", "a");
  fclose(logfile);
  chmod("/var/log/ushoutd.log", 0600);

  time_t timestamp;
  while (1) {
    socklen_t client_len = sizeof(client); // sizeof(struct sockaddr_in)

    // Accept connection from an incoming client
    client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

    if (client_fd < 0) {
      puts("Could not establish new connection\n");
      return 1;
    } else {
      // Write the time when the client connected into the logfile
      timestamp = time(0);
      logfile = fopen("/var/log/ushoutd.log", "a");
      fprintf(logfile,"client connected | time: %s", ctime(&timestamp));
      fclose(logfile);
    }

    while (1) {
      // Receive a message from client
      int read = recv(client_fd, buf, BUFFER_SIZE, 0);

      if (!read) break; // done reading the message from the client
      if (read < 0) {
        puts("Client read failed\n");
        return 1;
      } else {
        // Write the message's content and the time the server was reveiving the message into the logfile
        timestamp = time(0);
        logfile = fopen("/var/log/ushoutd.log","a");
        fprintf(logfile, "message received | content: \"%.*s\" | time: %s", read - 1, buf, ctime(&timestamp));
        fclose(logfile);
      }
      // Send the message back to client
      err = send(client_fd, buf, read, 0);
      if (err < 0) {
        puts("Client write failed\n");
        return 1;
      }
    }
  }

  return 0;
}
