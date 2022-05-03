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
	XBOX_ADDRESS.sin_family = AF_INET;
	XBOX_ADDRESS.sin_port = htons(3074);
	XBOX_ADDRESS.sin_addr.s_addr = inet_addr("0.0.0.1");//This is the broadcast address, 192.168.2.52 is the actual ip of the XBOX
	////////////////////////////////////////
	
	HUBSERVER_ADDRESS.sin_family = AF_INET;
	HUBSERVER_ADDRESS.sin_port = htons(25565);
	HUBSERVER_ADDRESS.sin_addr.s_addr = inet_addr("192.168.1.190"); //100.1.75.26 for Farm House // 100.8.130.221 is for external
	// Introduce server_socket and laptop_socket (hub server)
	char init_message[18] = "talk to me shawty";
	send_datagram(*server_socket, init_message, 18, (struct sockaddr*) &HUBSERVER_ADDRESS, slen);
	printf("Incoming traffic listener setup completed successfully!\n");
}

void *wifi_listener_thread(void *arg){
	struct thread_data *t_data = arg;
	char *buf = calloc(512, sizeof(char));
	struct sockaddr_in incoming_socket;
	int recv_len;
	int slen = sizeof(incoming_socket);
	while(1){
		printf("Waiting for data on Wifi Thread!\n");
		fflush(stdin);
		memset(buf, ' ', 512);
		recv_len = recvfrom(*(t_data->socket), buf, 512, 0, (struct sockaddr*) &incoming_socket, (unsigned int*) &slen);
		if( recv_len == -1 ){
			printf("ERROR LISTENING ON WIFI THREAD!\n");
			continue;
		}
		printf("Wifi thread received packet from %s:%d\nData: %s\n", inet_ntoa(incoming_socket.sin_addr), ntohs(incoming_socket.sin_port), buf);

		// WIFI -> R-PI -> XBOX
		// Using the broadcast address for now just so I dont need to worry about the specific IP of the XBOX
		printf("Sending data to xbox...\n");
		// send_datagram((int) *(t_data->socket), buf, recv_len, (struct sockaddr*) &XBOX_ADDRESS, slen);
		// HARDCODING
		send_datagram((int) *(t_data->socket), "h", 1, (struct sockaddr*) &XBOX_ADDRESS, slen );
		printf("Successfully sent data to Xbox!\n");
	}
}

// void *ethernet_listener_thread(void *arg){
// 	struct thread_data *t_data = arg;
// 	char *buf = calloc(512, sizeof(char));
// 	struct sockaddr_in incoming_socket;
// 	int recv_len;
// 	int slen = sizeof(incoming_socket);
// 	while(1){
// 		printf("Waiting for data on Ethernet Xbox Thread!\n");
// 		fflush(stdin);
// 		memset(buf, ' ', 512);
// 		recv_len = recvfrom(*(t_data->socket), buf, 512, 0, (struct sockaddr*) &incoming_socket, (unsigned int*) &slen);
// 		if( recv_len == -1){
// 			printf("ERROR LISTENING ON ETHERNET XBOX THREAD\n");
// 			continue;
// 		}
// 		printf("Ethernet thread received packet from %s:%d\nData: %s\n", inet_ntoa(incoming_socket.sin_addr), ntohs(incoming_socket.sin_port), buf);

// 		// Send Data out to HUBSERVER	[XBOX -> R-PI -> HUBSERVER]
// 		printf("Sending data to hubserver\n");
// 		send_datagram(*(t_data->socket), buf, recv_len, (struct sockaddr*) &HUBSERVER_ADDRESS, slen);
// 	}
// }

void *udp_listener_thread(void *arg){
	struct thread_data *t_data = arg;
	char *buf = calloc(512, sizeof(char));
	struct sockaddr_in incoming_connection_address;
	int recv_len;
	int slen = sizeof(incoming_connection_address);

	while(1){
		printf("Listening for data on Thread #%d...\n", t_data->thread_id);
		fflush(stdin);
		memset(buf, ' ', 512);
		recv_len = recvfrom(*(t_data->socket), buf, 512, 0, (struct sockaddr*) &incoming_connection_address, (unsigned int*) &slen);
		if(recv_len == -1){
			printf("!===!\nError listening on Thread #%d\n++++\n", t_data->thread_id);
			continue;
		}
		printf("Thread #%d Received: \"%s\"\n\tFrom: %s:%d\n", t_data->thread_id, buf, inet_ntoa(incoming_connection_address.sin_addr), ntohs(incoming_connection_address.sin_port));
		printf("Thread #%d: Sending data to %s:%d\n", t_data->thread_id, inet_ntoa(t_data->sendto_address.sin_addr), ntohs(t_data->sendto_address.sin_port));

		// TODO: Add sending logic
	}
}

// void create_listener_thread_eth(int *curr_socket, struct thread_data *t_data){
// 	if(pthread_create(&threads[THREAD_ID-1], NULL, ethernet_listener_thread, t_data) != 0){
// 		printf("Error Creating Thread for PORT: %d", t_data->port_num);
// 		exit(EXIT_FAILURE);
// 	}
// }

