/*
 * pong.c - UDP ping/pong server code
 *          author: <your name>
 */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"

#define PORTNO "1266"
#define BUF_SIZE 500


int main(int argc, char **argv) {
  int ch;
  int nping = 1;                    // default packet count
  char *pongport = strdup(PORTNO);  // default port
  
  int sfd, s;
  unsigned char buf[BUF_SIZE];
  ssize_t nread;
  socklen_t peer_addrlen;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  struct sockaddr_storage peer_addr;
  char ip[INET_ADDRSTRLEN];
  void *addr;


  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    default:
      fprintf(stderr, "usage: pong [-n #pings] [-p port]\n");
    }
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  s = getaddrinfo(NULL, pongport, &hints, &result);
  if (s != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    free(pongport);
    exit(EXIT_FAILURE);
  }
  
  for (rp = result; rp != NULL; rp = rp-> ai_next){
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;
      if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) 
        break;
      close(sfd);
  }
  
  
  freeaddrinfo(result);

  if (rp == NULL) {
    fprintf(stderr, "Could not bind\n");
    free(pongport);
    exit(EXIT_FAILURE);
  }

  int pingcount = 0;
  while(pingcount < nping){

    peer_addrlen = sizeof(peer_addr);
    nread = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr *) &peer_addr, &peer_addrlen);

    if (nread == -1)
      continue;
    
    addr = &((struct sockaddr_in *)&peer_addr)->sin_addr;
    inet_ntop(AF_INET, addr, ip, sizeof(ip));


    for (int i = 0; i < nread; i++){
      buf[i]++;
    }

      if (sendto(sfd, buf, nread, 0, (struct sockaddr *) &peer_addr, peer_addrlen) != nread){
        fprintf(stderr, "Error sending response\n");
    } else
      printf("pong[%d]: received packet from %s\n", pingcount, ip);
     pingcount++;

  }
  printf("exiting\n");
  free(pongport);
  return 0;
}

