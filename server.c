#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <ctype.h>

#include "structures.h"
#include "functions/admin.h"
#include "functions/login.h"
#include "functions/student.h"

#define MAX_CLIENTS 1000
#define MAX_MSG_SIZE 4096

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char mag[MAX_MSG_SIZE];
    int clientCount = 0;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        perror("Socker creation error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9008);

    if (bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1)
    {
        perror("Error Listening");
        exit(EXIT_FAILURE);
    }
    printf("Server Listening on port 9008");

    while (1)
    {
        clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (clientSocket == -1)
        {
            perror("Client Connection Failed");
            continue;
        }
        pid_t childPID = fork();
        if (childPID == -1)
        {
            perror("Fork Failed");
            continue;
        }
        if (childPID == 0)
        {
            close(serverSocket);
            if (login(clientSocket) == 0)
            {
                exit(EXIT_FAILURE);
            }
            else
            {
                exit(EXIT_SUCCESS);
            }
        }
        else
        {
            close(clientSocket);
            clientCount++;
            if (clientCount >= MAX_CLIENTS)
            {
                printf("Maximum number of client connections reached.\nRejecting new connections");
                close(clientSocket);
            }
        }
    }
    close(serverSocket);
    return 0;
}