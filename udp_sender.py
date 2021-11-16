import socket

UDP_IP="192.168.1.190"
UDP_PORT=25565
MESSAGE='h'

print("UDP target IP: "+UDP_IP)
print("UDP target port: "+str(UDP_PORT))
print("message: "+MESSAGE)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(bytes(MESSAGE, "utf-8"), (UDP_IP, UDP_PORT))