void create_udp_listener_thread(int *socket, struct thread_data *t_data){
	if(pthread_create(&threads[THREAD_ID - 1], NULL, udp_listener_thread, t_data) != 0){
		printf("!==!\nError creating Thread #%d\n++++\n", t_data->thread_id);
		exit(EXIT_FAILURE);
	}
}

void create_listener_thread_wifi(int *curr_socket, struct thread_data *t_data){
	// if(pthread_create(&threads[THREAD_ID-1], NULL, wifi_listener_thread, t_data) != 0){
	// 	printf("Error Creating Thread for PORT: %d", t_data->port_num);
	// 	exit(EXIT_FAILURE);
	// }
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

	char* rpi_ip = "192.168.1.205";			// TODO: Get these via function call
	char* broadcast_ip = "192.168.2.1";		// TODO: Get these via function call

	create_udp_socket(&wifi_facing_8080_socket, "192.168.1.205", 8080);	// This IP is the R-PI's
	create_udp_socket(&ethernet_facing_BROADCAST_socket, "192.168.2.255", 3074);		// This is the broadcast address so that we can broadcast packets from internet to xbox
	// create_udp_socket(&rpi_ethernet_DIRECT_socket, "192.168.1.205", 3074);
	create_udp_socket(&ethernet_facing_BROADCAST_socket2, BROADCAST_ADDRESS, 3074);

	// Establish Communications with Hubserver
	setSocketToCommunicateWithHubServer(&wifi_facing_8080_socket);

	// Create threads for the sockets we just made
	struct thread_data t_data[3];
	t_data[0].socket = &wifi_facing_8080_socket;
	t_data[0].port_num = 8080;
	t_data[0].thread_id = 1;
	t_data[0].sendto_address = XBOX_ADDRESS;
	t_data[1].socket = &ethernet_facing_BROADCAST_socket;
	t_data[1].port_num = 3074;
	t_data[1].thread_id = 2;
	t_data[1].sendto_address = HUBSERVER_ADDRESS;
	t_data[2].socket = &ethernet_facing_BROADCAST_socket2;
	t_data[2].port_num = 3074;
	t_data[2].thread_id = 3;
	t_data[2].sendto_address = HUBSERVER_ADDRESS;

	// create_listener_thread_wifi(&wifi_facing_8080_socket, &t_data[0]);
	// create_listener_thread_eth(&ethernet_facing_BROADCAST_socket, &t_data[1]);
	// create_listener_thread_eth(&ethernet_facing_BROADCAST_socket2, &t_data[2]);

	create_udp_listener_thread();

	 for(int i=0; i<NUM_THREADS; i++){
		if( pthread_join(*(threads+(i*sizeof(pthread_t))), NULL) != 0 ){
			printf("ERROR JOINING SOCKET #%d\n", i);
			exit(EXIT_FAILURE);
		}
	}

	/* OLD CODE BELOW */
	
	
	// Wifi socket listens on 8080 and communicates between the
	// ethernet socket and the router
	// Responsible for filtering LAN game packets and sending to hub-server
	// other wise itll just send it to the ethernet interface
	// int wifi_8080_socket; //Handles communication between hub-server and RPI
	// Xbox IP is 192.168.2.52 (use 'arp -a')

	//create_udp_socket(&ethernet_socket, BROADCAST_ADDRESS, 3074); // Xbox IP use arp -a
	// create_udp_socket(&wifi_8080_socket, "192.168.1.205", 8080);
	// setSocketToCommunicateWithHubServer(&wifi_8080_socket);
	
	
	//TODO: uncomment this
	//handle_threads(&ethernet_socket, &wifi_8080_socket);

	// THIS IS TEMPORARY //
	// int port_nums[6] = {88, 3074, 53, 500, 3544, 4500};
	// int udp_sockets[NUM_THREADS-1];
	// struct thread_data t_data[NUM_THREADS];
	// t_data[0].socket = &wifi_8080_socket;
	// t_data[0].thread_id = THREAD_ID;
	// t_data[0].port_num = 8080;
	// create_listener_thread_wifi(&wifi_8080_socket, &t_data[0]);
	// create_listener_thread_eth();
	// THREAD_ID+=1;
	/* for(int i=0; i<NUM_THREADS-1; i++){
		if(port_nums[i] == 3074)
			create_udp_socket(&udp_sockets[i], BROADCAST_ADDRESS, port_nums[i]);
		else
			create_udp_socket(&udp_sockets[i], "192.168.1.205", port_nums[i]);
		create_udp_socket(&udp_sockets[i], BROADCAST_ADDRESS, port_nums[i]);
		t_data[i+1].socket = &udp_sockets[i];
		t_data[i+1].thread_id = THREAD_ID;
		t_data[i+1].port_num = port_nums[i];
		create_listener_thread_eth(&udp_sockets[i], &t_data[i+1]);
		THREAD_ID+=1;
	} */ 
	// for(int i=0; i<NUM_THREADS; i++){
	// 	if( pthread_join(*(threads+(i*sizeof(pthread_t))), NULL) != 0 ){
	// 		printf("ERROR JOINING SOCKET #%d\n", i);
	// 		exit(EXIT_FAILURE);
	// 	}
	// }
	//////////////////////



	return 0;
}
