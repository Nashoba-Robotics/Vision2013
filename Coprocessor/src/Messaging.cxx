#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include "Messaging.h"

#define BUFFERSIZE		1024

// Print out information about the targets to the console
// Send a message about the targets to the crio
int sendMessage(const char *ipAddr, float distance, float angle, float tension) {
  char buffer[BUFFERSIZE];
  char sendbuffer[BUFFERSIZE];
  struct sockaddr_in addr;
  int sd;
  unsigned int addr_size;
  
  if ( (sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("Socket");
    return -1;
  }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(500);
  if ( inet_aton(ipAddr, &addr.sin_addr) == 0 ) {
    perror(ipAddr);
    return -1;
  }
  sprintf(sendbuffer, "Distance=%f:Angle=%f:Tension=%f", distance, angle, tension);
  //  printf("Distance=%f:Angle%f:Tension=%f\n", distance,angle,tension);
  sendto(sd, sendbuffer, strlen(sendbuffer)+1, 0, (struct sockaddr*)&addr, sizeof(addr));
  close(sd);
}

