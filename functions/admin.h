#ifndef ADMIN_H
#define ADMIN_H

void addStudent(int clientSocket);
void addProfessor(int clientSocket);
void view(int clientSocket, int type, int ID);
void activate(int clientSocket, int status);
void modify(int clientSocket, int type);
void resetPassword(int clientSocket);

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
                write(clientSocket, "Username is Incorrect~\n", 23);
                return 0;
            }
            else
            {
                if (strcmp(admin.password, password) != 0)
                {
                    write(clientSocket, "Password is Incorrect~\n", 23);
                    return 0;
                }
                else
                {
                    write(clientSocket, "Successful Authentication of Admin\nDisplaying Admin Menu\n$", 58);
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
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
        write(clientSocket, ADMIN_MENU, strlen(ADMIN_MENU));
        memset(dummy, 0, sizeof(dummy));
        if (read(clientSocket, dummy, sizeof(dummy)) == 0)
        {
            write(clientSocket, "No input try again~\n", 21);
            break;
        }
        if (dummy[0] == '$')
        {
        }
        else if (!isalnum((unsigned char)dummy[0]))
        {
            write(clientSocket, "Wrong option entered~\n", 23);
            char skip[2];
            read(clientSocket, skip, 2);
        }
        else
        {
            bool validInput = true;
            for (int i = 0; i < (int)sizeof(dummy); i++)
            {
                if (dummy[i] == '\0')
                    break;
                if (!isalnum((unsigned char)dummy[i]) && !isspace((unsigned char)dummy[i]))
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
                resetPassword(clientSocket);
                break;
            case 10:
                write(clientSocket, "Exiting...~\n", 13);
                return;
                break;
            default:
                printf("Invalid choice. Please select a valid option (1-10).\n");
            }
        }
    }
}

