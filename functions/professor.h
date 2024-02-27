#ifndef PROFESSOR_H
#define PROFESSOR_H

void addCourse(int clientSocket, int session);
int *viewAllCourses(int clientSocket, int session, int *courseCount, int flag);
void deleteCourse(int clientSocket, int session);
struct Enrollment *viewEnrollments(int clientSocket, int session, int *enrollCount, int flag);
void modifyCourse(int clientSocket, int session);
int handleSeats(int clientSocket, int session, int cid, int prevSeats, int currSeats, int available);


int professorLogin(int clientSocket, char *userName, char *password, int *session)
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

void professorHandler(int clientSocket, int session)
{
    char resp[4];
    int choice;
    read(clientSocket, resp, 4);
    if (strchr(resp, '$') != NULL)
    {
    }
    while (1)
    {
        write(clientSocket, PROFESSOR_MENU, 185);
        memset(resp, 0, 4);
        if (read(clientSocket, resp, 4) == 0)
        {
            write(clientSocket, "No input try again...~\n", 24);
            break;
        }
        if (resp[0] == '$')
        {
        }
        else if (!isalnum(resp))
        {
            write(clientSocket, "Wrong option entered...$\n", 26);
        }
        else
        {
            choice = atoi(resp);
            switch (choice)
            {
            case 1:
                int courseCnt;
                viewAllCourses(clientSocket, session, &courseCnt, 1);
                break;
            case 2:
                addCourse(clientSocket, session);
                break;
            case 3:
                deleteCourse(clientSocket, session);
                break;
            case 4:
                modifyCourse(clientSocket, session);
                break;
            case 5:
                int cnt;
                viewEnrollments(clientSocket, session, &cnt, 1);
                break;
            case 6:
                write(clientSocket, "Exiting...~\n", 13);
                return;
                break;
            default:
                printf("Invalid choice. Please select a valid option (1-9).\n");
            }
        }
    }
}

