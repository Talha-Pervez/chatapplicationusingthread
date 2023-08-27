#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10

struct Client {
    int socket;
    char username[50];
};

struct Client clients[MAX_CLIENTS];

void *handle_client(void *arg) {
    int client_index = *((int *)arg);
    int client_socket = clients[client_index].socket;

    char buffer[1024];
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            printf("Client %s disconnected.\n", clients[client_index].username);
            close(client_socket);
            clients[client_index].socket = -1;
            break;
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (i != client_index && clients[i].socket != -1) {
                send(clients[i].socket, buffer, bytes_received, 0);
            }
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port Number>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    // Initialize the clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = -1;
    }

    // Accept and handle incoming connections
    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        int client_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket == -1) {
                client_index = i;
                break;
            }
        }

        if (client_index == -1) {
            // Maximum clients reached
            close(client_socket);
            continue;
        }
	
	char welcome_message[] = "Welcome to the chat server!\n";
    	send(client_socket, welcome_message, sizeof(welcome_message), 0);
    	
        clients[client_index].socket = client_socket;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, &client_index);
        pthread_detach(client_thread);

        printf("Client connected. Thread created for client %d.\n", client_index);
    }

    close(server_socket);

    return 0;
}

