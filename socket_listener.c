#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include </usr/include/arm-linux-gnueabihf/sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "error_handlers.h"

#define BROADCAST_IP "255.255.255.255"
#define NUM_THREADS 3

int THREAD_ID = 1;
struct sockaddr_in XBOX_ADDRESS;
struct sockaddr_in HUBSERVER_ADDRESS;

// TEMP ////////////////////////////////////
struct sockaddr_in TEMP_DIRECT_XBOX_ADDRESS;
struct sockaddr_in PHAUX_ADDRESS;
struct sockaddr_in TEST_ADDRESS;
////////////////////////////////////////////

pthread_t threads[NUM_THREADS];

struct thread_data {
	int *socket;
	int thread_id;
	int port_num;
	struct sockaddr_in sendto_address;
	struct sockaddr_in sendto_address2;
};

void print_buffer_with_recv_len(char *buf, int recv_len){
	printf("\n\"|");
	for(int i=0; i<recv_len; i++){
		printf("%d|", buf[i]);
	}
	printf("\"\n");
}

void send_datagram(int socket, char *buf, size_t recv_len, struct sockaddr* to, int slen){
	int send_to = sendto(socket, buf, recv_len, 0, to, slen);
	if(send_to == -1){
		handle_error("sendto");
	}
}

void setSocketToCommunicateWithHubServer(int *server_socket){
	char buf[512];
	int recv_len, slen = sizeof(HUBSERVER_ADDRESS);

	
	// Introduce server_socket and laptop_socket (hub server)
	char init_message[18] = "talk to me shawty";
	send_datagram(*server_socket, init_message, 18, (struct sockaddr*) &HUBSERVER_ADDRESS, slen);
	printf("Incoming traffic listener setup completed successfully!\n");
}

void write_data_to_file(char *buf){
	FILE *file_pointer;
	file_pointer = fopen("./DATA.txt", "w");
	if(file_pointer == NULL){
		printf("File Error!\n");
		exit(EXIT_FAILURE);
	}
	fprintf(file_pointer, "%s", buf);
	fclose(file_pointer);
}

/**
 * Creates and listens on wifi facing socket
 */
void *wifi_facing_udp_listener_thread(void *arg){
	struct thread_data *t_data = arg;
	struct sockaddr_in incoming_connection_address;
	int recv_len;
	int slen = sizeof(incoming_connection_address);
	printf("Listening for data on Wifi Socket [Thread #%d]...\n", t_data->thread_id);
	while(1) {
		char *buf = malloc(512*sizeof(char));
		fflush(stdin);
		recv_len = recvfrom(*(t_data->socket), buf, 512, 0, (struct sockaddr*) &incoming_connection_address, (unsigned int*) &slen);
		if(recv_len == -1) {
			printf("!===!\nError listening on Thread #%d\n!===!\n", t_data->thread_id);
			continue;
		}
		printf("Thread #%d Received: \"%X\"\n\tFrom: %s:%d\n", t_data->thread_id, buf, inet_ntoa(incoming_connection_address.sin_addr), ntohs(incoming_connection_address.sin_port));
	
		// Check if incomming address is hubserver
		// If so, send data to ethernet socket
		if(strcmp(inet_ntoa(incoming_connection_address.sin_addr), "192.168.1.190")) {
			printf("Thread #%d, sending data to ETHERNET FACING SOCKET\n", t_data->thread_id);
			send_datagram( *(t_data->socket), buf, recv_len, (struct sockaddr*) &(t_data->sendto_address2), slen);
		} else if (strcmp(inet_ntoa(incoming_connection_address.sin_addr), "192.168.2.1")) {
			printf("Thread #%d, sending data to HUBSERVER\n", t_data->thread_id);
			send_datagram( *(t_data->socket), buf, recv_len, (struct sockaddr*) &(t_data->sendto_address), slen);
		} else {
			printf("Thread #%d, [ERROR]??I don't know where to send this\n");
		}
		free(buf);
	}
}

/**
 * Listens on Ethernet facing socket
 */
