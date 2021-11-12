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
#define NUM_THREADS 2

struct sockaddr_in ethernet_xbox_socket;
struct sockaddr_in hubserver_25565_socket;

struct thread_data {
	int *socket;
	int thread_id;
};

void incoming_traffic_listener_setup(int *server_socket){
	char buf[512];
	int recv_len, slen = sizeof(hubserver_25565_socket);
	ethernet_xbox_socket.sin_port = AF_INET;
	ethernet_xbox_socket.sin_port = htons(0);
	hubserver_25565_socket.sin_family = AF_INET;
	hubserver_25565_socket.sin_port = htons(25565);
	hubserver_25565_socket.sin_addr.s_addr = inet_addr("192.168.1.190"); //100.1.75.26 for Farm House // 100.8.130.221 is for external
	// Introduce server_socket and laptop_socket (hub server)
	char init_message[18] = "talk to me shawty";
	int bytes_sent = sendto(*server_socket, init_message, 18, 0, (const struct sockaddr*) &hubserver_25565_socket, slen);
	printf("%d", bytes_sent);
	if( bytes_sent == -1){
		printf("ERRROR SENDING INIT MESSAGE TO SERVER\n");
		exit(EXIT_FAILURE);
	}
	printf("Incoming traffic listener setup completed successfully!\n");
}

int send_datagram(int socket, char *buf, int recv_len, struct sockaddr* socket_addr, int slen){
	int send_to = sendto(socket, buf, recv_len, 0, socket_addr, slen);
	if(send_to == -1){
		//TODO: DELETE HANDLE_ERROR CALL
		handle_error("sendto");
		printf("ERROR SENDING DATA\n");
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

		send_to = send_datagram(*(t_data->socket), buf, recv_len, (struct sockaddr*) &ethernet_xbox_socket, slen);
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
		if(ntohs(ethernet_xbox_socket.sin_port) == 0){
			ethernet_xbox_socket.sin_port = incoming_socket.sin_port;
			ethernet_xbox_socket.sin_addr.s_addr = incoming_socket.sin_addr.s_addr;
		}
		// Send Data out to HUBSERVER	[XBOX -> R-PI -> HUBSERVER]
		send_to = send_datagram(*(t_data->socket), buf, recv_len, (struct sockaddr*) &hubserver_25565_socket, slen);
		if(send_to == -1){
			printf("ERROR SENDING DATA TO HUBSERVER\n");
			continue;
		}
	}
}

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

int main(int argc, char *argv[]){
	// Ethernet socket listens on port 3074 and hears communication
	// from Xbox and Wlan0
	int ethernet_socket;
	// Wifi socket listens on 8080 and communicates between the
	// ethernet socket and the router
	// Responsible for filtering LAN game packets and sending to hub-server
	// other wise itll just send it to the ethernet interface
	int wifi_8080_socket; //Handles communication between hub-server and RPI
	// Xbox IP is 192.168.2.52 (use 'arp -a')

	create_udp_socket(&ethernet_socket, BROADCAST_ADDRESS, 3074); // Xbox IP use arp -a
	create_udp_socket(&wifi_8080_socket, "192.168.1.205", 8080);
	incoming_traffic_listener_setup(&wifi_8080_socket);
	handle_threads(&ethernet_socket, &wifi_8080_socket);
	return 0;
}
