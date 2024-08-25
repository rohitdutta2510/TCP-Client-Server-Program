#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024

void ping_server(int client_socket, struct sockaddr_in server_address, int ping_num){
    char buffer[BUFFER_SIZE];
    struct timeval send_time, receive_time;
    int send;

    gettimeofday(&send_time, NULL); // get the ping sending time
    sprintf(buffer, "Ping request %d", ping_num); // create the payload to send
    // Send ping request to the server
    send = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_address, sizeof(server_address));
    if (send == -1) { 
        perror("[-]Error sending ping request");
        return;
    }

    // Receive acknowledgment
    ssize_t recv_len = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (recv_len == -1) {
        perror("[-]Error receiving acknowledgment");
        return;
    }

    gettimeofday(&receive_time, NULL); // get the acknowledgment received time
    double rtt = (receive_time.tv_sec - send_time.tv_sec) * 1000.0 + (receive_time.tv_usec - send_time.tv_usec) / 1000.0;

    printf("[+]Received Ack: %s, RTT: %f ms\n", buffer, rtt);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <server_ip> <port> <num_requests> <interval_ms>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int num_requests = atoi(argv[3]);
    int interval_ms = atoi(argv[4]);

    int client_socket;
    struct sockaddr_in server_address;

    // Create socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Error creating client socket");
        exit(1);
    }

    // Set up server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = server_port;

    for (int i = 0; i < num_requests; i++) {
        ping_server(client_socket, server_address, i);

        if (i < num_requests - 1) {
            usleep(interval_ms * 1000); // interval between cosecutive pings
        }
    }

    close(client_socket);
    return 0;
}