void addStudent(int clientSocket)
{
    int fd = open("files/studentDetails", O_RDWR | O_APPEND);
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
        return;
    }
    int id = setCount(1);
    struct Student st;
    memset(&st, 0, sizeof(st));
    char age[4], gen[4];
    memset(age, 0, sizeof(age));
    memset(gen, 0, sizeof(gen));
    sprintf(st.userName, "MT%d", id);
    write(clientSocket, "Enter Name: ", 12);
    int n = read(clientSocket, st.name, sizeof(st.name) - 1);
    if (n > 0) st.name[n] = '\0';
    write(clientSocket, "Enter Email: ", 13);
    n = read(clientSocket, st.email, sizeof(st.email) - 1);
    if (n > 0) st.email[n] = '\0';
    write(clientSocket, "Enter Address: ", 15);
    n = read(clientSocket, st.address, sizeof(st.address) - 1);
    if (n > 0) st.address[n] = '\0';
    write(clientSocket, "Enter Gender(m/f): ", 19);
    read(clientSocket, gen, sizeof(gen) - 1);
    st.gender = gen[0];
    write(clientSocket, "Enter Age: ", 11);
    read(clientSocket, age, sizeof(age) - 1);
    st.age = atoi(age);
    st.rollNumber = id;
    st.isActive = true;
    strcpy(st.password, "iiitb");
    write(fd, &st, sizeof(struct Student));
    char tempBuffer[150];
    char skip[2];
    sprintf(tempBuffer, "\nSuccessfully added student\nNote: Login-id is MT{Student-ID}\nStudent-Id Generated is: %d\n$", id);
    write(clientSocket, tempBuffer, strlen(tempBuffer) + 1);
    read(clientSocket, skip, 2);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void addProfessor(int clientSocket)
{
    int fd = open("files/teacherDetails", O_RDWR | O_APPEND);
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
        return;
    }
    int id = setCount(2);
    struct Professor st;
    memset(&st, 0, sizeof(st));
    sprintf(st.userName, "PROF%d", id);
    write(clientSocket, "Enter Name: ", 12);
    int n = read(clientSocket, st.name, sizeof(st.name) - 1);
    if (n > 0) st.name[n] = '\0';
    write(clientSocket, "Enter Email: ", 13);
    n = read(clientSocket, st.email, sizeof(st.email) - 1);
    if (n > 0) st.email[n] = '\0';
    write(clientSocket, "Enter Address: ", 15);
    n = read(clientSocket, st.address, sizeof(st.address) - 1);
    if (n > 0) st.address[n] = '\0';
    write(clientSocket, "Enter Designation: ", 19);
    n = read(clientSocket, st.designation, sizeof(st.designation) - 1);
    if (n > 0) st.designation[n] = '\0';
    write(clientSocket, "Enter Department: ", 18);
    n = read(clientSocket, st.department, sizeof(st.department) - 1);
    if (n > 0) st.department[n] = '\0';
    st.id = id;
    strcpy(st.password, "iiitbp");
    write(fd, &st, sizeof(struct Professor));
    char tempBuffer[150];
    char skip[2];
    sprintf(tempBuffer, "\nSuccessfully added Teacher\nNote: Login-id is PROF{Prof-ID}\nProf-Id Generated is: %d\n$", id);
    write(clientSocket, tempBuffer, strlen(tempBuffer) + 1);
    read(clientSocket, skip, 2);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void view(int clientSocket, int type, int ID)
{
    int fd, len;
    if (type == 1)
    {
        fd = open("files/studentDetails", O_RDONLY);
        if (fd == -1)
        {
            exit(1);
        }
        len = sizeof(struct Student);
    }
    else if (type == 2)
    {
        fd = open("files/teacherDetails", O_RDONLY);
        if (fd == -1)
        {
            exit(1);
        }
        len = sizeof(struct Professor);
    }
    else
    {
        return;
    }

    int total = getCount(type);
    if (total == 0)
    {
        char sk[2];
        write(clientSocket, "No records found.\n$", 19);
        read(clientSocket, sk, 2);
        close(fd);
        return;
    }

    /* If a specific ID was requested, show only that record */
    int startID = (ID == -1) ? 1 : ID;
    int endID   = (ID == -1) ? total : ID;

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining read lock on Record!");
        close(fd);
        return;
    }

    for (int i = startID; i <= endID; i++)
    {
        lseek(fd, (i - 1) * len, SEEK_SET);
        char send[400];
        char sk[2];
        if (type == 1)
        {
            struct Student st;
            memset(&st, 0, sizeof(st));
            read(fd, &st, sizeof(struct Student));
            snprintf(send, sizeof(send),
                     "--- Student #%d ---\nName: %s\nUsername: %s\nEmail: %s\nAddress: %s\nGender: %c\nAge: %d\nRoll No: %d\nStatus: %s\n$",
                     i, st.name, st.userName, st.email, st.address,
                     st.gender ? st.gender : '-', st.age, st.rollNumber,
                     st.isActive ? "Active" : "Inactive");
        }
        else
        {
            struct Professor p;
            memset(&p, 0, sizeof(p));
            read(fd, &p, sizeof(struct Professor));
            snprintf(send, sizeof(send),
                     "--- Professor #%d ---\nName: %s\nUsername: %s\nEmail: %s\nDepartment: %s\nDesignation: %s\nAddress: %s\n$",
                     i, p.name, p.userName, p.email, p.department, p.designation, p.address);
        }
        write(clientSocket, send, strlen(send) + 1);
        read(clientSocket, sk, 2);  /* consume client's $ response before showing next record */
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void activate(int clientSocket, int status)
{
    int fd = open("files/studentDetails", O_RDWR);
    if (fd == -1)
    {
        exit(1);
    }
    int id;
    char buff[10];
    while (1)
    {
        if (status == 1)
            write(clientSocket, "Enter the Student ID number to Activate: ", 41);
        else
            write(clientSocket, "Enter the Student ID number to Deactivate: ", 43);
        memset(buff, 0, 10);
        read(clientSocket, buff, 10);
        if (buff[0] == '~')
        {
            continue;
        }
        if (!isalnum((unsigned char)buff[0]))
        {
            write(clientSocket, "Wrong id entered, Try again...$\n", 32);
            char sk[2];
            read(clientSocket, sk, 2);
        }
        else
        {
            id = atoi(buff);
            if (id <= 0 || getCount(1) < id)
            {
                write(clientSocket, "Wrong id entered, Try again...$\n", 32);
                char sk[2];
                read(clientSocket, sk, 2);
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
        return;
    }
    lseek(fd, (id - 1) * sizeof(struct Student), SEEK_SET);
    struct Student st;
    memset(&st, 0, sizeof(st));
    read(fd, &st, sizeof(struct Student));
    st.isActive = (status == 1) ? true : false;
    lseek(fd, (id - 1) * sizeof(struct Student), SEEK_SET);
    write(fd, &st, sizeof(struct Student));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    char msg[60];
    char skip[2];
    snprintf(msg, sizeof(msg), "Student %s successfully\n$",
             status == 1 ? "Activated" : "Deactivated");
    write(clientSocket, msg, strlen(msg) + 1);
    read(clientSocket, skip, 2);
}

void modify(int clientSocket, int type)
{
    int fd, len;
    if (type == 1)
    {
        fd = open("files/studentDetails", O_RDWR);
        if (fd == -1)
        {
            exit(1);
        }
        len = sizeof(struct Student);
    }
    else if (type == 2)
    {
        fd = open("files/teacherDetails", O_RDWR);
        if (fd == -1)
        {
            exit(1);
        }
        len = sizeof(struct Professor);
    }
    else
    {
        return;
    }
    int id;
    char buff[10];
    while (1)
    {
        write(clientSocket, "Enter the ID number to access: ", 31);
        memset(buff, 0, 10);
        read(clientSocket, buff, 10);
        if (buff[0] == '$')
        {
            continue;
        }
        if (!isalnum((unsigned char)buff[0]))
        {
            write(clientSocket, "Wrong id entered, Try again...$\n", 32);
            char sk[2];
            read(clientSocket, sk, 2);
        }
        else
        {
            id = atoi(buff);
            if (id <= 0 || getCount(type) < id)
            {
                write(clientSocket, "Wrong id entered, Try again...$\n", 32);
                char sk[2];
                read(clientSocket, sk, 2);
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
        perror("Error obtaining write lock on Record!");
        close(fd);
        return;
    }
    lseek(fd, (id - 1) * len, SEEK_SET);
    if (type == 1)
    {
        struct Student st;
        memset(&st, 0, sizeof(st));
        read(fd, &st, len);
        char resp[4];
        int choice;
        while (1)
        {
            write(clientSocket, "Enter option{1. Name, 2. Email, 3. Address, 4. Age}: ", 53);
            memset(resp, 0, sizeof(resp));
            read(clientSocket, resp, sizeof(resp));
            if (resp[0] == '~')
            {
                continue;
            }
            if (!isalnum((unsigned char)resp[0]))
            {
                write(clientSocket, "Wrong option entered, Try again...$\n", 36);
                char sk[2];
                read(clientSocket, sk, 2);
            }
            else
            {
                choice = atoi(resp);
                if (choice > 4 || choice < 1)
                {
                    write(clientSocket, "Wrong option entered, Try again...$\n", 36);
                    char sk[2];
                    read(clientSocket, sk, 2);
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
            write(clientSocket, "Enter Name: ", 12);
            memset(st.name, 0, sizeof(st.name));
            { int n = read(clientSocket, st.name, sizeof(st.name) - 1); if (n > 0) st.name[n] = '\0'; }
            break;
        case 2:
            write(clientSocket, "Enter Email: ", 13);
            memset(st.email, 0, sizeof(st.email));
            { int n = read(clientSocket, st.email, sizeof(st.email) - 1); if (n > 0) st.email[n] = '\0'; }
            break;
        case 3:
            write(clientSocket, "Enter Address: ", 15);
            memset(st.address, 0, sizeof(st.address));
            { int n = read(clientSocket, st.address, sizeof(st.address) - 1); if (n > 0) st.address[n] = '\0'; }
            break;
        case 4:
        {
            char age[8];
            memset(age, 0, sizeof(age));
            write(clientSocket, "Enter Age: ", 11);
            read(clientSocket, age, sizeof(age) - 1);
            st.age = atoi(age);
            break;
        }
        default:
            break;
        }
        lseek(fd, (id - 1) * len, SEEK_SET);
        write(fd, &st, sizeof(st));
        char sk[2];
        write(clientSocket, "Modified Successfully\n$", 23);
        read(clientSocket, sk, 2);
    }

    if (type == 2)
    {
        struct Professor st;
        memset(&st, 0, sizeof(st));
        read(fd, &st, len);
        char resp[4];
        int choice;
        while (1)
        {
            write(clientSocket, "Enter option{1. Name, 2. Email, 3. Address, 4. Department, 5. Designation}: ", 76);
            memset(resp, 0, sizeof(resp));
            read(clientSocket, resp, sizeof(resp));
            if (resp[0] == '~')
            {
                continue;
            }
            if (!isalnum((unsigned char)resp[0]))
            {
                write(clientSocket, "Wrong option entered, Try again...$\n", 36);
                char sk[2];
                read(clientSocket, sk, 2);
            }
            else
            {
                choice = atoi(resp);
                if (choice > 5 || choice < 1)
                {
                    write(clientSocket, "Wrong option entered, Try again...$\n", 36);
                    char sk[2];
                    read(clientSocket, sk, 2);
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
            write(clientSocket, "Enter Name: ", 12);
            memset(st.name, 0, sizeof(st.name));
            { int n = read(clientSocket, st.name, sizeof(st.name) - 1); if (n > 0) st.name[n] = '\0'; }
            break;
        case 2:
            write(clientSocket, "Enter Email: ", 13);
            memset(st.email, 0, sizeof(st.email));
            { int n = read(clientSocket, st.email, sizeof(st.email) - 1); if (n > 0) st.email[n] = '\0'; }
            break;
        case 3:
            write(clientSocket, "Enter Address: ", 15);
            memset(st.address, 0, sizeof(st.address));
            { int n = read(clientSocket, st.address, sizeof(st.address) - 1); if (n > 0) st.address[n] = '\0'; }
            break;
        case 4:
            write(clientSocket, "Enter Department: ", 18);
            memset(st.department, 0, sizeof(st.department));
            { int n = read(clientSocket, st.department, sizeof(st.department) - 1); if (n > 0) st.department[n] = '\0'; }
            break;
        case 5:
            write(clientSocket, "Enter Designation: ", 19);
            memset(st.designation, 0, sizeof(st.designation));
            { int n = read(clientSocket, st.designation, sizeof(st.designation) - 1); if (n > 0) st.designation[n] = '\0'; }
            break;
        default:
            break;
        }

        lseek(fd, (id - 1) * len, SEEK_SET);
        write(fd, &st, sizeof(st));
        char sk[2];
        write(clientSocket, "Modified Successfully\n$", 23);
        read(clientSocket, sk, 2);
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void resetPassword(int clientSocket)
{
    char resp[4];
    int type;
    while (1)
    {
        write(clientSocket, "Reset password for: 1. Student  2. Teacher\nEnter choice: ", 57);
        memset(resp, 0, sizeof(resp));
        read(clientSocket, resp, sizeof(resp));
        if (!isalnum((unsigned char)resp[0]))
        {
            write(clientSocket, "Invalid choice, try again.\n$", 28);
            char sk[2];
            read(clientSocket, sk, 2);
            continue;
        }
        type = atoi(resp);
        if (type == 1 || type == 2)
            break;
        write(clientSocket, "Invalid choice, try again.\n$", 28);
        char sk[2];
        read(clientSocket, sk, 2);
    }

    int total = getCount(type);
    if (total == 0)
    {
        char sk[2];
        write(clientSocket, "No records found.\n$", 19);
        read(clientSocket, sk, 2);
        return;
    }

    /* Ask for ID */
    char buff[10];
    int id;
    while (1)
    {
        write(clientSocket, "Enter ID: ", 10);
        memset(buff, 0, sizeof(buff));
        read(clientSocket, buff, sizeof(buff));
        if (!isalnum((unsigned char)buff[0]))
        {
            write(clientSocket, "Invalid ID, try again.\n$", 24);
            char sk[2];
            read(clientSocket, sk, 2);
            continue;
        }
        id = atoi(buff);
        if (id >= 1 && id <= total)
            break;
        write(clientSocket, "Invalid ID, try again.\n$", 24);
        char sk[2];
        read(clientSocket, sk, 2);
    }
    char newPass[50];
    memset(newPass, 0, sizeof(newPass));
    write(clientSocket, "Enter new password: ", 20);
    int n = read(clientSocket, newPass, sizeof(newPass) - 1);
    if (n > 0) newPass[n] = '\0';

    const char *path = (type == 1) ? "files/studentDetails" : "files/teacherDetails";
    int len = (type == 1) ? (int)sizeof(struct Student) : (int)sizeof(struct Professor);

    int fd = open(path, O_RDWR);
    if (fd == -1)
    {
        exit(1);
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
        perror("Error obtaining write lock for password reset");
        close(fd);
        return;
    }
    lseek(fd, (id - 1) * len, SEEK_SET);
    if (type == 1)
    {
        struct Student st;
        memset(&st, 0, sizeof(st));
        read(fd, &st, sizeof(st));
        strncpy(st.password, newPass, sizeof(st.password) - 1);
        st.password[sizeof(st.password) - 1] = '\0';
        lseek(fd, (id - 1) * len, SEEK_SET);
        write(fd, &st, sizeof(st));
    }
    else
    {
        struct Professor p;
        memset(&p, 0, sizeof(p));
        read(fd, &p, sizeof(p));
        strncpy(p.password, newPass, sizeof(p.password) - 1);
        p.password[sizeof(p.password) - 1] = '\0';
        lseek(fd, (id - 1) * len, SEEK_SET);
        write(fd, &p, sizeof(p));
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    char skip[2];
    write(clientSocket, "Password reset successfully.\n$", 30);
    read(clientSocket, skip, 2);
}

#endif
