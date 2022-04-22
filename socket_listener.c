#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include </usr/include/arm-linux-gnueabihf/sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "error_handlers.h"
#include "socket_handlers.h"

#define BROADCAST_ADDRESS "255.255.255.255"
#define NUM_THREADS 3
// #define HUBSERVER_IP "192.168.1.205"

int THREAD_ID = 1;
struct sockaddr_in ethernet_xbox_socket;
struct sockaddr_in HUBSERVER_ADDRESS;
pthread_t threads[NUM_THREADS];

struct thread_data {
	int *socket;
	int thread_id;
	int port_num;
};

void setSocketToCommunicateWithHubServer(int *server_socket){
	char buf[512];
	int recv_len, slen = sizeof(HUBSERVER_ADDRESS);

	// PLEASE MOVE THIS CODE BLOCK ELSEWHERE
	ethernet_xbox_socket.sin_family = AF_INET;
	ethernet_xbox_socket.sin_port = htons(3074);
	ethernet_xbox_socket.sin_addr.s_addr = inet_addr("192.168.2.1");//This is the broadcast address, 192.168.2.52 is the actual ip of the XBOX
	////////////////////////////////////////
	
	HUBSERVER_ADDRESS.sin_family = AF_INET;
	HUBSERVER_ADDRESS.sin_port = htons(25565);
	HUBSERVER_ADDRESS.sin_addr.s_addr = inet_addr("192.168.1.190"); //100.1.75.26 for Farm House // 100.8.130.221 is for external
	// Introduce server_socket and laptop_socket (hub server)
	char init_message[18] = "talk to me shawty";
	int bytes_sent = send_datagram(*server_socket, init_message, 18, (const struct sockaddr*) &HUBSERVER_ADDRESS, slen);
	// int bytes_sent = sendto(*server_socket, init_message, 18, 0, (const struct sockaddr*) &HUBSERVER_ADDRESS, slen);
	printf("%d", bytes_sent);
	if( bytes_sent == -1) {
		printf("ERRROR SENDING INIT MESSAGE TO SERVER, ERROR MSG: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("Incoming traffic listener setup completed successfully!\n");
}

int send_datagram(int socket, char *buf, size_t recv_len, struct sockaddr* to, int slen){
	
	int send_to = sendto(socket, buf, recv_len, 0, to, slen);
	if(send_to == -1){
		//TODO: DELETE HANDLE_ERROR CALL
		handle_error("sendto");
		printf("ERROR SENDING DATA: %s\n", strerror(errno));
		return -1;
	}
	return 1;
}

void *wifi_listener_thread(void *arg){
	struct thread_data *t_data = arg;
	char *buf = calloc(512, sizeof(char));
	struct sockaddr_in incoming_socket;
	int recv_len, send_to;
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

		send_to = send_datagram((int) *(t_data->socket), buf, recv_len, (struct sockaddr*) &ethernet_xbox_socket, slen);
		if(send_to == -1){
			printf("ERROR SENDING DATA TO XBOX\n");
			continue;
		}
		printf("Successfully sent data to Xbox!\n");
	}
}

void *ethernet_listener_thread(void *arg){
	struct thread_data *t_data = arg;
	char *buf = calloc(512, sizeof(char));
	struct sockaddr_in incoming_socket;
	int recv_len, send_to;
	int slen = sizeof(incoming_socket);
	while(1){
		printf("Waiting for data on Ethernet Xbox Thread!\n");
		fflush(stdin);
		memset(buf, ' ', 512);
		recv_len = recvfrom(*(t_data->socket), buf, 512, 0, (struct sockaddr*) &incoming_socket, (unsigned int*) &slen);
		if( recv_len == -1){
			printf("ERROR LISTENING ON ETHERNET XBOX THREAD\n");
			continue;
		}
		printf("Ethernet thread received packet from %s:%d\nData: %s\n", inet_ntoa(incoming_socket.sin_addr), ntohs(incoming_socket.sin_port), buf);

		// Send Data out to HUBSERVER	[XBOX -> R-PI -> HUBSERVER]
		send_to = send_datagram(*(t_data->socket), buf, recv_len, (struct sockaddr*) &HUBSERVER_ADDRESS, slen);
		if(send_to == -1){
			printf("ERROR SENDING DATA TO HUBSERVER\n");
			continue;
		}
	}
}

/* NOT BEING USED */
void handle_threads(int *ethernet_socket, int *wifi_8080_socket){
	pthread_t threads[2];
	struct thread_data t_data[2];

	t_data[0].socket = ethernet_socket;
	t_data[0].thread_id = 1;
	t_data[1].socket = wifi_8080_socket;
	t_data[1].thread_id = 2;

	if( pthread_create(&threads[0], NULL, ethernet_listener_thread, &t_data[0]) != 0 ){
		printf("ERROR CREATING XBOX ETHERNET THREAD!\n");
		exit(EXIT_FAILURE);
	}

	if( pthread_create(&threads[1], NULL, wifi_listener_thread, &t_data[1]) != 0 ){
		printf("ERROR CREATING HUBSERVER WIFI THREAD!\n");
		exit(EXIT_FAILURE);
	}

	for(int i=0; i<NUM_THREADS; i++){
		if( pthread_join(*(threads+(i*sizeof(pthread_t))), NULL) != 0 ){
			printf("ERROR JOINING SOCKET #%d\n", i);
			exit(EXIT_FAILURE);
		}
	}
	printf("Finished joining threads\n");
}


void create_listener_thread_eth(int *curr_socket, struct thread_data *t_data){
	if(pthread_create(&threads[THREAD_ID-1], NULL, ethernet_listener_thread, t_data) != 0){
		printf("Error Creating Thread for PORT: %d", t_data->port_num);
		exit(EXIT_FAILURE);
	}
}

void create_listener_thread_wifi(int *curr_socket, struct thread_data *t_data){
	if(pthread_create(&threads[THREAD_ID-1], NULL, wifi_listener_thread, t_data) != 0){
		printf("Error Creating Thread for PORT: %d", t_data->port_num);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]){
	/*
		Create 3 Sockets:
			1) Internet facing that handles communication to and from hub-server	[port 8080]
			2) Ethernet facing that handles BROADCAST packets						[port 3074]
			3) Ethernet facing that handles direct packets to Raspberry pi			[port 3074]
	*/
	int internet_to_rpi_bridge_socket;
	int rpi_ethernet_BROADCAST_socket;
	int rpi_ethernet_DIRECT_socket;

	char* rpi_ip = "192.168.1.205";			// TODO: Get these via function call
	char* broadcast_ip = "192.168.2.1";		// TODO: Get these via function call

	create_udp_socket(&internet_to_rpi_bridge_socket, "192.168.1.205", 8080);	// This IP is the R-PI's
	create_udp_socket(&rpi_ethernet_BROADCAST_socket, "192.168.2.1", 3074);		// This is the broadcast address so that we can broadcast packets from internet to xbox
	create_udp_socket(&rpi_ethernet_DIRECT_socket, "192.168.1.205", 3074);

	struct thread_data t_data[2];
	t_data[0].socket = &internet_to_rpi_bridge_socket;
	t_data[0].port_num = 8080;
	t_data[0].thread_id = 1;
	t_data[1].socket = &rpi_ethernet_BROADCAST_socket;
	t_data[1].port_num = 3074;
	t_data[1].thread_id = 2;
	t_data[2].socket = &rpi_ethernet_DIRECT_socket;
	t_data[2].port_num = 3074;
	t_data[2].thread_id = 3;

	// Establish Communications with Hubserver
	setSocketToCommunicateWithHubServer(&internet_to_rpi_bridge_socket);

	// Create threads for the sockets we just made
	create_listener_thread_wifi(&internet_to_rpi_bridge_socket, &t_data[0]);
	create_listener_thread_eth(&rpi_ethernet_BROADCAST_socket, &t_data[1]);
	create_listener_thread_eth(&rpi_ethernet_DIRECT_socket, &t_data[2]);

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
