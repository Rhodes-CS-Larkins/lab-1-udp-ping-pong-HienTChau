/*
 * ping.c - UDP ping/pong client code
 *          author: <your name>
 */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"

#define PORTNO "1266"

#define BUF_SIZE 500

int main(int argc, char **argv) {
  int ch, errors = 0;
  int nping = 1;                        // default packet count
  char *ponghost = strdup("localhost"); // default host
  char *pongport = strdup(PORTNO);      // default port
  int arraysize = 100;                  // default packet size
  
  unsigned char *arr;
  int sfd, s;
  unsigned char buf[BUF_SIZE];
  ssize_t nread;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  double starttime, finishtime, elapsedtime;
  double total = 0;
  double average;


  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'h':
      ponghost = strdup(optarg);
      break;
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    case 's':
      arraysize = atoi(optarg);
      break;
    default:
      fprintf(stderr, "usage: ping [-h host] [-n #pings] [-p port] [-s size]\n");
    }
  }

  // UDP ping implemenation goes here
  // Init array

  arr = (unsigned char *)malloc(arraysize);
  if (arr == NULL){
    printf("Allocation for arr failed");
    free(ponghost);
    free(pongport);
    free(arr);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < arraysize; i++){
    arr[i] = 200;
  } 

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  
  s = getaddrinfo(ponghost, pongport, &hints, &result);
  if (s != 0){
    fprintf(stderr, "gai error: %s\n", gai_strerror(s));
    free(ponghost);
    free(pongport);
    free(arr);
    exit(EXIT_FAILURE);
  }

  for (rp = result; rp != NULL; rp = rp->ai_next){
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;
    if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break;
    close(sfd);
  }
  freeaddrinfo(result);

  if (rp == NULL){
    fprintf(stderr, "Could not connect\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < nping; i++){
    starttime = get_wctime();

    if(write(sfd, arr, arraysize) != arraysize) {
      fprintf(stderr, "partial/failed write\n");
      errors++;
      continue;
    }

    nread = read(sfd, buf, BUF_SIZE);
    finishtime = get_wctime();

    if (nread == -1){
      perror("read");
      errors++;
      continue;
    }

    if (nread != arraysize){
      printf("Expected size of  %d, got %zd\n", arraysize, nread);
      errors++;
      continue;
    }
    for (int j = 0; j < nread; j++){
      if (buf[j] != arr[j]+1){
        printf("Invalid message\n");
        errors++;
        break;
      }
    }

    elapsedtime = finishtime - starttime;
    total += elapsedtime;

    printf("ping[%d] : round-trip time: %f ms\n", i, elapsedtime);
  }

  average = total/nping;

  if (errors > 0){
    printf("nping: %d arraysize: %d errors: %d ponghost: %s pongport: %s\n",
      nping, arraysize, errors, ponghost, pongport);
  } else {
    printf("no errors detected\n");
  }

  printf("time to send %d packets of %d bytes %f ms (%f avg per packet)\n", nping, arraysize, total, average);
  free(ponghost);
  free(pongport);
  free(arr);
  return 0;
}