void *ethernet_facing_udp_listener_thread(void *arg) {
	struct thread_data *t_data = arg;
	struct sockaddr_in incoming_connection_address;
	int recv_len;
	int slen = sizeof(incoming_connection_address);
	printf("Listening for data on Ethernet Socket [Thread #%d]...\n", t_data->thread_id);
	while(1) {
		char *buf = malloc(512*sizeof(char));
		fflush(stdin);
		recv_len = recvfrom(*(t_data->socket), buf, 512, 0, (struct sockaddr*) &incoming_connection_address, (unsigned int*) &slen);
		if(recv_len == -1) {
			printf("!===!\nError listening on Thread #%d\n!===!\n", t_data->thread_id);
			continue;
		}
		printf("Thread #%d Received: \"%X\"\n\tFrom: %s:%d\n", t_data->thread_id, buf, inet_ntoa(incoming_connection_address.sin_addr), ntohs(incoming_connection_address.sin_port));

		if(strcmp(inet_ntoa(incoming_connection_address.sin_addr), BROADCAST_IP) || strcmp(inet_ntoa(incoming_connection_address.sin_addr), "192.168.2.255")) {
			printf("Thread #%d, sending data to WIFI FACING SOCKET\n", t_data->thread_id);
			send_datagram( *(t_data->socket), buf, recv_len, (struct sockaddr*) &(t_data->sendto_address2), slen);
		} else if(strcmp(inet_ntoa(incoming_connection_address.sin_addr), "192.168.1.205")) {
			printf("Thread #%d, sending data to BROADCAST ADDRESS\n", t_data->thread_id);
			send_datagram( *(t_data->socket), buf, recv_len, (struct sockaddr*) &(t_data->sendto_address), slen);
		} else {
			printf("Thread #%d, [ERROR]??I don't know where to send this\n");
		}
		free(buf);
	}
}

void *ethernet_broadcast_listener_thread(void *arg) {
	struct thread_data *t_data = arg;
	struct sockaddr_in incoming_connection_address;
	int recv_len;
	int slen = sizeof(incoming_connection_address);
	printf("Listening for data on Ethernet Broadcast Socket [Thread #%d]...\n", t_data->thread_id);
	while(1) {
		char *buf = malloc(512*sizeof(char));
		fflush(stdin);
		recv_len = recvfrom(*(t_data->socket), buf, 512, 0, (struct sockaddr*) &incoming_connection_address, (unsigned int*) &slen);
		if(recv_len == -1) {
			printf("!===!\nError listening on Thread #%d\n!===!\n", t_data->thread_id);
			continue;
		}
		printf("Thread #%d Received: \"%X\"\n\tFrom: %s:%d\n", t_data->thread_id, buf, inet_ntoa(incoming_connection_address.sin_addr), ntohs(incoming_connection_address.sin_port));
		printf("Thread #%d, sending data to ETHERNET FACING SOCKET\n", t_data->thread_id);
		send_datagram( *(t_data->socket), buf, recv_len, (struct sockaddr*) &(t_data->sendto_address2), slen);
		free(buf);
	}
}

void create_udp_listener_thread(struct thread_data *t_data){
	if(pthread_create(&threads[0], NULL, wifi_facing_udp_listener_thread, &t_data[0]) != 0){
		printf("!==!\nError creating Thread #%d\n++++\n", t_data->thread_id);
		exit(EXIT_FAILURE);
	}
	if(pthread_create(&threads[1], NULL, ethernet_facing_udp_listener_thread, &t_data[1]) != 0){
		printf("!==!\nError creating Thread #%d\n++++\n", t_data->thread_id);
		exit(EXIT_FAILURE);
	}
	if(pthread_create(&threads[2], NULL, ethernet_broadcast_listener_thread, &t_data[1]) != 0){
		printf("!==!\nError creating Thread #%d\n++++\n", t_data->thread_id);
		exit(EXIT_FAILURE);
	}
}

