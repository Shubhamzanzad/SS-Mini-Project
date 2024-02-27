#ifndef ENROLLMENT_H
#define ENROLLMENT_H

struct Enrollment getStudentEnrollment(int clientSocket, int sid, int cid)
{
    int fd = open("files/enrollments", O_RDWR | O_APPEND);
    struct Enrollment e = {0, 0, 0, 0, 0};
    if (fd == -1)
    {
        exit(1);
    }
    if (getCount(3) < cid)
    {
        write(clientSocket, "Wrong Course id..~\n", 20);
        return e;
    }
    if (getCount(1) < sid)
    {
        write(clientSocket, "Wrong Student id..~\n", 21);
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
    }
    lseek(fd, 0, SEEK_SET);
    int bytesRead;

    while ((bytesRead = read(fd, &e, sizeof(struct Enrollment))) == sizeof(struct Enrollment) || bytesRead == -1)
    {
        if (e.studentID == sid && e.courseID == cid)
        {
            break;
        }
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    return e;
}

#endif