#include <stdio.h>
#include <unistd.h>
#include <resolv.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

enum command {
  CMD_HELP,
  CMD_WALLET,
  CMD_NONE
};

struct server_command {
  char *key;
  int value;
  int arity;
};

struct server_command commands[] = {
  {"help", CMD_HELP, 0}
};

struct server_command none = {"none", CMD_NONE, 0};

#define MAX_COMMANDS (sizeof(commands) / sizeof(struct server_command))

struct server_command str_to_command(char *cmd_string) {
  for (int i = 0; i < MAX_COMMANDS; i++) {
    struct server_command cmd = commands[i];

    printf("%s\n", cmd.key);

    if (strcmp(cmd.key, cmd_string) == 0) {
      return cmd;
    }
  }

  return none;
}

int read_from_client (int client_fd) {
  char buffer[1024];
  int nbytes;

  nbytes = read(client_fd, buffer, 1024);

  if (nbytes < 0) {
    fprintf(stderr, "Read err\n");
    return -1;
  } else if (nbytes == 0) {
    return -1;
  } else {
    buffer[strcspn(buffer, "\r\n")] = 0;
    struct server_command cmd = str_to_command(buffer);

    switch(cmd.value) {
      case CMD_HELP: {
        char out[] = "\n"
          "- commands -\n"
          "help\n"
          "wallet new\n"
          "wallet list\n"
        "";

        write(client_fd, out, strlen(out));
        break;
      }
      case CMD_NONE: {
        char out[] = "invalid command specified\n";
        write(client_fd, out, strlen(out));
        break;
      }
    }
  }

  return 0;
}

int start_server() {
  struct sockaddr_in self;
  int socket_desc;
  char buffer[1024];
  fd_set active_fd_set, read_fd_set;

  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc == -1) {
    fprintf(stderr, "Could not create socket\n");
    return -1;
  }

  self.sin_family = AF_INET;
  self.sin_port = htons(9898);
  self.sin_addr.s_addr = INADDR_ANY;

  int opt = 1;
  if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0 ) {
    fprintf(stderr, "Could not set options on socket\n");
    return -1;
  }

  if (bind(socket_desc, (struct sockaddr*) &self, sizeof(self)) != 0) {
    fprintf(stderr, "Could not bind socket\n");
    return -1;
  }

  if (listen(socket_desc, 20) != 0) {
    fprintf(stderr, "Could not listen socket\n");
    return -1;
  }

  printf("Listening on port 9898..\n");

  FD_ZERO(&active_fd_set);
  FD_SET(socket_desc, &active_fd_set);

  while (1) {
    read_fd_set = active_fd_set;

    if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
      fprintf(stderr, "Select err\n");
      return -1;
    }

    for (int i = 0; i < FD_SETSIZE; i++) {
      if (FD_ISSET(i, &read_fd_set)) {
        if (i == socket_desc) {
          int clientfd;
          struct sockaddr_in client_addr;
          unsigned int addr_len = sizeof(client_addr);
          clientfd = accept(socket_desc, (struct sockaddr *) &client_addr, &addr_len);

          if (clientfd < 0) {
            fprintf(stderr, "Accept err\n");
            return -1;
          }

          fprintf(stderr, "Server: connect from host %s\n", inet_ntoa(client_addr.sin_addr));
          FD_SET(clientfd, &active_fd_set);
        } else if (read_from_client(i) < 0) {
          close(i);
          FD_CLR(i, &active_fd_set);
        }
      }
    }
  }

  close(socket_desc);

  return 0;
}
