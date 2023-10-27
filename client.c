#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <operação> <número1> <número2>\n", argv[0]);
        return 1;
    }

    int client_socket;
    struct sockaddr_in server_addr;

    /* Create socket */
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }

    /* Configure server address */ 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Connect to the server */
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    /* Prepare a message for the server */
    char message[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "client %s %s %s", argv[1], argv[2], argv[3]);

    /* Send the message to the server */
    send(client_socket, message, strlen(message), 0);

    /* Receive and print the result from the server */
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(client_socket, buffer, BUFFER_SIZE, 0) < 0) {
        perror("Error receiving message");
    } else {
        printf("Received result from server: %s\n", buffer);
    }

    /* Close the socket */
    close(client_socket);

    return 0;
}

