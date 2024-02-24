#ifndef STUDENT_H
#define STUDENT_H

int studentLogin(int clientSocket, char userName[50], char password[50], int *session)
{
    struct Student student;
    int fd = open("files/student", O_RDONLY);
    if (fd == -1)
    {
        perror("Student File opening error");
        return 0;
    }
    int id;
    if (sscanf(userName, "%d", &id) == 1)
    {
        int offset = lseek(fd, (id - 1) * sizeof(struct Student), SEEK_SET);
        if (offset != -1)
        {
            struct flock lock;
            lock.l_type = F_RDLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = (id - 1) * sizeof(struct Student);
            lock.l_len = 0;
            lock.l_pid = getpid();
            int f = fcntl(fd, F_SETLKW, &lock);
            if (f == -1)
            {
                perror("Error Obtaining Lock");
                close(fd);
                return 0;
            }
            int readBytes = read(fd, &student, sizeof(student));
            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            if (readBytes > 0)
            {
                if (getCount(1) + 1 > id)
                {
                    if (strcmp(student.userName, userName) != 0 || strcmp(student.password, password) != 0)
                    {
                        write(clientSocket, "Username or Password wrong.....~\n", 34);
                        return 0;
                    }
                    else if (strcmp(student.userName, userName) == 0 && strcmp(student.password, password) == 0)
                    {
                        if (student.isActive)
                        {
                            *session = id;
                            write(clientSocket, "Successfully Authenticated as Student....$\n", 44);
                            return 1;
                        }
                        else
                            write(clientSocket, "Your ID is Blocked.....~\n", 26);
                    }
                }
                else
                    write(clientSocket, "Username not found.....~\n", 26);
            }
            else
                write(clientSocket, "Username wrong.....~\n", 22);
        }
    }
    return 0;
}

#endif