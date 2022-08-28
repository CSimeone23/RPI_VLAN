#include <stdio.h>
#include </usr/include/arm-linux-gnueabihf/sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>

void getErrno() {
  switch(errno) {
    case EBADF:
      printf("EBADF\n");
      return;
    case EFAULT:
      printf("EFAULT\n");
      return;
    case EINVAL:
      printf("EINVAL\n");
      return;
    case ENOTTY:
      printf("ENOTTY\n");
      return;
    default:
      printf("A new error??? errno = %d\n", errno);
  }
}

int main(int argc, char* argv[]) {
  struct ifreq ifr;
  int enable_flag = 1;

  // Xbox mac address starts with 1C:1A:DF
  char spoofed_mac_address = "1C:1A:DF:12:12:12"
  sscanf(spoofed_mac_address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
      &ifr.ifr_hwaddr.sa_data[0],
      &ifr.ifr_hwaddr.sa_data[1],
      &ifr.ifr_hwaddr.sa_data[2],
      &ifr.ifr_hwaddr.sa_data[3],
      &ifr.ifr_hwaddr.sa_data[4],
      &ifr.ifr_hwaddr.sa_data[5]);

  int test_socket = socket(AF_INET, SOCK_DGRA, 0);
  if (test_socket == -1) {
    printf("Failed to create socket\n");
    return 1;
  }

  // MAC SPOOFING //
  strcpy(ifr.ifr_name, "eth0");
  ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

  int isSuccessful = ioctl(test_socket, SIOCSIFHWADDR, &ifr);
  if (isSuccessful != -1) {
    printf("MAC ADDRESS CHANGE SUCCESSFULLY!!\n");
    return 0;
  }

  // Fails
  getErrno();
  return 1;
}