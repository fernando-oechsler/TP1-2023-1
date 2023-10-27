#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

#define PORT 8080
#define MAX_WORKERS 2

struct Worker {
    int socket;
    bool isIdle;
};

struct Worker workers[MAX_WORKERS];
sem_t workerSemaphore;

void *handleClient(void *arg) {
    const char serverBusyMessage[] = "Sistema ocupado. Tente mais tarde.";
    char *clientBuffer = (char *)arg;
    int clientSocket = *((int *)(arg + 256));
    int workerIndex = -1;

    char *clientPosition = strstr(clientBuffer, "client");
    if (clientPosition != NULL) {
        int index = clientPosition - clientBuffer;
        char clientRequest[256];
        strcpy(clientRequest, clientBuffer + index + 7);

        sem_wait(&workerSemaphore);

        for (int i = 0; i < MAX_WORKERS; i++) {
            if (workers[i].isIdle) {
                workerIndex = i;
                workers[i].isIdle = false;
                break;
            }
        }

        sem_post(&workerSemaphore);

        if (workerIndex != -1) {
            write(workers[workerIndex].socket, clientRequest, strlen(clientRequest));

            char workerResponse[256];
            ssize_t responseLength = read(workers[workerIndex].socket, workerResponse, sizeof(workerResponse));
            if (responseLength > 0) {
                workerResponse[responseLength] = '\0';
                write(clientSocket, workerResponse, strlen(workerResponse));
            }

            sem_wait(&workerSemaphore);

            workers[workerIndex].isIdle = true;

            sem_post(&workerSemaphore);
        } else {
            write(clientSocket, serverBusyMessage, strlen(serverBusyMessage));
        }
    }

    free(clientBuffer);
    close(clientSocket);
    pthread_exit(NULL);
}

int main() {
    const char workerHelloMessage[] = "worker";
    const char clientHelloMessage[] = "client";
    const char quitMessage[] = "quit";

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Erro ao criar o socket do servidor");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Erro ao fazer bind");
        exit(1);
    }

    if (listen(serverSocket, 5) == 0) {
        printf("Aguardando por conexões...\n");
    } else {
        perror("Erro ao aguardar por conexões");
        exit(1);
    }

    sem_init(&workerSemaphore, 0, 1);

    for (int i = 0; i < MAX_WORKERS; i++) {
        workers[i].socket = -1;
        workers[i].isIdle = true;
    }

    while (1) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket < 0) {
            perror("Erro ao aceitar a conexão");
            continue;
        }

        char buffer[256];
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';

            if (strncmp(buffer, clientHelloMessage, strlen(clientHelloMessage)) == 0) {
                char *clientBuffer = malloc(512);
                strcpy(clientBuffer, buffer);
                memcpy(clientBuffer + 256, &clientSocket, sizeof(int));
                pthread_t thread;
                pthread_create(&thread, NULL, handleClient, clientBuffer);
            } else if (strncmp(buffer, workerHelloMessage, strlen(workerHelloMessage)) == 0) {
                int workerIndex = -1;

                sem_wait(&workerSemaphore);

                for (int i = 0; i < MAX_WORKERS; i++) {
                    if (workers[i].socket == -1) {
                        workers[i].socket = clientSocket;
                        workerIndex = i;
                        break;
                    }
                }

                sem_post(&workerSemaphore);

                if (workerIndex == -1) {
                    write(clientSocket, quitMessage, strlen(quitMessage));
                    close(clientSocket);
                }
            } else {
                printf("Conexão não identificada.\n");
                close(clientSocket);
            }
        }
    }

    sem_destroy(&workerSemaphore);
    close(serverSocket);

    return 0;
}

