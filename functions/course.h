#ifndef COURSE_H
#define COURSE_H

struct Course getCourses(int clientSocket, int id)
{
    struct Course c;
    int fd = open("files/courses", O_RDONLY);
    if (fd == -1)
    {
        exit(1);
    }
    if (getCount(3) < id)
    {
        write(clientSocket, "Wrong Course id..~\n", 20);
        return c;
    }
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (id - 1) * sizeof(struct Course);
    lock.l_len = sizeof(struct Course);
    lock.l_pid = getpid();
    int f = fcntl(fd, F_SETLKW, &lock);
    if (f == -1)
    {
        perror("Error obtaining Read lock on Courses Record!");
        close(fd);
    }
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    int bytesRead;
    read(fd, &c, sizeof(struct Course));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    return c;
}

void setAvailability(int clientSocket, int id, int cnt)
{
    int fd = open("files/courses", O_RDWR);
    if (fd == -1)
    {
        exit(1);
    }
    if (getCount(3) < id)
    {
        write(clientSocket, "Wrong Course id..$\n", 20);
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
        perror("Error obtaining Read lock on Courses Record!");
        close(fd);
    }
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    struct Course c;
    int bytesRead;
    read(fd, &c, sizeof(struct Course));
    c.available += cnt;
    lseek(fd, (id - 1) * sizeof(struct Course), SEEK_SET);
    write(fd, &c, sizeof(struct Course));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    return;
}
#endif