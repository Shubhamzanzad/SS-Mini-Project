#ifndef ADMIN_H
#define ADMIN_H

int adminLogin(int clientSocket, char userName[50], char password[50])
{
    struct Admin admin;
    int fd = open("files/adminCred", O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening Admin Credentials File");
        exit(EXIT_FAILURE);
    }
    int offset = lseek(fd, 0 * sizeof(struct Admin), SEEK_SET);
    if (offset != -1)
    {
        struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
        lock.l_pid = getpid();
        int f = fcntl(fd, F_SETLKW, &lock);
        if (f == -1)
        {
            perror("Error getting read lock for Admin Credentials");
            close(fd);
            return 0;
        }
        int readBytes = read(fd, &admin, sizeof(admin));
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        if (readBytes > 0)
        {
            if (strcmp(admin.userName, userName) != 0)
            {
                write(clientSocket, "Username is Incorrect~\n", 22);
                return 0;
            }
            else
            {
                if (strcmp(admin.password, password) != 0)
                {
                    write(clientSocket, "Password is Incorrect~\n", 22);
                    return 0;
                }
                else
                {
                    write(clientSocket, "Successful Authentication of Admin\nDisplaying Admin Menu$\n", 59);
                    return 1;
                }
            }
            return 0;
        }
    }
}

void adminHandler(int clientSocket)
{
    char dummy;
    memset(dummy, 0, 1);
    read(clientSocket, dummy, 1);
    if (strchr(dummy, '$') != NULL)
        continue;
    while (1)
    {
        write(clientSocket, ADMIN_MENU, 225);
        if (read(clientSocket, dummy, 1) == 0)
        {
            write(clientSocket, "No input try again~\n", 21);
            break;
        }
        if (resp[0] == '$')
        {
        }
        else if (!isnum(dummy))
        {
            write(clientSocket, "Wrong option entered~\n", 23);
            char skip[2];
            rd(clientSocket, skip, 2);
        }
        else
        {
            int choice = atoi(dummy);
            switch (choice)
            {
            case 1:
                addStudent(clientSocket);
                break;
            case 2:
                view(clientSocket, 1, -1);
                break;
            case 3:
                addProfessor(clientSocket);
                break;
            case 4:
                view(clientSocket, 2, -1);
                break;
            case 5:
                activate(clientSocket, 1);
                break;
            case 6:
                activate(clientSocket, 0);
                break;
            case 7:
                modify(clientSocket, 1);
                break;
            case 8:
                modify(clientSocket, 2);
                break;
            case 9:
                write(clientSocket, "Exiting...~\n", 13);
                return;
                break;
            default:
                printf("Invalid choice. Please select a valid option (1-9).\n");
            }
        }
    }
}

void addStudent(int clientSocket)
{
    int fd = open("files/student", O_RDWR | O_APPEND);
    if (fd == -1)
    {
        exit(1);
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(struct Student);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Student Record!");
        close(fd);
    }
    int id = set_count(1);
    struct Student st;
    char age[2], gen[2];
    sprintf(st.userName, "MT%d", id);
    wr(client_socket, "Enter Name: ", 13);
    rd(client_socket, st.name, sizeof(st.name));
    wr(client_socket, "Enter Email: ", 14);
    rd(client_socket, st.email, sizeof(st.email));
    wr(client_socket, "Enter Address: ", 16);
    rd(client_socket, st.address, sizeof(st.address));
    wr(client_socket, "Enter Gender: ", 15);
    rd(client_socket, gen, sizeof(gen));
    st.gender = gen[0];
    wr(client_socket, "Enter Age: ", 12);
    rd(client_socket, age, sizeof(age));
    st.age = atoi(age);
    st.rollNumber = id;
    st.isActive = true;
    strcpy(st.password, "iiitb");
    write(fd, &st, sizeof(struct Student));
    char tempBuffer[100];
    sprintf(tempBuffer, "\nSuccessfully added student\nNote: Login-id is MT{Student-ID}\nStudent-Id Generated is: %d...\n~", id);
    wr(client_socket, tempBuffer, strlen(tempBuffer) + 1);
    // Unlocking
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

#endif