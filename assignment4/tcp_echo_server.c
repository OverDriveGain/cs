#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "ushout.h"

#define BUFFER_SIZE 1024
#define CLIENT_MAX    10

int main (int argc, char *argv[]) {
  int port, server_fd, client_fd, err, i;
  struct sockaddr_in server, client;
  int client_fds[10], client_active;
  socklen_t client_len;
  char buf[BUFFER_SIZE];
  fd_set read_fds;
  int max_fd;

  time_t timestamp;
  FILE * log;

  if (argc < 2) {
    printf("Usage: %s [port]\n", argv[0]);
    return 1;
  }

  /* Create logfile and add the correct permissions */
  log = mklog("/var/log/ushoutd.log");

  /* initialise each client_fds[1..CLIENT_MAX] with 0 so they are not checked */
  for (i = 0; i < CLIENT_MAX; i++) {
      client_fds[i] = 0;
  }

  /* Create server socket (and explicitly allow multiple connections) */
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    puts("Could not create socket\n");
    return 1;
  }

  /* Prepare the sockaddr_in structure */
  port = atoi(argv[1]);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  /* Bind socket */
  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) {
    puts("Could not bind socket\n");
    return 1;
  }

  /* Listen with a maximum of 5 pending connections */
  err = listen(server_fd, 5);
  if (err < 0) {
    puts("Could not listen on socket\n");
    return 1;
  }
  printf("Server is listening on port %d for incomming messages\n", port);

  /* close all file descriptors, except our communication socket	*/
  /*closeAllFileDescriptorsExcept(server_fd);*/

  client_len = sizeof(client); /* sizeof(struct sockaddr_in) */

  while (1) {
    /* clear all fds in set and the buffer (aka latest message) */
    FD_ZERO(&read_fds);
    memset(buf,0,strlen(buf));

    /* adding server fd */
    FD_SET(server_fd, &read_fds);
    max_fd = server_fd;

    /* adding child fds */
    for (i = 0; i < CLIENT_MAX; i++) {
      client_fd = client_fds[i];

      /* add valid client fd to read list */
      if(client_fd > 0) {
        FD_SET(client_fd, &read_fds);
      }

      /* remember the highest fd number (needed by the select function) */
      if(client_fd > max_fd) {
        max_fd = client_fd;
      }
    }

    /* check for activities on the fds (no timeout) */
    client_active = select(max_fd + 1 , &read_fds , NULL , NULL , NULL);

    if ((client_active < 0) && (errno!=EINTR)) {
      printf("Select failed\n");
      return 1;
    }

    /* If there is any activity on the server socket then there is an incoming connection */
    if (FD_ISSET(server_fd, &read_fds)) {
      /* Accept connection from the incoming client */
      client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);
      if (client_fd < 0) {
        puts("Could not establish new connection\n");
        return 1;
      }

      /* Write the time when the client connected and his ip address into the log */
      timestamp = time(0);
      log = fopen("/var/log/ushoutd.log", "a");
      fprintf(log,"Client connected | ip: %s | time: %s", inet_ntoa(client.sin_addr), ctime(&timestamp));
      fclose(log);

      /* print some informations about the newly connected client */
      printf("New connection | client\'s fd = %d | ip = %s\n",
        client_fd, inet_ntoa(client.sin_addr));

      /* adding new client fd to the fd set (client_fds) */
      for (i = 0; i < CLIENT_MAX; i++) {
        if (client_fds[i] == 0)
        {
          client_fds[i] = client_fd;
          printf("Adding to list of client fds as %d\n" , i);
          break;
        }
      }
    }

    /* Maybe there are IO operation on some other sockets */
    for (i = 0; i < CLIENT_MAX; i++) {
      client_fd = client_fds[i];

      if (FD_ISSET(client_fd, &read_fds)) {
        /* Receive a message from client */
        int read = recv(client_fd, buf, BUFFER_SIZE, 0);
        if (read < 0) {
          puts("Client read failed\n");
          return 1;
        } else if (read == 0) { /* Client disconnected */
          getpeername(client_fd , (struct sockaddr*) &client, (socklen_t*) &client_len);
          printf("Client disconnected | ip: %s\n", inet_ntoa(client.sin_addr));
          /* Log the disconneted client */
          timestamp = time(0);
          log = fopen("/var/log/ushoutd.log","a");
          fprintf(log, "Client disconnected | ip: %s | time: %s", inet_ntoa(client.sin_addr), ctime(&timestamp));
          fclose(log);
          /* Close the fd (socket) and set it to 0 in array for reuse */
          close(client_fd);
          client_fds[i] = 0;
        } else { /* received message from client */
          getpeername(client_fd , (struct sockaddr*) &client, (socklen_t*) &client_len);
          /* Write the message's content and the time the server was reveiving the message into the log */
          timestamp = time(0);
          log = fopen("/var/log/ushoutd.log","a");
          fprintf(log, "Message received | ip: %s | content: \"%.*s\" | time: %s", inet_ntoa(client.sin_addr), read-1, buf, ctime(&timestamp));
          fclose(log);
          /* Send the message to all clients if there is one */
          for (i = 0; i < max_fd; i++) {
            client_fd = client_fds[i];
            if (client_fd != 0) {
              err = send(client_fd, buf, read, 0);
              if (err < 0) {
                puts("Send message back to client failed\n");
                return 1;
              }
            }
          }
        }
      }
    }
  }

  return 0;
}
