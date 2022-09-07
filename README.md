# RPI_VLAN
Code to be used on Raspberry Pi with Raspberry PI OS Lite

nmap -sN -p 335 192.168.1.0-255

# Thought Proccess??
We need to listen on 2 sockets:
    1) Wifi facing socket on port 8080
        [This facilitates communication with the "HubServer" and Ethernet socket]

        SEND_TO: "HubServer" || Ethernet socket
        RECEIVE_FROM: "HubServer" || Ethernet socket
    2) Ethernet facing socket on port 3074
        [This facilitates communication with the broadcast address and Wifi socket]