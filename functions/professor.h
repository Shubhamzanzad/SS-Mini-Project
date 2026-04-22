#ifndef PROFESSOR_H
#define PROFESSOR_H

void addCourse(int clientSocket, int session);
int *viewAllCourses(int clientSocket, int session, int *courseCount, int flag);
void deleteCourse(int clientSocket, int session);
struct Enrollment *viewEnrollments(int clientSocket, int session, int *enrollCount, int flag);
void modifyCourse(int clientSocket, int session);
int handleSeats(int clientSocket, int session, int cid, int prevSeats, int currSeats, int available);

/* Forward declaration from admin.h so we can show student details */
void view(int clientSocket, int type, int ID);

int professorLogin(int clientSocket, char *userName, char *password, int *session)
{
    struct Professor p;
    int fd = open("files/teacherDetails", O_RDONLY);
    if (fd == -1)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    int id;
    if (sscanf(userName, "PROF%d", &id) == 1)
    {
        int offset = lseek(fd, (id - 1) * sizeof(struct Professor), SEEK_SET);
        if (offset != -1)
        {
            struct flock lock;
            lock.l_type = F_RDLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = (id - 1) * sizeof(struct Professor);
            lock.l_len = sizeof(struct Professor);
            lock.l_pid = getpid();
            int f = fcntl(fd, F_SETLKW, &lock);
            if (f == -1)
            {
                perror("Error obtaining read lock on Professor Record!");
                close(fd);
                return 0;
            }
            int readBytes = read(fd, &p, sizeof(p));
            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            if (readBytes > 0)
            {
                if (strcmp(p.userName, userName) != 0)
                {
                    write(clientSocket, "Username is Incorrect~\n", 23);
                    return 0;
                }
                else if (strcmp(p.password, password) != 0)
                {
                    write(clientSocket, "Password is incorrect~\n", 23);
                    return 0;
                }
                *session = p.id;
                write(clientSocket, "Authenticated as faculty\n$", 26);
                return 1;
            }
        }
        else
            write(clientSocket, "Username not found~\n", 20);
    }
    else
        write(clientSocket, "Username not found~\n", 20);
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
        write(clientSocket, PROFESSOR_MENU, strlen(PROFESSOR_MENU));
        memset(resp, 0, 4);
        if (read(clientSocket, resp, 4) == 0)
        {
            write(clientSocket, "No input try again...~\n", 23);
            break;
        }
        if (resp[0] == '$')
        {
        }
        else if (!isalnum((unsigned char)resp[0]))
        {
            write(clientSocket, "Wrong option entered...$\n", 25);
            char sk[2];
            read(clientSocket, sk, 2);
        }
        else
        {
            choice = atoi(resp);
            switch (choice)
            {
            case 1:
            {
                int courseCnt;
                viewAllCourses(clientSocket, session, &courseCnt, 1);
                break;
            }
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
            {
                int cnt;
                viewEnrollments(clientSocket, session, &cnt, 1);
                break;
            }
            case 6:
                write(clientSocket, "Exiting...~\n", 13);
                return;
                break;
            default:
                printf("Invalid choice. Please select a valid option (1-6).\n");
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
        return;
    }
    int id = setCount(3);
    struct Course c;
    memset(&c, 0, sizeof(c));
    char seats[8], credits[4];
    memset(seats, 0, sizeof(seats));
    memset(credits, 0, sizeof(credits));
    write(clientSocket, "Enter Name: ", 12);
    int n = read(clientSocket, c.cname, sizeof(c.cname) - 1);
    if (n > 0) c.cname[n] = '\0';
    write(clientSocket, "Enter Department: ", 18);
    n = read(clientSocket, c.department, sizeof(c.department) - 1);
    if (n > 0) c.department[n] = '\0';
    write(clientSocket, "Enter Credits: ", 15);
    read(clientSocket, credits, sizeof(credits) - 1);
    write(clientSocket, "Enter Total Seats: ", 19);
    read(clientSocket, seats, sizeof(seats) - 1);
    c.credits = atoi(credits);
    c.seats = atoi(seats);
    c.available = c.seats;
    c.profId = session;
    c.isActive = 1;
    c.id = id;
    write(fd, &c, sizeof(struct Course));
    char tempBuffer[100];
    char skip[2];
    sprintf(tempBuffer, "\nSuccessfully added Course\nCourse-Id Generated is: %d\n$", id);
    write(clientSocket, tempBuffer, strlen(tempBuffer) + 1);
    read(clientSocket, skip, 2);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

int *viewAllCourses(int clientSocket, int session, int *courseCount, int flag)
{
    int fd = open("files/courses", O_RDONLY);
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
        *courseCount = 0;
        return NULL;
    }
    lseek(fd, 0, SEEK_SET);
    struct Course c;
    memset(&c, 0, sizeof(c));
    int bytesRead;
    int arraySize = 1;
    int count = 0;
    int *cid = (int *)malloc(arraySize * sizeof(int));
    while ((bytesRead = read(fd, &c, sizeof(struct Course))) == sizeof(struct Course))
    {
        if (c.isActive && c.profId == session)
        {
            if (flag)
            {
                char send[300], skip[2];
                snprintf(send, sizeof(send), "Course Name: %s\nDepartment: %s\nCredits: %d\nTotal seats: %d\nAvailable Seats: %d\nId: %d\n$",
                         c.cname, c.department, c.credits, c.seats, c.available, c.id);
                write(clientSocket, send, strlen(send) + 1);
                read(clientSocket, skip, 2);
            }

            arraySize += 1;
            cid = (int *)realloc(cid, arraySize * sizeof(int));
            if (cid == NULL)
            {
                perror("Memory reallocation failed");
                break;
            }
            cid[count++] = c.id;
        }
        memset(&c, 0, sizeof(c));
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    *courseCount = count;
    if (count == 0 && flag)
    {
        char skip[2];
        write(clientSocket, "No courses found...\n$", 21);
        read(clientSocket, skip, 2);
    }
    return cid;
}

void deleteCourse(int clientSocket, int session)
{
    int courseCount;
    int *cids = viewAllCourses(clientSocket, session, &courseCount, 1);
    if (courseCount == 0)
    {
        if (cids) free(cids);
        return;
    }
    int enrollCnt;
    struct Enrollment *e = viewEnrollments(clientSocket, session, &enrollCnt, 0);
    int id;
    char buff[10];
    while (1)
    {
        write(clientSocket, "Enter the course ID number to Deactivate: ", 42);
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
            if (id <= 0 || getCount(3) < id)
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
    int flag = 0;
    for (int i = 0; i < courseCount; i++)
    {
        if (cids[i] == id)
        {
            flag = 1;
            break;
        }
    }
    if (cids) free(cids);
    if (flag == 0)
    {
        char sk[2];
        write(clientSocket, "Course not Found...\n$\n", 22);
        read(clientSocket, sk, 2);
        if (e) free(e);
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
        if (e) free(e);
        return;
    }
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    struct Course c;
    memset(&c, 0, sizeof(c));
    read(fd, &c, sizeof(struct Course));
    c.isActive = 0;
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    write(fd, &c, sizeof(struct Course));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    /* Unenroll all students from this course */
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
        perror("Error obtaining write lock on Enrollments Record!");
        close(fd);
        if (e) free(e);
        return;
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
    if (e) free(e);

    char skip[2];
    write(clientSocket, "Course deactivated successfully\n$", 33);
    read(clientSocket, skip, 2);
}

struct Enrollment *viewEnrollments(int clientSocket, int session, int *enrollCount, int flag)
{
    int fd = open("files/enrollments", O_RDONLY);
    if (fd == -1)
    {
        exit(1);
    }
    int courseCount = 0, arraySize = 1, count = 0;
    struct Enrollment *e = (struct Enrollment *)malloc(arraySize * sizeof(struct Enrollment));

    int *cids = viewAllCourses(clientSocket, session, &courseCount, 0);
    if (courseCount == 0)
    {
        if (flag)
        {
            char sk[2];
            write(clientSocket, "No Courses Created...\n$", 23);
            read(clientSocket, sk, 2);
        }
        close(fd);
        if (cids) free(cids);
        *enrollCount = 0;
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
        perror("Error obtaining Read lock on Enrollments Record!");
        close(fd);
        if (cids) free(cids);
        *enrollCount = 0;
        return e;
    }
    int bytesRead;
    for (int i = 0; i < courseCount; i++)
    {
        struct Course c = getCourses(clientSocket, cids[i]);
        int ec = 0;

        if (flag)
        {
            char send[300], skip[2];
            snprintf(send, sizeof(send), "Course Name: %s\nDepartment: %s\nCredits: %d\nTotal seats: %d\nAvailable Seats: %d\nId: %d\n$",
                     c.cname, c.department, c.credits, c.seats, c.available, c.id);
            write(clientSocket, send, strlen(send) + 1);
            read(clientSocket, skip, 2);
        }

        struct Enrollment er;
        memset(&er, 0, sizeof(er));
        lseek(fd, 0, SEEK_SET);
        while ((bytesRead = read(fd, &er, sizeof(struct Enrollment))) == sizeof(struct Enrollment))
        {
            if (er.isEnrolled && er.courseID == c.id && er.profID == session)
            {
                if (flag)
                {
                    /* Read the enrolled student's info and display */
                    int sfd = open("files/studentDetails", O_RDONLY);
                    if (sfd != -1)
                    {
                        struct Student stu;
                        memset(&stu, 0, sizeof(stu));
                        lseek(sfd, (er.studentID - 1) * sizeof(struct Student), SEEK_SET);
                        read(sfd, &stu, sizeof(struct Student));
                        close(sfd);
                        char sk[2];
                        char stuInfo[300];
                        snprintf(stuInfo, sizeof(stuInfo),
                                 "Student: %s | Email: %s | Roll No: %d\n$",
                                 stu.name, stu.email, stu.rollNumber);
                        write(clientSocket, stuInfo, strlen(stuInfo) + 1);
                        read(clientSocket, sk, 2);
                    }
                }
                arraySize += 1;
                e = (struct Enrollment *)realloc(e, arraySize * sizeof(struct Enrollment));
                if (e == NULL)
                {
                    perror("Memory reallocation failed");
                    break;
                }
                e[count++] = er;
                ec++;
            }
            memset(&er, 0, sizeof(er));
        }
        if (ec == 0 && flag)
        {
            char sk[2];
            write(clientSocket, "  No enrollments for this course\n$", 34);
            read(clientSocket, sk, 2);
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    if (cids) free(cids);
    *enrollCount = count;
    return e;
}

void modifyCourse(int clientSocket, int session)
{
    int courseCount;
    int *cids = viewAllCourses(clientSocket, session, &courseCount, 1);
    if (courseCount == 0)
    {
        if (cids) free(cids);
        return;
    }

    int id;
    char buff[10];
    while (1)
    {
        write(clientSocket, "Enter the course ID number to Modify: ", 38);
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
            if (id <= 0 || getCount(3) < id)
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
    int flag = 0;
    for (int i = 0; i < courseCount; i++)
    {
        if (cids[i] == id)
        {
            flag = 1;
            break;
        }
    }
    if (cids) free(cids);
    if (flag == 0)
    {
        char sk[2];
        write(clientSocket, "Course not Found...\n$\n", 22);
        read(clientSocket, sk, 2);
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
        return;
    }
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    struct Course c;
    memset(&c, 0, sizeof(c));
    read(fd, &c, sizeof(struct Course));
    char resp[4];
    int choice;
    while (1)
    {
        write(clientSocket, "Enter option{1. Course Name, 2. Department, 3. Credits, 4. Seats}: ", 67);
        memset(resp, 0, sizeof(resp));
        read(clientSocket, resp, sizeof(resp));
        if (resp[0] == '$')
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
    char credits[8];
    memset(credits, 0, sizeof(credits));
    switch (choice)
    {
    case 1:
        write(clientSocket, "Enter Course Name: ", 19);
        memset(c.cname, 0, sizeof(c.cname));
        { int n = read(clientSocket, c.cname, sizeof(c.cname) - 1); if (n > 0) c.cname[n] = '\0'; }
        break;
    case 2:
        write(clientSocket, "Enter Department: ", 18);
        memset(c.department, 0, sizeof(c.department));
        { int n = read(clientSocket, c.department, sizeof(c.department) - 1); if (n > 0) c.department[n] = '\0'; }
        break;
    case 3:
        write(clientSocket, "Enter Credits: ", 15);
        read(clientSocket, credits, sizeof(credits) - 1);
        c.credits = atoi(credits);
        break;
    case 4:
    {
        char s[8];
        memset(s, 0, sizeof(s));
        write(clientSocket, "Enter Seats: ", 13);
        read(clientSocket, s, sizeof(s) - 1);
        int seats = atoi(s);
        int retV = handleSeats(clientSocket, session, id, c.seats, seats, c.available);
        c.available += retV;
        c.seats = seats;
        break;
    }
    default:
        break;
    }
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    write(fd, &c, sizeof(struct Course));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    char skip[2];
    write(clientSocket, "Course modified successfully\n$", 30);
    read(clientSocket, skip, 2);
}

int handleSeats(int clientSocket, int session, int cid, int prevSeats, int currSeats, int available)
{
    if (prevSeats <= currSeats)
    {
        return currSeats - prevSeats;
    }
    else
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
                if (e) free(e);
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
                perror("Error obtaining write lock on Enrollments Record!");
                close(fd);
                if (e) free(e);
                return -2;
            }

            int file_size = lseek(fd, 0, SEEK_END);
            if (file_size == -1)
            {
                perror("Error getting file size");
                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                if (e) free(e);
                return -2;
            }
            for (int i = file_size - struct_size; i >= 0 && unEnrollNum > 0; i -= struct_size)
            {
                lseek(fd, i, SEEK_SET);
                struct Enrollment unEnroll;
                memset(&unEnroll, 0, sizeof(unEnroll));
                if (read(fd, &unEnroll, struct_size) != struct_size)
                {
                    perror("Error reading the file");
                    break;
                }
                if (unEnroll.courseID == cid && unEnroll.isEnrolled)
                {
                    int eid = unEnroll.enrollmentID;
                    unEnroll.isEnrolled = 0;
                    lseek(fd, (eid - 1) * sizeof(struct Enrollment), SEEK_SET);
                    write(fd, &unEnroll, sizeof(struct Enrollment));
                    unEnrollNum--;
                }
            }

            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            if (e) free(e);
            return -available;
        }
    }
}

#endif
