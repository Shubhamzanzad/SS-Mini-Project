#ifndef STUDENT_H
#define STUDENT_H

void viewCourses(int clientSocket);
void enrollCourse(int clientSocket, int session);
int *getEnrollments(int clientSocket, int session, int *cnt, int flag);
void dropCourse(int clientSocket, int session);
void changePassword(int clientSocket, int session);

int studentLogin(int clientSocket, char userName[50], char password[50], int *session)
{
    struct Student student;
    int fd = open("files/studentDetails", O_RDONLY);
    if (fd == -1)
    {
        perror("Student File opening error");
        return 0;
    }
    int id;
    if (sscanf(userName, "MT%d", &id) == 1)
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
            memset(&student, 0, sizeof(student));
            int readBytes = read(fd, &student, sizeof(student));
            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            if (readBytes > 0)
            {
                if (getCount(1) >= id)
                {
                    if (strcmp(student.userName, userName) != 0 || strcmp(student.password, password) != 0)
                    {
                        write(clientSocket, "Username or Password wrong.....~\n", 33);
                        return 0;
                    }
                    else
                    {
                        if (student.isActive)
                        {
                            *session = id;
                            write(clientSocket, "Successfully Authenticated as Student....\n$", 43);
                            return 1;
                        }
                        else
                            write(clientSocket, "Your ID is Blocked.....~\n", 25);
                    }
                }
                else
                    write(clientSocket, "Username not found.....~\n", 25);
            }
            else
                write(clientSocket, "Username wrong.....~\n", 21);
        }
    }
    else
    {
        write(clientSocket, "Username not found.....~\n", 25);
        close(fd);
    }
    return 0;
}