void addCourse(int clientSocket, int session)
{
    int fd = open("files/courses", O_RDWR | O_APPEND);
    if (fd == -1)
    {
        exit(1);
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(struct Course);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Course Record!");
        close(fd);
    }
    int id = setCount(3);
    struct Course c;
    char seats[4], credits[2];
    write(clientSocket, "Enter Name: ", 13);
    read(clientSocket, c.cname, sizeof(c.cname));
    write(clientSocket, "Enter Department: ", 19);
    read(clientSocket, c.department, sizeof(c.department));
    write(clientSocket, "Enter Credits: ", 16);
    read(clientSocket, credits, 2);
    write(clientSocket, "Enter Total Seats: ", 20);
    read(clientSocket, seats, 4);
    c.credits = atoi(credits);
    c.seats = atoi(seats);
    c.available = c.seats;
    c.profId = session;
    c.isActive = 1;
    c.id = id;
    write(fd, &c, sizeof(struct Course));
    char tempBuffer[100];
    sprintf(tempBuffer, "\nSuccessfully added Course\nCourse-Id Generated is: %d\n$", id);
    write(clientSocket, tempBuffer, strlen(tempBuffer) + 1);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

int *viewAllCourses(int clientSocket, int session, int *courseCount, int flag)
{
    int fd = open("records/courses", O_RDONLY);
    if (fd == -1)
    {
        exit(1);
    }
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining Read lock on Courses Record!");
        close(fd);
    }
    lseek(fd, 0, SEEK_SET);
    struct Course c;
    int bytesRead;
    int arraySize = 1;
    int count = 0;
    int *cid = (int *)malloc(arraySize * sizeof(int));
    while ((bytesRead = read(fd, &c, sizeof(struct Course))) == sizeof(struct Course) || bytesRead == -1)
    {
        if (c.isActive && c.profId == session)
        {
            if (flag)
            {
                char send[100 + sizeof(struct Course)], skip[2];
                sprintf(send, "Course Name: %s\nDepartment: %s\nCredits: %d\nTotal seats: %d\nAvailable Seats: %d\nId: %d\n$\n", c.cname, c.department, c.credits, c.seats, c.available, c.id);
                write(clientSocket, send, strlen(send) + 1);
                read(clientSocket, skip, 2);
                memset(send, 0, 100 + sizeof(struct Course));
            }

            arraySize += 1;
            cid = (int *)realloc(cid, arraySize * sizeof(int));
            if (cid == NULL)
            {
                perror("Memory reallocation failed");
            }
            cid[count++] = c.id;
        }
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    *courseCount = count;
    return cid;
}

void deleteCourse(int clientSocket, int session)
{
    int fd = open("files/courses", O_RDWR);
    if (fd == -1)
    {
        exit(1);
    }
    int courseCount;
    int *cids = viewAllCourses(clientSocket, session, &courseCount, 1);
    int enrollCnt;
    struct Enrollment *e = viewEnrollments(clientSocket, session, &enrollCnt, 0);
    int id;
    char buff[10];
    while (1)
    {
        write(clientSocket, "Enter the course ID number to Deactivate: ", 43);
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
            if (id <= 0 || getCount(3) < id)
            {
                write(clientSocket, "Wrong id entered, Try again...$\n", 33);
            }
            else
            {
                break;
            }
        }
    }
    int flag = 0;
    for (int i = 0; i < courseCount; i++)
    {
        if (cids[i] == id)
        {
            flag = 1;
            break;
        }
    }
    if (flag == 0)
    {
        write(clientSocket, "Course not Found...\n$\n", 23);
        return;
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (id - 1) * sizeof(struct Course);
    lock.l_len = sizeof(struct Course);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Course Record!");
        close(fd);
    }
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    struct Course c;
    read(fd, &c, sizeof(struct Course));
    c.isActive = 0;
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    write(fd, &c, sizeof(struct Course));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    fd = open("files/enrollments", O_RDWR);
    if (fd == -1)
    {
        exit(1);
    }
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining Read lock on Courses Record!");
        close(fd);
    }
    lseek(fd, 0, SEEK_SET);
    for (int i = 0; i < enrollCnt; i++)
    {
        struct Enrollment w = e[i];
        if (w.courseID == id)
        {
            int eid = w.enrollmentID;
            w.isEnrolled = 0;
            lseek(fd, (eid - 1) * sizeof(struct Enrollment), SEEK_SET);
            write(fd, &w, sizeof(struct Enrollment));
        }
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

struct Enrollment *viewEnrollments(int clientSocket, int session, int *enrollCount, int flag)
{
    int fd = open("records/enrollments", O_RDONLY);
    if (fd == -1)
    {
        exit(1);
    }
    int courseCount = 0, arraySize = 1, count = 0;
    struct Enrollment *e = (struct Enrollment *)malloc(arraySize * sizeof(struct Enrollment));

    int *cids = viewAllCourses(clientSocket, session, &courseCount, 0);
    if (courseCount == 0)
    {
        write(clientSocket, "No Courses Created...\n$\n", 25);
        return e;
    }
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining Read lock on Courses Record!");
        close(fd);
    }
    int bytesRead;
    for (int i = 0; i < courseCount; i++)
    {
        struct Course c = getCourses(clientSocket, cids[i]);
        char send[100 + sizeof(struct Course)], skip[2];
        if (flag)
        {
            sprintf(send, "Course Name: %s\nDepartment: %s\nCredits: %d\nTotal seats: %d\nAvailable Seats: %d\nId: %d\n~\n", c.cname, c.department, c.credits, c.seats, c.available, c.id);
            write(clientSocket, send, strlen(send) + 1);
            read(clientSocket, skip, 2);
            memset(send, 0, 100 + sizeof(struct Course));
        }
        struct Enrollment er;
        lseek(fd, 0, SEEK_SET);
        int ec = 0;
        while ((bytesRead = read(fd, &er, sizeof(struct Enrollment))) == sizeof(struct Enrollment) || bytesRead == -1)
        {
            if (er.isEnrolled && er.courseID == c.id && er.profID == session)
            {
                char sk[2];
                if (flag)
                {
                    write(clientSocket, ".........................Enrollment Details.........................\n$\n", 72);
                    viewEnrollments(clientSocket, 1, &er.studentID, 1);
                    read(clientSocket, sk, 2);
                }
                arraySize += 1;
                e = (struct Enrollment *)realloc(e, arraySize * sizeof(struct Enrollment));
                if (e == NULL)
                {
                    perror("Memory reallocation failed");
                }
                e[count++] = er;
                ec++;
            }
        }
        if (ec == 0)
        {
            char sk[2];
            write(clientSocket, ".........................Enrollment Details.........................\n$\n", 72);
            write(clientSocket, "No Enrollments for this course...\n$\n", 37);
            read(clientSocket, sk, 2);
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    *enrollCount = count;
    return e;
}

void modifyCourse(int clientSocket, int session)
{
    int courseCount;
    int *cids = viewAllCourses(clientSocket, session, &courseCount, 1);

    int id;
    char buff[10];
    while (1)
    {
        write(clientSocket, "Enter the course ID number to Modify: ", 39);
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
            if (id <= 0 || getCount(3) < id)
            {
                write(clientSocket, "Wrong id entered, Try again...$\n", 33);
            }
            else
            {
                break;
            }
        }
    }
    int flag = 0;
    for (int i = 0; i < courseCount; i++)
    {
        if (cids[i] == id)
        {
            flag = 1;
            break;
        }
    }
    if (flag == 0)
    {
        write(clientSocket, "Course not Found...\n$\n", 23);
        return;
    }
    int fd = open("files/courses", O_RDWR);
    if (fd == -1)
    {
        exit(1);
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (id - 1) * sizeof(struct Course);
    lock.l_len = sizeof(struct Course);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Course Record!");
        close(fd);
    }
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    struct Course c;
    read(fd, &c, sizeof(struct Course));
    char resp[2];
    int choice;
    while (1)
    {
        write(clientSocket, "Enter option{1. Course Name, 2. Department, 3. Credits, 4. Seats}: ", 68);
        memset(resp, 0, 2);
        read(clientSocket, resp, 2);
        if (resp[0] == '$')
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
            if (choice > 4 || choice < 1)
            {
                write(clientSocket, "Wrong option entered, Try again...$\n", 37);
            }
            else
            {
                break;
            }
        }
    }
    char credits[4];
    switch (choice)
    {
    case 1:
        write(clientSocket, "Enter Course Name: ", 20);
        read(clientSocket, c.cname, sizeof(c.cname));
        break;
    case 2:
        write(clientSocket, "Enter Department: ", 19);
        read(clientSocket, c.department, sizeof(c.department));
        break;
    case 3:
        write(clientSocket, "Enter Credits: ", 16);
        read(clientSocket, credits, sizeof(credits));
        c.credits = atoi(credits);
        break;
    case 4:
        char s[4];
        write(clientSocket, "Enter Seats: ", 14);
        read(clientSocket, s, sizeof(s));
        int seats = atoi(s);
        int retV = handleSeats(clientSocket, session, id, c.seats, seats, c.available);
        c.available += retV;
        c.seats = seats;
        break;
    default:
        break;
    }
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    write(fd, &c, sizeof(struct Course));

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

int handleSeats(int clientSocket, int session, int cid, int prevSeats, int currSeats, int available)
{
    if (prevSeats <= currSeats)
    {
        return currSeats - prevSeats;
    }
    else if (prevSeats > currSeats)
    {

        int seatsDecr = prevSeats - currSeats;
        if (available >= seatsDecr)
            return -seatsDecr;
        else
        {
            int unEnrollNum = seatsDecr - available;
            int enrollCnt;
            struct Enrollment *e = viewEnrollments(clientSocket, session, &enrollCnt, 0);
            int struct_size = sizeof(struct Enrollment);

            int fd = open("files/enrollments", O_RDWR);
            if (fd == -1)
            {
                exit(1);
            }
            struct flock lock;
            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = 0;
            lock.l_len = 0;
            lock.l_pid = getpid();
            int f = fcntl(fd, F_SETLKW, &lock);
            if (f == -1)
            {
                perror("Error obtaining Read lock on Courses Record!");
                close(fd);
            }

            int file_size = lseek(fd, 0, SEEK_END);

            if (file_size == -1)
            {
                perror("Error getting file size");
                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                return -2;
            }
            for (int i = file_size - struct_size; i >= 0; i -= struct_size)
            {
                lseek(fd, i, SEEK_SET);
                if (unEnrollNum > 0)
                {
                    struct Enrollment unEnroll;
                    if (read(fd, &unEnroll, struct_size) == -1)
                    {
                        perror("Error reading the file");
                        lock.l_type = F_UNLCK;
                        fcntl(fd, F_SETLK, &lock);
                        close(fd);
                        return 1;
                    }
                    struct Enrollment w = unEnroll;
                    if (w.courseID == cid)
                    {
                        int eid = w.enrollmentID;
                        w.isEnrolled = 0;
                        lseek(fd, (eid - 1) * sizeof(struct Enrollment), SEEK_SET);
                        write(fd, &w, sizeof(struct Enrollment));
                    }
                    unEnrollNum--;
                }
                else
                    break;
            }

            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            return -available;
        }
    }
}

#endif