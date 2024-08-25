#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define LOG_FILE "ping_log.txt"

// print the server log in the file
void log_ping_request(struct sockaddr_in client_address, double rtt) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL) {
        perror("Failed to open log file");
        exit(1);
    }
    fprintf(log, "Source IP: %s, Port: %d, RTT: %f ms\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), rtt);
    fclose(log);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <ip address> <port>\n", argv[0]);
        exit(1);
    }

    int server_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t add_len = sizeof(client_address);
    char buffer[BUFFER_SIZE];

    // Create socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("[-]Error creating server socket");
        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    // Set up server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = atoi(argv[2]);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("[-]Error binding server socket");
        close(server_socket);
        exit(1);
    }
    printf("[+]Bind to the port number: %d\n", atoi(argv[2]));
    printf("[+]Listening...\n");

    while (1) {
        ssize_t recv_len = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_address, &add_len);
        if (recv_len == -1) {
            perror("[-]Error receiving data");
            continue;
        }

        buffer[recv_len] = '\0'; // Ensure null-terminated string
        printf("[+]Received data from %s (%d)\n %s\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), buffer);

        // Record the time when the ping request was received
        struct timeval receive_time;
        gettimeofday(&receive_time, NULL);

        // Send acknowledgment with the same payload
        if (sendto(server_socket, buffer, recv_len, 0, (struct sockaddr *)&client_address, add_len) == -1) {
            perror("[-]Error sending acknowledgment");
            continue;
        }

        // Calculate RTT
        struct timeval send_time;
        gettimeofday(&send_time, NULL);
        double rtt = (send_time.tv_sec - receive_time.tv_sec) * 1000.0 + (send_time.tv_usec - receive_time.tv_usec) / 1000.0;

        // Log the ping request and RTT
        log_ping_request(client_address, rtt);
    }
    // close server
    close(server_socket);
    return 0;
}
