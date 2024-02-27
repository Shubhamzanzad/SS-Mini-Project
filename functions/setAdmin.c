#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "../structures.h"


int main()
{
    struct Admin admin;
    strcpy(admin.userName, "root");
    strcpy(admin.password, "root");

    int fd = open("../files/adminCred", O_RDWR);
    if (fd == -1)
    {
        perror("Failed to open adminCred file");
        exit(1);
    }

    int bytesWritten = write(fd, &admin, sizeof(struct Admin));
    if (bytesWritten == -1)
    {
        perror("Failed to write to the adminCred file");
        close(fd);
        exit(1);
    }
    close(fd);

    fd = open("../files/accountCnt", O_RDWR);
    if (fd == -1)
    {
        perror("Failed to open accountCount file");
        exit(1);
    }

    struct record rec = {0, 0, 0, 0};
    bytesWritten = write(fd, &rec, sizeof(rec));
    if (bytesWritten == -1)
    {
        perror("Failed to write to the accountCount file");
        close(fd);
        exit(1);
    }
    close(fd);

    return 0;
}