void studentHandler(int clientSocket, int session)
{
    char resp[4];
    int choice;
    read(clientSocket, resp, 4);
    if (strchr(resp, '$') != NULL)
    {
    }
    while (1)
    {
        write(clientSocket, STUDENT_MENU, strlen(STUDENT_MENU));
        memset(resp, 0, 4);
        if (read(clientSocket, resp, sizeof(resp)) == 0)
        {
            write(clientSocket, "No input try again...~\n", 23);
            break;
        }
        else if (resp[0] == '$')
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
                viewCourses(clientSocket);
                break;
            case 2:
                enrollCourse(clientSocket, session);
                break;
            case 3:
                dropCourse(clientSocket, session);
                break;
            case 4:
            {
                int cnt;
                int *enrolled = getEnrollments(clientSocket, session, &cnt, 1);
                if (cnt == 0)
                {
                    char skip[2];
                    write(clientSocket, "No Courses Enrolled...$\n", 24);
                    read(clientSocket, skip, 2);
                }
                if (enrolled) free(enrolled);
                break;
            }
            case 5:
                changePassword(clientSocket, session);
                break;
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

void viewCourses(int clientSocket)
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
        return;
    }
    lseek(fd, 0, SEEK_SET);
    int courseCnt = getCount(3);
    if (courseCnt == 0)
    {
        char skip[2];
        write(clientSocket, "No Courses Available...$\n", 25);
        read(clientSocket, skip, 2);
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        return;
    }
    struct Course c;
    memset(&c, 0, sizeof(c));
    int bytesRead;
    int shown = 0;
    while ((bytesRead = read(fd, &c, sizeof(struct Course))) == sizeof(struct Course))
    {
        if (c.isActive)
        {
            char send[300], skip[2];
            snprintf(send, sizeof(send), "Course Name: %s\nDepartment: %s\nCredits: %d\nTotal seats: %d\nAvailable Seats: %d\nId: %d\n$",
                     c.cname, c.department, c.credits, c.seats, c.available, c.id);
            write(clientSocket, send, strlen(send) + 1);
            read(clientSocket, skip, 2);
            shown++;
        }
        memset(&c, 0, sizeof(c));
    }
    if (shown == 0)
    {
        char skip[2];
        write(clientSocket, "No Active Courses Available...$\n", 32);
        read(clientSocket, skip, 2);
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void enrollCourse(int clientSocket, int session)
{
    int fd = open("files/enrollments", O_RDWR | O_APPEND);
    if (fd == -1)
    {
        exit(1);
    }
    int courseCnt = getCount(3);
    if (courseCnt == 0)
    {
        char skip[2];
        write(clientSocket, "No Courses Available...$\n", 25);
        read(clientSocket, skip, 2);
        close(fd);
        return;
    }
    int id;
    char buff[10];
    while (1)
    {
        write(clientSocket, "Enter the course ID number to Enroll: ", 38);
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
    int cnt;
    int *enrolled = getEnrollments(clientSocket, session, &cnt, 0);
    int already = 0;
    for (int i = 0; i < cnt; i++)
    {
        if (enrolled[i] == id)
        {
            already = 1;
            break;
        }
    }
    if (enrolled) free(enrolled);
    if (already == 1)
    {
        char skip[2];
        write(clientSocket, "Course already Enrolled...$\n", 28);
        read(clientSocket, skip, 2);
        close(fd);
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(struct Enrollment);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Enrollment Record!");
        close(fd);
        return;
    }
    struct Course c = getCourses(clientSocket, id);
    if (!c.isActive)
    {
        char skip[2];
        write(clientSocket, "Course is not active...$\n", 25);
        read(clientSocket, skip, 2);
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        return;
    }
    if (c.available > 0)
    {
        int enrollId = setCount(4);
        setAvailability(clientSocket, id, -1);
        struct Enrollment e = {session, c.id, c.profId, enrollId, 1};
        write(fd, &e, sizeof(struct Enrollment));
        char tempBuffer[150];
        char skip[2];
        snprintf(tempBuffer, sizeof(tempBuffer), "\nSuccessfully Enrolled in the course - %s\n$", c.cname);
        write(clientSocket, tempBuffer, strlen(tempBuffer) + 1);
        read(clientSocket, skip, 2);
    }
    else
    {
        char skip[2];
        write(clientSocket, "No seats left...$\n", 18);
        read(clientSocket, skip, 2);
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

int *getEnrollments(int clientSocket, int session, int *cnt, int flag)
{
    int fd = open("files/enrollments", O_RDONLY);
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
        perror("Error obtaining Read lock on Enrollments Record!");
        close(fd);
        *cnt = 0;
        return NULL;
    }
    lseek(fd, 0, SEEK_SET);
    struct Enrollment e;
    memset(&e, 0, sizeof(e));
    int bytesRead;
    int arraySize = 1;
    int count = 0;
    int *cid = (int *)malloc(arraySize * sizeof(int));
    while ((bytesRead = read(fd, &e, sizeof(struct Enrollment))) == sizeof(struct Enrollment))
    {
        if (e.isEnrolled && e.studentID == session)
        {
            if (flag)
            {
                char send[300], skip[2];
                struct Course c = getCourses(clientSocket, e.courseID);
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
            cid[count++] = e.courseID;
        }
        memset(&e, 0, sizeof(e));
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    *cnt = count;
    return cid;
}

void dropCourse(int clientSocket, int session)
{
    int fd = open("files/enrollments", O_RDWR);
    if (fd == -1)
    {
        exit(1);
    }
    int courseCnt = getCount(3);
    if (courseCnt == 0)
    {
        char skip[2];
        write(clientSocket, "No Courses Available...$\n", 25);
        read(clientSocket, skip, 2);
        close(fd);
        return;
    }
    int cnt;
    int *enrolled = getEnrollments(clientSocket, session, &cnt, 0);
    if (cnt == 0)
    {
        char skip[2];
        write(clientSocket, "No Courses Enrolled...$\n", 24);
        read(clientSocket, skip, 2);
        if (enrolled) free(enrolled);
        close(fd);
        return;
    }
    int cid;
    char buff[10];
    struct flock lock;
    while (1)
    {
        write(clientSocket, "Enter the course ID number to Drop: ", 36);
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
            cid = atoi(buff);
            if (cid <= 0 || getCount(3) < cid)
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
    for (int i = 0; i < cnt; i++)
    {
        if (enrolled[i] == cid)
        {
            flag = 1;
            break;
        }
    }
    if (enrolled) free(enrolled);
    if (flag == 0)
    {
        char skip[2];
        write(clientSocket, "Course not Enrolled...$\n", 24);
        read(clientSocket, skip, 2);
        close(fd);
        return;
    }

    struct Enrollment e = getStudentEnrollment(clientSocket, session, cid);
    int eid = e.enrollmentID;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (eid - 1) * sizeof(struct Enrollment);
    lock.l_len = sizeof(struct Enrollment);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock on Enrollment Record!");
        close(fd);
        return;
    }
    if (e.isEnrolled)
    {
        setAvailability(clientSocket, cid, 1);  /* restore seat using course ID */
        e.isEnrolled = 0;
        lseek(fd, (eid - 1) * sizeof(struct Enrollment), SEEK_SET);
        write(fd, &e, sizeof(struct Enrollment));
        char skip[2];
        write(clientSocket, "Course dropped successfully\n$\n", 30);
        read(clientSocket, skip, 2);
    }
    else
    {
        char skip[2];
        write(clientSocket, "Enrollment not active...$\n", 26);
        read(clientSocket, skip, 2);
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void changePassword(int clientSocket, int session)
{
    int fd = open("files/studentDetails", O_RDWR);
    if (fd == -1)
    {
        exit(1);
    }
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (session - 1) * sizeof(struct Student);
    lock.l_len = sizeof(struct Student);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining write lock for password change");
        close(fd);
        return;
    }
    lseek(fd, (session - 1) * sizeof(struct Student), SEEK_SET);
    struct Student st;
    memset(&st, 0, sizeof(st));
    read(fd, &st, sizeof(st));

    /* Verify current password */
    char current[50];
    memset(current, 0, sizeof(current));
    write(clientSocket, "Enter current password: ", 24);
    int n = read(clientSocket, current, sizeof(current) - 1);
    if (n > 0) current[n] = '\0';

    if (strcmp(st.password, current) != 0)
    {
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        char skip[2];
        write(clientSocket, "Incorrect current password.\n$", 29);
        read(clientSocket, skip, 2);
        return;
    }

    char newPass[50];
    memset(newPass, 0, sizeof(newPass));
    write(clientSocket, "Enter new password: ", 20);
    n = read(clientSocket, newPass, sizeof(newPass) - 1);
    if (n > 0) newPass[n] = '\0';

    if (strlen(newPass) == 0)
    {
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        char skip[2];
        write(clientSocket, "Password cannot be empty.\n$", 27);
        read(clientSocket, skip, 2);
        return;
    }

    strncpy(st.password, newPass, sizeof(st.password) - 1);
    st.password[sizeof(st.password) - 1] = '\0';
    lseek(fd, (session - 1) * sizeof(struct Student), SEEK_SET);
    write(fd, &st, sizeof(st));

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    char skip[2];
    write(clientSocket, "Password changed successfully.\n$", 32);
    read(clientSocket, skip, 2);
}

#endif
