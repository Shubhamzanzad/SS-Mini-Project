#ifndef LOGIN_H
#define LOGIN_H
#define MAX_MSG_SIZE 4096

int login(int clientSocket)
{
    char msg[MAX_MSG_SIZE];
    write(clientSocket, LOGIN_MENU, 110);
    memset(msg, 0, sizeof(msg));
    int bytesRecieved = read(clientSocket, msg, sizeof(msg));
    if (bytesRecieved <= 0)
    {
        return 0;
    }
    msg[bytesRecieved] = '\0';
    int res = atoi(msg);
    if (res < 1 || res > 3)
    {
        write(clientSocket, "Enter Correct Option\n", 22);
        return 0;
    }
    char userName[50], password[50];
    write(clientSocket, "Enter Username: ", 16);
    memset(userName, 0, sizeof(userName));
    bytesRecieved = read(clientSocket, userName, sizeof(userName));
    if (bytesRecieved <= 0)
    {
        return 0;
    }
    write(clientSocket, "Enter Password: ", 16);
    memset(password, 0, sizeof(password));
    bytesRecieved = read(clientSocket, password, sizeof(password));
    if (bytesRecieved <= 0)
    {
        return 0;
    }
    switch (res)
    {
    case 1:
        if (adminLogin(clientSocket, userName, password))
        {
            adminHandler(clientSocket);
            return 1;
        }
        else
            return 0;
        break;
    case 2:
        int profSession;
        if (professorLogin(clientSocket, userName, password, &profSession))
        {
            professorHandler(clientSocket);
            return 1;
        }
        else
            return 0;
        break;
    case 3:
        int studentSession;
        if (studentLogin(clientSocket, userName, password, &studentSession))
        {
            studentHandler(clientSocket);
            return 1;
        }
        else
            return 0;
        break;
    default:
        break;
    }
    return 0;
}
#endif