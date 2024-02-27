#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#define MAX_MSG_SIZE 4096

void connectServer(int clientSocket);

int main()
{
    int clientSocket;
    struct sockaddr_in server_addr;
    char buff[MAX_MSG_SIZE];

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        perror("Socket Connection Failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9008);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(clientSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server\n");
    connectServer(clientSocket);
    close(clientSocket);
    return 0;
}

void connectServer(int clientSocket)
{
    char msg[MAX_MSG_SIZE], temp[MAX_MSG_SIZE];
    int writeBytes, bytesReceived, len;
    while (1)
    {
        memset(msg, 0, sizeof(msg));
        memset(temp, 0, sizeof(temp));
        bytesReceived = read(clientSocket, msg, sizeof(msg));

        if (bytesReceived <= 0)
        {
            perror("Error while reading client socket");
            close(clientSocket);
            return;
        }
        else if (strchr(msg, '~') != NULL)
        {
            // Delimiter to close connection
            strncpy(temp, msg, strlen(msg) - 1);
            printf("%s\n", temp);
            fflush(stdout);
            printf("Closing connection\n");
            break;
        }
        else if (strchr(msg, '$') != NULL)
        {
            // Delimiter to skip input
            strncpy(temp, msg, strlen(msg) - 1);
            printf("%s\n", temp);
            fflush(stdout);
            printf("\n");
            char skip = '$';
            writeBytes = write(clientSocket, &skip, 1);
            if (writeBytes == -1)
            {
                perror("Error while writing to server");
                return;
            }
        }
        else
        {
            char writeBuffer[MAX_MSG_SIZE];
            memset(writeBuffer, 0, sizeof(writeBuffer));
            if (strchr(msg, '@'))
            {
                strncpy(temp, msg, strlen(msg) - 1);
                printf("%s", temp);
                fflush(stdout);
                char password[100];
                strcpy(writeBuffer, getpass(password));
            }
            else
            {
                printf("%s", msg);
                fflush(stdout);
                fgets(writeBuffer, sizeof(writeBuffer), stdin);
                writeBuffer[strcspn(writeBuffer, "\n")] = 0; // Remove newline character
            }
            writeBytes = write(clientSocket, writeBuffer, strlen(writeBuffer));
            if (writeBytes <= 0)
            {
                perror("Error while writing to server");
                exit(EXIT_FAILURE);
            }
        }
    }
}
