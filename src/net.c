#include <stdio.h>
#include <unistd.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int start_server() {
  struct sockaddr_in self;
  int socket_desc;
  char buffer[1024];
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc == -1) {
    fprintf(stderr, "Could not create socket\n");
    return -1;
  }

  self.sin_family = AF_INET;
  self.sin_port = htons(9898);
  self.sin_addr.s_addr = INADDR_ANY;

  if (bind(socket_desc, (struct sockaddr*) &self, sizeof(self)) != 0) {
    fprintf(stderr, "Could not bind socket\n");
    return -1;
  }

  if (listen(socket_desc, 20) != 0) {
    fprintf(stderr, "Could not listen socket\n");
    return -1;
  }

  printf("Listening on port 9898..\n");

  while (1) {
    int clientfd;
    struct sockaddr_in client_addr;
    unsigned int addr_len = sizeof(client_addr);

    clientfd = accept(socket_desc, (struct sockaddr*) &client_addr, &addr_len);
    printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    /*---Echo back anything sent---*/
    send(clientfd, buffer, recv(clientfd, buffer, 1024, 0), 0);

    /*---Close data connection---*/
    close(clientfd);
  }

  close(socket_desc);

  return 0;
}
