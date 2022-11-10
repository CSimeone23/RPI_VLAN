#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include </usr/include/arm-linux-gnueabihf/sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

#define MAX_BUFFER_SIZE 65536
FILE *logfile;
int raw_socket;

struct sockaddr_ll {
  unsigned short sll_family;   /* Always AF_PACKET */
  unsigned short sll_protocol; /* Physical-layer protocol */
  int            sll_ifindex;  /* Interface number */
  unsigned short sll_hatype;   /* ARP hardware type */
  unsigned char  sll_pkttype;  /* Packet type */
  unsigned char  sll_halen;    /* Length of address */
  unsigned char  sll_addr[8];  /* Physical-layer address */
};

int main(int argc, char *argv[]) {
  int saddr_size, data_size;
  struct sockaddr saddr;
  struct in_addr in;

  unsigned char *buffer = (unsigned char*)malloc(MAX_BUFFER_SIZE);

  logfile = fopen("log.txt", "w");
  if(logfile == NULL) {
    printf("Unable to create Log File.\n");
    exit(EXIT_FAILURE);
  }

  printf("Starting...\n"); 
  raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if(raw_socket < 0) {
    printf("Make sure you run this as Root User\n");
    printf("Error Creating socket, error_code: %d\n", errno);
    exit(EXIT_FAILURE);
  }
  printf("Created Socket!\n");

  printf("Listening...\n");
  int maxCount = 20;
  int currentCount = 0;
  while(currentCount < maxCount) {
    saddr_size = sizeof saddr;
    data_size = recvfrom(raw_socket, buffer, MAX_BUFFER_SIZE, 0, &saddr, &saddr_size);
    if(data_size < 0) {
      printf("Recvfrom Error, failed to get packets\n");
      exit(EXIT_FAILURE);
    }
    printf("Buffer = %s", buffer);
    currentCount++;
  } // End of while loop

  printf("Finished Execution :)\n");
  exit(EXIT_SUCCESS);
}