void create_udp_socket(int *udp_socket, char *ipv4_address, int port){
	*udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(*udp_socket == -1){
		handle_error("socket");
	}
	int enable = 1;
	int sock_options = setsockopt(*udp_socket, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(int));
	if(sock_options < 0){
		printf("SOCKET OPTION ERROR\n");
		exit(EXIT_FAILURE);
	}
	sock_options = setsockopt(*udp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	if(sock_options < 0){
		printf("SOCKET OPTION ERROR\n");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ipv4_address);
	if(bind(*udp_socket, (struct sockaddr*) &addr, sizeof(addr)) == 0){
		printf("UDP Socket %s:%d was created successfully!\n", ipv4_address, port);
		return;
	}
	handle_error("bind");
}



int main(int argc, char *argv[]){
	/*
		Create 3 Sockets:
			1) Internet facing that handles communication to and from hub-server	[port 8080]
			2) Ethernet facing that handles BROADCAST packets						[port 3074]
			3) Ethernet facing that handles direct packets to Raspberry pi			[port 3074]
	
		Added 2 more sockets for testing purposes:
			1) Phaux socket sends data to the Xbox.
				Trying to see if having data sent from a in-network IP allows communication with Xbox
			2) uPnP socket.
				No clue what this actually is... It's called Universal Plug and Play that is for finding 
				devices on local networt. Just seeing what is sent to this port.
	*/
	char *rpi_ip_wifi = "192.168.1.205";
	char *rpi_ip_ethernet = "192.168.2.1";
	char *ethernet_broadcast_ip = "192.168.2.255";
	int wifi_faced_udp_socket;
	int ethernet_faced_udp_socket;
	int ethernet_broadcast_socket;

	create_udp_socket(&wifi_faced_udp_socket, rpi_ip_wifi, 8080);
	create_udp_socket(&ethernet_faced_udp_socket, rpi_ip_ethernet, 3074);
	create_udp_socket(&ethernet_broadcast_socket, ethernet_broadcast_ip, 3074);

	//setAddresses();
	HUBSERVER_ADDRESS.sin_family = AF_INET;
	HUBSERVER_ADDRESS.sin_port = htons(25565);
	HUBSERVER_ADDRESS.sin_addr.s_addr = inet_addr("192.168.1.190");


	struct sockaddr_in ETHERNET_FACED_ADDRESS;
	ETHERNET_FACED_ADDRESS.sin_family = AF_INET;
	ETHERNET_FACED_ADDRESS.sin_port = htons(3074);
	ETHERNET_FACED_ADDRESS.sin_addr.s_addr = inet_addr(rpi_ip_ethernet);

	struct sockaddr_in BROADCAST_ADDRESS;
	BROADCAST_ADDRESS.sin_family = AF_INET;
	BROADCAST_ADDRESS.sin_port = htons(3074);
	BROADCAST_ADDRESS.sin_addr.s_addr = inet_addr(BROADCAST_IP);

	struct sockaddr_in WIFI_FACED_ADDRESS;
	WIFI_FACED_ADDRESS.sin_family = AF_INET;
	WIFI_FACED_ADDRESS.sin_port = htons(8080);
	WIFI_FACED_ADDRESS.sin_addr.s_addr = inet_addr(rpi_ip_wifi);

	// Establish Communications with Hubserver
	setSocketToCommunicateWithHubServer(&wifi_faced_udp_socket);

	// Create threads for the sockets we just made
	struct thread_data t_data[NUM_THREADS - 1];

	t_data[0].socket = &wifi_faced_udp_socket;
	t_data[0].port_num = 8080;
	t_data[0].thread_id = 1;
	t_data[0].sendto_address = HUBSERVER_ADDRESS;
	t_data[0].sendto_address2 = ETHERNET_FACED_ADDRESS;

	t_data[1].socket = &ethernet_faced_udp_socket;
	t_data[1].port_num = 3074;
	t_data[1].thread_id = 2;
	t_data[1].sendto_address = BROADCAST_ADDRESS;
	t_data[1].sendto_address2 = WIFI_FACED_ADDRESS;

	t_data[2].socket = &ethernet_broadcast_socket;
	t_data[2].port_num = 3074;
	t_data[2].thread_id = 3;
	t_data[2].sendto_address = ETHERNET_FACED_ADDRESS;
	//t_data[2].sendto_address2 = ((void *)0); // null

	create_udp_listener_thread(t_data);


	// create_udp_listener_thread(&wifi_facing_8080_socket, &t_data[0]);
	// create_udp_listener_thread(&ethernet_facing_BROADCAST_socket, &t_data[1]);
	// create_udp_listener_thread(&ethernet_facing_BROADCAST_socket2, &t_data[2]);
	
	// create_udp_listener_thread(&rpi_router_socket, &t_data[3]);

	//create_udp_listener_thread(&phaux_address_socket, &t_data[3]);
	//create_udp_listener_thread(&uPnP_socket, &t_data[4]);


	// Below code will hopefully never run so Im just leaving as is just in case
	for(int i=0; i<NUM_THREADS; i++){
		if( pthread_join(*(threads+(i*sizeof(pthread_t))), NULL) != 0 ){
			printf("ERROR JOINING SOCKET #%d\n", i);
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
