import socket
import threading

MAX_BUFFER_SIZE = 65565

HUBSERVER_IP = "192.168.1.190"
HUBSERVER_PORT = 25565

BROADCAST_IP = "192.168.2S.255"
UDP_PORT = 3074

LOCAL_ETHERNET_IP = "192.168.2.1"
LOCALHOST_PORT = 5000

def hubserver_to_local_listener(local_socket):
    # Hubserver -> Local
    # For now this will handle the 'Searching' ping
    # If this is hit then it should be sent to broadcast ip
    while True:
        data, addr = local_socket.recvfrom(MAX_BUFFER_SIZE)
        print("[LOCAL:5000] received message: %s" % data)
        local_socket.sendto(data, (BROADCAST_IP, UDP_PORT))
        print("Sent data from local -> broadcast")

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
    
    local_ethernet_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print("Creating Threads!")
    t1 = threading.Thread(target=broadcast_listener, args=(udp_sock_3074, ))
    t2 = threading.Thread(target=hubserver_to_local_listener, args=(local_ethernet_sock, ))
    t1.start()
    print('Thread 1 Started')
    t2.start()
    print('Thread 2 started!')