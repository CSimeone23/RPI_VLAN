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

#define BROADCAST_ADDRESS "255.255.255.255"
#define NUM_THREADS 3
// #define HUBSERVER_IP "192.168.1.205"

int THREAD_ID = 1;
struct sockaddr_in XBOX_ADDRESS;
struct sockaddr_in HUBSERVER_ADDRESS;

/* TEMP */
struct sockaddr_in TEMP_DIRECT_XBOX_ADDRESS;
/* **** */

pthread_t threads[NUM_THREADS];

struct thread_data {
	int *socket;
	int thread_id;
	int port_num;
	struct sockaddr_in sendto_address;
};


void send_datagram(int socket, char *buf, size_t recv_len, struct sockaddr* to, int slen){
	int send_to = sendto(socket, buf, recv_len, 0, to, slen);
	if(send_to == -1){
		handle_error("sendto");
	}
}

void setSocketToCommunicateWithHubServer(int *server_socket){
	char buf[512];
	int recv_len, slen = sizeof(HUBSERVER_ADDRESS);

	// PLEASE MOVE THIS CODE BLOCK ELSEWHERE
	TEMP_DIRECT_XBOX_ADDRESS.sin_family = AF_INET;
	TEMP_DIRECT_XBOX_ADDRESS.sin_port = htons(3074);
	TEMP_DIRECT_XBOX_ADDRESS.sin_addr.s_addr = inet_addr("192.168.2.52"); //Actual xbox IP
	////////////////////////////////////////

	XBOX_ADDRESS.sin_family = AF_INET;
	XBOX_ADDRESS.sin_port = htons(3074);
	XBOX_ADDRESS.sin_addr.s_addr = inet_addr("255.255.255.255");//This is the broadcast address, 192.168.2.52 is the actual ip of the XBOX
	////////////////////////////////////////
	
	HUBSERVER_ADDRESS.sin_family = AF_INET;
	HUBSERVER_ADDRESS.sin_port = htons(25565);
	HUBSERVER_ADDRESS.sin_addr.s_addr = inet_addr("192.168.1.190"); //100.1.75.26 for Farm House // 100.8.130.221 is for external
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

void *udp_listener_thread(void *arg){
	struct thread_data *t_data = arg;
	struct sockaddr_in incoming_connection_address;
	int recv_len;
	int slen = sizeof(incoming_connection_address);

	while(1){
		printf("Listening for data on Thread #%d...\n", t_data->thread_id);
		//char *buf = malloc(512*sizeof(char));
		int *buf = malloc(512*sizeof(int));
		fflush(stdin);
		// memset(buf, ' ', 512);
		recv_len = recvfrom(*(t_data->socket), buf, 512, 0, (struct sockaddr*) &incoming_connection_address, (unsigned int*) &slen);
		if(recv_len == -1){
			printf("!===!\nError listening on Thread #%d\n++++\n", t_data->thread_id);
			continue;
		}
		printf("Thread #%d Received: \"%s\"\n\tFrom: %s:%d\n", t_data->thread_id, buf, inet_ntoa(incoming_connection_address.sin_addr), ntohs(incoming_connection_address.sin_port));
		/* TEMP */
		if(strcmp(inet_ntoa(incoming_connection_address.sin_addr), "0.0.0.1") == 0){
			printf("\nRecv Length = %d\n", recv_len);
			//write_data_to_file(buf);
		}
		// Make sure we dont get stuck in a loop
		if(strcmp(inet_ntoa(incoming_connection_address.sin_addr), "192.168.2.1") == 0 && t_data->thread_id == 3){
			printf("Thread #3: Preventing Infinite Loop\n");
			printf("Thread #3: Sending it directly to the Xbox??\n");
			//send_datagram( *(t_data->socket), buf, recv_len, (struct sockaddr*) &TEMP_DIRECT_XBOX_ADDRESS, slen);
			free(buf);
			continue;
		}
		printf("Thread #%d: Sending data to %s:%d\n", t_data->thread_id, inet_ntoa(t_data->sendto_address.sin_addr), ntohs(t_data->sendto_address.sin_port));
		//send_datagram( *(t_data->socket), buf, recv_len, (struct sockaddr*) &(t_data->sendto_address), slen);
		free(buf);
	}
}

void create_udp_listener_thread(int *socket, struct thread_data *t_data){
	if(pthread_create(&threads[THREAD_ID - 1], NULL, udp_listener_thread, t_data) != 0){
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
	*/

	int wifi_facing_8080_socket;
	int ethernet_facing_BROADCAST_socket;
	int ethernet_facing_BROADCAST_socket2;

	char* rpi_ip = "192.168.1.205";				// TODO: Get these via function call
	char* broadcast_ip = "192.168.2.255";		// TODO: Get these via function call

	create_udp_socket(&wifi_facing_8080_socket, "192.168.1.205", 8080);	// This IP is the R-PI's
	create_udp_socket(&ethernet_facing_BROADCAST_socket, "192.168.2.255", 3074);		// This is the broadcast address so that we can broadcast packets from internet to xbox
	create_udp_socket(&ethernet_facing_BROADCAST_socket2, BROADCAST_ADDRESS, 3074);

	// This socket will receive from wifi and send to xbox/broadcast 
	// This socket will be on the same network as the Xbox so it'll think its another console
	int phaux_address_socket;
	create_udp_socket(&phaux_address_socket, "192.168.2.1", 3074);

	struct sockaddr_in PHAUX_ADDRESS;
	PHAUX_ADDRESS.sin_family = AF_INET;
	PHAUX_ADDRESS.sin_port = htons(3074);
	PHAUX_ADDRESS.sin_addr.s_addr = inet_addr("192.168.2.1");


 
	// Establish Communications with Hubserver
	setSocketToCommunicateWithHubServer(&wifi_facing_8080_socket);

	// Create threads for the sockets we just made
	struct thread_data t_data[4];

	t_data[0].socket = &wifi_facing_8080_socket;
	t_data[0].port_num = 8080;
	t_data[0].thread_id = 1;
	t_data[0].sendto_address = PHAUX_ADDRESS;// XBOX_ADDRESS;

	t_data[1].socket = &ethernet_facing_BROADCAST_socket;
	t_data[1].port_num = 3074;
	t_data[1].thread_id = 2;
	t_data[1].sendto_address = HUBSERVER_ADDRESS;

	t_data[2].socket = &ethernet_facing_BROADCAST_socket2;
	t_data[2].port_num = 3074;
	t_data[2].thread_id = 3;
	t_data[2].sendto_address = HUBSERVER_ADDRESS;

	t_data[3].socket = &phaux_address_socket;
	t_data[3].port_num = 3074;
	t_data[3].thread_id = 4;
	t_data[3].sendto_address = XBOX_ADDRESS;

	// create_listener_thread_wifi(&wifi_facing_8080_socket, &t_data[0]);
	// create_listener_thread_eth(&ethernet_facing_BROADCAST_socket, &t_data[1]);
	// create_listener_thread_eth(&ethernet_facing_BROADCAST_socket2, &t_data[2]);

	create_udp_listener_thread(&wifi_facing_8080_socket, &t_data[0]);
	create_udp_listener_thread(&ethernet_facing_BROADCAST_socket, &t_data[1]);
	create_udp_listener_thread(&ethernet_facing_BROADCAST_socket2, &t_data[2]);

	create_udp_listener_thread(&phaux_address_socket, &t_data[3]);

	 for(int i=0; i<NUM_THREADS; i++){
		if( pthread_join(*(threads+(i*sizeof(pthread_t))), NULL) != 0 ){
			printf("ERROR JOINING SOCKET #%d\n", i);
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
