#ifndef ADMIN_H
#define ADMIN_H

void addStudent(int clientSocket);
void addProfessor(int clientSocket);
void view(int clientSocket, int type, int ID);
void activate(int clientSocket, int status);
void modify(int clientSocket, int type);

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
    char dummy[4];
    memset(dummy, 0, sizeof(dummy));
    read(clientSocket, dummy, sizeof(dummy));
    if (strchr(dummy, '$') != NULL)
    {
    }
    while (1)
    {
        write(clientSocket, ADMIN_MENU, 224);
        memset(dummy, 0, sizeof(dummy));
        if (read(clientSocket, dummy, sizeof(dummy)) == 0)
        {
            write(clientSocket, "No input try again~\n", 21);
            break;
        }
        if (dummy[0] == '$')
        {
        }
        else if (!isalnum(dummy))
        {
            write(clientSocket, "Wrong option entered~\n", 23);
            char skip[2];
            read(clientSocket, skip, 2);
        }
        else
        {
            bool validInput = true;
            for (int i = 0; i < sizeof(dummy); i++)
            {
                if (dummy[i] == '\0')
                    break;
                if (!isalnum(dummy[i]) && !isspace(dummy[i]))
                {
                    validInput = false;
                    break;
                }
            }

            if (!validInput)
            {
                write(clientSocket, "Wrong option entered~\n", 23);
                char skip[2];
                read(clientSocket, skip, 2);
            }
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
    int id = setCount(1);
    struct Student st;
    char age[2], gen[2];
    sprintf(st.userName, "MT%d", id);
    write(clientSocket, "Enter Name: ", 13);
    read(clientSocket, st.name, sizeof(st.name));
    write(clientSocket, "Enter Email: ", 14);
    read(clientSocket, st.email, sizeof(st.email));
    write(clientSocket, "Enter Address: ", 16);
    read(clientSocket, st.address, sizeof(st.address));
    write(clientSocket, "Enter Gender: ", 15);
    read(clientSocket, gen, sizeof(gen));
    st.gender = gen[0];
    write(clientSocket, "Enter Age: ", 12);
    read(clientSocket, age, sizeof(age));
    st.age = atoi(age);
    st.rollNumber = id;
    st.isActive = true;
    strcpy(st.password, "iiitb");
    write(fd, &st, sizeof(struct Student));
    char tempBuffer[100];
    sprintf(tempBuffer, "\nSuccessfully added student\nNote: Login-id is MT{Student-ID}\nStudent-Id Generated is: %d...\n~", id);
    write(clientSocket, tempBuffer, strlen(tempBuffer) + 1);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void addProfessor(int clientSocket)
{
    int fd = open("files/professor", O_RDWR | O_APPEND);
    if (fd == -1)
    {
        exit(1);
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(struct Professor);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Professor Record!");
        close(fd);
    }
    int id = setCount(2);
    struct Professor st;
    char gender[2];
    sprintf(st.userName, "MT%d", id);
    write(clientSocket, "Enter Name: ", 13);
    read(clientSocket, st.name, sizeof(st.name));
    write(clientSocket, "Enter Email: ", 14);
    read(clientSocket, st.email, sizeof(st.email));
    write(clientSocket, "Enter Address: ", 16);
    read(clientSocket, st.address, sizeof(st.address));
    write(clientSocket, "Enter Designation: ", 19);
    read(clientSocket, st.designation, sizeof(st.designation));
    write(clientSocket, "Enter Department: ", 18);
    read(clientSocket, st.department, sizeof(st.department));
    st.id = id;
    strcpy(st.password, "iiitbp");
    write(fd, &st, sizeof(struct Professor));
    char tempBuffer[100];
    sprintf(tempBuffer, "\nSuccessfully added Teacher\nNote: Login-id is PROF{Prof-ID}\nProf-Id Generated is: %d...\n$", id);
    write(clientSocket, tempBuffer, strlen(tempBuffer) + 1);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void view(int clientSocket, int type, int ID)
{
    int fd, len;
    if (type == 1)
    {
        fd = open("files/student", O_RDONLY);
        if (fd == -1)
        {
            exit(1);
        }
        len = sizeof(struct Student);
    }
    else if (type == 2)
    {
        fd = open("files/professor", O_RDONLY);
        if (fd == -1)
        {
            exit(1);
        }
        len = sizeof(struct Professor);
    }
    int id;
    char buff[10];
    if (ID == -1)
    {
        while (1)
        {
            write(clientSocket, "Enter the ID number to access: ", 32);
            memset(buff, 0, 10);
            read(clientSocket, buff, 10);
            if (buff[0] == '$')
            {
                continue;
            }
            if (!isalnum(buff))
            {
                write(clientSocket, "Wrong id entered, Try again...$\n", 33);
            }
            else
            {
                id = atoi(buff);
                if (id <= 0 || getCount(type) < id)
                {
                    write(clientSocket, "Wrong id entered, Try again...$\n", 33);
                }
                else
                {
                    break;
                }
            }
        }
    }
    else
        id = ID;
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (id - 1) * len;
    lock.l_len = len;
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Student Record!");
        close(fd);
    }
    lseek(fd, (id - 1) * len, SEEK_SET);
    if (type == 1)
    {
        struct Student st;
        read(fd, &st, sizeof(struct Student));
        char send[100 + sizeof(struct Student)];
        sprintf(send, "Name: %s\nEmail: %s\nAddress: %s\nRollno: %d\nUsername: %s$\n", st.name, st.email, st.address, st.rollNumber, st.userName);
        write(clientSocket, send, strlen(send) + 1);
    }
    else
    {
        struct Professor p;
        read(fd, &p, sizeof(struct Professor));
        char send[100 + sizeof(struct Professor)];
        sprintf(send, "Name: %s\nEmail: %s\nDepartment: %s\nDesignation: %s\nAddress: %s\nUsername: %s$\n", p.name, p.email, p.department, p.designation, p.address, p.userName);
        write(clientSocket, send, strlen(send) + 1);
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void activate(int clientSocket, int status)
{
    int fd = open("files/student", O_RDWR);
    if (fd == -1)
    {
        exit(1);
    }
    int id;
    char buff[10];
    while (1)
    {
        if (status == 1)
            write(clientSocket, "Enter the Student ID number to Activate: ", 42);
        else
            write(clientSocket, "Enter the Student ID number to Deactivate: ", 44);
        memset(buff, 0, 10);
        read(clientSocket, buff, 10);
        if (buff[0] == '~')
        {
            continue;
        }
        if (!isalnum(buff))
        {
            write(clientSocket, "Wrong id entered, Try again...$\n", 33);
        }
        else
        {
            id = atoi(buff);
            if (id <= 0 || getCount(1) < id)
            {
                write(clientSocket, "Wrong id entered, Try again...$\n", 33);
            }
            else
            {
                break;
            }
        }
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (id - 1) * sizeof(struct Student);
    lock.l_len = sizeof(struct Student);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Student Record!");
        close(fd);
    }
    lseek(fd, (id - 1) * sizeof(struct Student), SEEK_SET);
    struct Student st;
    read(fd, &st, sizeof(struct Student));
    if (status == 1)
        st.isActive = true;
    else
        st.isActive = false;
    lseek(fd, (id - 1) * sizeof(struct Student), SEEK_SET);
    write(fd, &st, sizeof(struct Student));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void modify(int clientSocket, int type)
{
    int fd, len;
    if (type == 1)
    {
        fd = open("files/student", O_RDWR);
        if (fd == -1)
        {
            exit(1);
        }
        len = sizeof(struct Student);
    }
    else if (type == 2)
    {
        fd = open("files/professor", O_RDWR);
        if (fd == -1)
        {
            exit(1);
        }
        len = sizeof(struct Professor);
    }
    int id;
    char buff[10];
    while (1)
    {
        write(clientSocket, "Enter the ID number to access: ", 32);
        memset(buff, 0, 10);
        read(clientSocket, buff, 10);
        if (buff[0] == '$')
        {
            continue;
        }
        if (!isalnum(buff))
        {
            write(clientSocket, "Wrong id entered, Try again...$\n", 33);
        }
        else
        {
            id = atoi(buff);
            if (id <= 0 || getCount(type) < id)
            {
                write(clientSocket, "Wrong id entered, Try again...$\n", 33);
            }
            else
            {
                break;
            }
        }
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (id - 1) * len;
    lock.l_len = len;
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Student Record!");
        close(fd);
    }
    lseek(fd, (id - 1) * len, SEEK_SET);
    if (type == 1)
    {
        struct Student st;
        read(fd, &st, len);
        char resp[2];
        int choice;
        while (1)
        {
            write(clientSocket, "Enter option{1. Name, 2. Email, 3. Address, 4. Age}: ", 54);
            memset(resp, 0, 2);
            read(clientSocket, resp, 2);
            if (resp[0] == '~')
            {
                continue;
            }
            if (!isalnum(resp))
            {
                write(clientSocket, "Wrong option entered, Try again...$\n", 37);
            }
            else
            {
                choice = atoi(resp);
                if (choice > 5 || choice < 1)
                {
                    write(clientSocket, "Wrong option entered, Try again...$\n", 37);
                }
                else
                {
                    break;
                }
            }
        }
        switch (choice)
        {
        case 1:
            write(clientSocket, "Enter Name: ", 13);
            read(clientSocket, st.name, sizeof(st.name));
            break;
        case 2:
            write(clientSocket, "Enter Email: ", 14);
            read(clientSocket, st.email, sizeof(st.email));
            break;
        case 3:
            write(clientSocket, "Enter Address: ", 16);
            read(clientSocket, st.address, sizeof(st.address));
            break;
        case 4:
            char age[2];
            write(clientSocket, "Enter Age: ", 12);
            read(clientSocket, age, sizeof(age));
            st.age = atoi(age);
            break;
        default:
            break;
        }
        lseek(fd, (id - 1) * len, SEEK_SET);
        write(fd, &st, sizeof(st));
    }

    if (type == 2)
    {
        struct Professor st;
        read(fd, &st, len);
        char resp[2];
        int choice;
        while (1)
        {
            write(clientSocket, "Enter option{1. Name, 2. Email, 3. Address, 4. Department, 5. Designation}: ", 77);
            memset(resp, 0, 2);
            read(clientSocket, resp, 2);
            if (resp[0] == '~')
            {
                continue;
            }
            if (!isalnum(resp))
            {
                write(clientSocket, "Wrong option entered, Try again...$\n", 37);
            }
            else
            {
                choice = atoi(resp);
                if (choice > 5 || choice < 1)
                {
                    write(clientSocket, "Wrong option entered, Try again...$\n", 37);
                }
                else
                {
                    break;
                }
            }
        }
        switch (choice)
        {
        case 1:
            write(clientSocket, "Enter Name: ", 13);
            read(clientSocket, st.name, sizeof(st.name));
            break;
        case 2:
            write(clientSocket, "Enter Email: ", 14);
            read(clientSocket, st.email, sizeof(st.email));
            break;
        case 3:
            write(clientSocket, "Enter Address: ", 16);
            read(clientSocket, st.address, sizeof(st.address));
            break;
        case 4:
            write(clientSocket, "Enter Deparment: ", 12);
            read(clientSocket, st.department, sizeof(st.department));
            break;
        case 5:
            write(clientSocket, "Enter Designation: ", 20);
            read(clientSocket, st.designation, sizeof(st.designation));
            break;
        default:
            break;
        }

        lseek(fd, (id - 1) * len, SEEK_SET);
        write(fd, &st, sizeof(st));
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

#endif