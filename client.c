#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <stdio_ext.h>

void *receive_messages(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[1024];
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            printf("Server disconnected.\n");
            break;
        }
        buffer[bytes_received] = '\0'; // Ensure null-terminated string
		printf("\033[2K\r"); // Clear current line and move cursor to the beginning
        // Extract sender's username and message content
        char *sender_username = strtok(buffer, ":");
        char *message_content = strtok(NULL, "\n");

        // Display the received message without the "Received:" prefix
        printf("%s: %s\n", sender_username, message_content);
        
        printf("Enter a message to send (or 'quit' to exit): ");
        fflush(stdout);
    
    }
    pthread_exit(NULL);
}
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <Game Type> <Server Name> <Port Number>\n", argv[0]);
        return 1;
    }

    char *game_type = argv[1];
    char *server_name = argv[2];
    int port = atoi(argv[3]);

    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Connect to the server
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(server_name);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        close(client_socket);
        return 1;
    }

    printf("Connected to the server.\n");
    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        printf("Server disconnected.\n");
    }

    buffer[bytes_received] = '\0'; // Null-terminate the received data
    printf("Server: %s", buffer);

    // Communication with the server
    char username[50];  // Add a username for identification
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0';  // Remove the newline character
    
	pthread_t receive_thread;
	pthread_create(&receive_thread, NULL, receive_messages, (void *)&client_socket);
	pthread_detach(receive_thread);
		
    char message[1024];
    while (1) {
		
		
 // Clear current line
        printf("Enter a message to send (or 'quit' to exit): ");
        fgets(message, sizeof(message), stdin);
        printf("\033[1A");  // Move cursor up one line
        printf("\033[2K");
        

        if (strcmp(message, "quit\n") == 0) {
            // Send quit message to the server
            send(client_socket, "QUIT", 4, 0);
            break;
        }

        // Send the message along with the username
        size_t full_message_size = strlen(username) + strlen(message) + 3;  // Username + ": " + message
        char *full_message = (char *)malloc(full_message_size);
        snprintf(full_message, full_message_size, "%s: %s", username, message);
        send(client_socket, full_message, strlen(full_message), 0);
        printf("%s: %s", username, message);
        free(full_message); 
        
    }

    close(client_socket);

    return 0;
}

