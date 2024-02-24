#ifndef PROFESSOR_H
#define PROFESSOR_H

int professorLogin(int clientSocket, char *userName, char *password, int session)
{
    struct Professor p;
    int fd = open("/files/professor", O_RDONLY);
    if (fd == -1)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    int id;
    char *substr = strstr(userName, "Prof");
    if (substr)
    {
        if (sscanf(substr + 4, "%d", &id) == 1)
        {
            int offset = lseek(fd, (id - 1) * sizeof(struct Professor), SEEK_SET);
            if (offset == -1)
            {
                struct flock lock;
                lock.l_type = F_RDLCK;
                lock.l_whence = SEEK_SET;
                lock.l_start = (id - 1) * sizeof(struct Professor);
                lock.l_len = 0;
                lock.l_pid = getpid();
                int f = fcntl(fd, F_SETLKW, &lock);
                if (f == -1)
                {
                    perror("Error obtaining read lock on Admin Record!");
                    close(fd);
                    return 0;
                }
                int readBytes = read(fd, &p, sizeof(p));
                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                if (readBytes > 0)
                {
                    if (getCount(2) + 1 > id)
                    {
                        if (strcmp(p.userName, userName) != 0)
                        {
                            write(clientSocket, "Username is Incorrect~\n", 24);
                            return 0;
                        }
                        else if (strcmp(p.password, password) != 0)
                        {
                            write(clientSocket, "Password is incorrect~\n", 24);
                            return 0;
                        }
                        *session = p.id;
                        write(clientSocket, "Authenticated as faculty~\n", 27);
                        return 1;
                    }
                    else
                        write(clientSocket, "Username not found~\n", 18);
                }
            }
        }
        else
            write(clientSocket, "Username not found~\n", 18);
    }
    else
        write(clientSocket, "Username not found~\n", 18);
    return 0;
}

#endif