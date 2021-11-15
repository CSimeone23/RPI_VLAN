#include </usr/include/arm-linux-gnueabihf/sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>

void create_udp_socket(int *udp_socket, char *ipv4_address, int port){
	*udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(*udp_socket == -1){
		handle_error("socket");
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ipv4_address);
	if(bind(*udp_socket, (struct sockaddr*) &addr, sizeof(addr)) == -1){
		handle_error("bind");
	}
	printf("UDP Socket %s:%d was created successfully!\n", ipv4_address, port);
}
