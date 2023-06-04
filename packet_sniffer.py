import socket
import threading

MAX_BUFFER_SIZE = 65565

HUBSERVER_IP = "192.168.1.190"
HUBSERVER_PORT = 25565

BROADCAST_IP = "192.168.2S.255"
UDP_PORT = 3074

WIFI_IP = "192.168.1.204"
WIFI_PORT = 5000

ETHERNET_IP = "192.168.2.1"
ETHERNET_PORT = 9999

def hubserver_to_wifi_listener(wifi_socket, ethernet_socket):
    # Hubserver -> Local
    # For now this will handle the 'Searching' ping
    # If this is hit then it should be sent to broadcast ip
    while True:
        data, addr = wifi_socket.recvfrom(MAX_BUFFER_SIZE)
        print(f"[{WIFI_IP}:{WIFI_PORT}] received message: {data}")
        ethernet_socket.sendto(data, (BROADCAST_IP, UDP_PORT))
        print("Sent data from wifi -> ethernet -> broadcast")

def broadcast_listener(broadcast_socket):
    # Whatever -> broadcast
    # Assuming that it's just the search ping we can
    # send this directly to the hubserver
    # THIS IS GOING TO CHANGE
    while True:
        data, addr = broadcast_socket.recvfrom(MAX_BUFFER_SIZE)
        print("[255.255.255.255:3074] received message: %s" % data)
        broadcast_socket.sendto(data, (HUBSERVER_IP, HUBSERVER_PORT))
        print("Broadcast socket -> hubserver DONE")

if __name__ == "__main__":
    udp_sock_3074 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_sock_3074.bind((BROADCAST_IP, UDP_PORT))
    
    wifi_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    wifi_sock.bind((WIFI_IP, WIFI_PORT))
    
    ethernet_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ethernet_sock.bind((ETHERNET_IP, ETHERNET_PORT))
    ethernet_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    print("Creating Threads!")
    t1 = threading.Thread(target=broadcast_listener, args=(udp_sock_3074, ))
    t2 = threading.Thread(target=hubserver_to_wifi_listener, args=(wifi_sock, ethernet_sock,))
    t1.start()
    print('Thread 1 Started')
    t2.start()
    print('Thread 2 started!')