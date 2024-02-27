#ifndef STUDENT_H
#define STUDENT_H

void viewCourses(int clientSocket);
void enrollCourse(int clientSocket, int session);
int *getEnrollments(int clientSocket, int session, int *cnt, int flag);
void dropCourse(int clientSocket, int session);

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
        write(clientSocket, STUDENT_MENU, sizeof(STUDENT_MENU) + 1);
        memset(resp, 0, 4);
        if (read(clientSocket, resp, sizeof(resp)) == 0)
        {
            write(clientSocket, "No input try again...~\n", 24);
            break;
        }
        else if (resp[0] == '$')
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
                viewCourses(clientSocket);
                break;
            case 2:
                enrollCourse(clientSocket, session);
                break;
            case 3:
                dropCourse(clientSocket, session);
                break;
            case 4:
                int cnt;
                char skip[2];
                getEnrollments(clientSocket, session, &cnt, 1);
                if (cnt == 0)
                {
                    write(clientSocket, "No Courses Enrolled...$\n", 25);
                    read(clientSocket, skip, 2);
                }
                break;
            case 5:
                write(clientSocket, "Exiting...~\n", 13);
                return;
                break;
            default:
                printf("Invalid choice. Please select a valid option (1-9).\n");
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
    }
    lseek(fd, 0, SEEK_SET);
    int courseCnt = getCount(3);
    if (courseCnt == 0)
    {
        write(clientSocket, "No Courses Available...~=$\n", 26);
    }
    struct Course c;
    int bytesRead;
    while ((bytesRead = read(fd, &c, sizeof(struct Course))) == sizeof(struct Course) || bytesRead == -1)
    {
        if (c.isActive)
        {
            char send[100 + sizeof(struct Course)], skip[2];
            sprintf(send, "Course Name: %s\nDepartment: %s\nCredits: %d\nTotal seats: %d\nAvailable Seats: %d\nId: %d\n~\n", c.cname, c.department, c.credits, c.seats, c.available, c.id);
            write(clientSocket, send, strlen(send) + 1);
            read(clientSocket, skip, 2);
            memset(send, 0, 100 + sizeof(struct Course));
        }
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
        write(clientSocket, "No Courses Available...$\n", 26);
        return;
    }
    int id;
    char buff[10];
    while (1)
    {
        write(clientSocket, "Enter the course ID number to Enroll: ", 39);
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
    int cnt;
    int *enrolled = getEnrollments(clientSocket, session, &cnt, 0);
    int flag = 0;
    for (int i = 0; i < cnt; i++)
    {
        if (enrolled[i] == id)
        {
            flag = 1;
            break;
        }
    }
    if (flag == 1)
    {
        write(clientSocket, "Course already Enrolled...\n$\n", 30);
        return;
    }
    else
    {
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
        struct Course c = getCourses(clientSocket, id);
        // Implement funcion
        if (c.available > 0)
        {
            int enrollId = setCount(4);
            setAvailability(clientSocket, id, -1);
            struct Enrollment e = {session, c.id, c.profId, enrollId, 1};
            write(fd, &e, sizeof(struct Enrollment));
            char tempBuffer[100];
            sprintf(tempBuffer, "\nSuccessfully Enrolled in the course - %s\n$\n", c.cname);
            write(clientSocket, tempBuffer, strlen(tempBuffer) + 1);
        }

        else
        {
            char skip[2];
            write(clientSocket, "No seats left...\n$\n", 20);
            read(clientSocket, skip, 2);
        }
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
    close(fd);
}

int *getEnrollments(int clientSocket, int session, int *cnt, int flag)
{
    int fd = open("files/enrollments", O_RDONLY);
    if (fd == -1)
    {
        exit(1);
    }
    int courseCnt = getCount(3);
    if (courseCnt == 0)
    {
        write(clientSocket, "No Courses Available...$\n", 26);
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
    struct Enrollment e;
    int bytesRead;
    int arraySize = 1;
    int count = 0;
    int *cid = (int *)malloc(arraySize * sizeof(int));
    while ((bytesRead = read(fd, &e, sizeof(struct Enrollment))) == sizeof(struct Enrollment) || bytesRead == -1)
    {

        if (e.isEnrolled && e.studentID == session)
        {
            char send[100 + sizeof(struct Course)], skip[2];
            struct Course c = getCourses(clientSocket, e.courseID);
            if (flag)
            {
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
            cid[count++] = e.courseID;
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    *cnt = count;
    return cid;
}

void dropCourse(int clientSocket, int session)
{
    int fd = open("records/enrollments", O_RDWR);
    if (fd == -1)
    {
        exit(1);
    }
    int courseCnt = getCount(3);
    if (courseCnt == 0)
    {
        write(clientSocket, "No Courses Available...$\n", 26);
        return;
    }
    int cnt;
    int *enrolled = getEnrollments(clientSocket, session, &cnt, 0);
    if (cnt == 0)
    {
        write(clientSocket, "No Courses Enrolled...$\n", 25);
        return;
    }
    int cid;
    char buff[10];
    struct flock lock;
    while (1)
    {
        write(clientSocket, "Enter the course ID number to Enroll: ", 39);
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
            cid = atoi(buff);
            if (cid <= 0 || getCount(3) < cid)
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
    for (int i = 0; i < cnt; i++)
    {
        if (enrolled[i] == cid)
        {
            flag = 1;
            break;
        }
    }
    if (flag == 0)
    {
        write(clientSocket, "Course not Enrolled...\n$\n", 26);
        return;
    }
    else
    {
        struct Enrollment e = getStudentEnrollment(clientSocket, session, cid);
        int id = e.enrollmentID;
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = (id - 1) * sizeof(struct Enrollment);
        lock.l_len = sizeof(struct Enrollment);
        lock.l_pid = getpid();
        int f = fcntl(fd, F_SETLKW, &lock);
        if (f == -1)
        {
            perror("Error obtaining write lock on Enrollment Record!");
            close(fd);
        }
        if (e.isEnrolled)
        {
            setAvailability(clientSocket, id, 1);
            e.isEnrolled = 0;
            lseek(fd, (id - 1) * sizeof(struct Enrollment), SEEK_SET);
            write(fd, &e, sizeof(struct Enrollment));
        }
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

#endif