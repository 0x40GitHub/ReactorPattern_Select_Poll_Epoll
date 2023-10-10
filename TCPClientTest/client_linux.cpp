#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define PORT 9999
char sbuf[51], rbuf[101];

int main() {
  int sockfd;
  struct sockaddr_in saddr;

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(PORT);
  saddr.sin_addr.s_addr = inet_addr("192.168.78.130");

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket");
    return -1;
  }

  if (connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
    perror("connect");
    return -1;
  }
  if (recv(sockfd, rbuf, sizeof(rbuf), 0) < 0) {
    perror("recv");
    return -1;
  }
  printf("server reply:%s\n", rbuf);

  printf("please enter data:\n");
  while (true)
  {
    scanf("%s", sbuf);

    if (send(sockfd, sbuf, sizeof(sbuf), 0) < 0) {
      perror("send");
      return -1;
    }

    if (recv(sockfd, rbuf, sizeof(rbuf), 0) < 0) {
      perror("recv");
      return -1;
    }

    printf("server reply:%s\n", rbuf);
  }
  
  close(sockfd);
  return 0;
}