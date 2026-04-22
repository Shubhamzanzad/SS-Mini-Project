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
    memset(&admin, 0, sizeof(admin));
    strcpy(admin.userName, "root");
    strcpy(admin.password, "root");

    int fd = open("files/adminCred", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("Failed to open adminCred file");
        exit(1);
    }
    if (write(fd, &admin, sizeof(struct Admin)) == -1)
    {
        perror("Failed to write to the adminCred file");
        close(fd);
        exit(1);
    }
    close(fd);

    fd = open("files/accountCnt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("Failed to open accountCount file");
        exit(1);
    }
    struct record rec = {0, 0, 0, 0};
    if (write(fd, &rec, sizeof(rec)) == -1)
    {
        perror("Failed to write to the accountCount file");
        close(fd);
        exit(1);
    }
    close(fd);

    /* Truncate all data files for a clean slate */
    const char *dataFiles[] = {
        "files/studentDetails",
        "files/teacherDetails",
        "files/courses",
        "files/enrollments"
    };
    for (int i = 0; i < 4; i++)
    {
        fd = open(dataFiles[i], O_RDWR | O_CREAT | O_TRUNC, 0644);
        if (fd == -1)
        {
            perror("Failed to create data file");
            exit(1);
        }
        close(fd);
    }

    printf("Initialization complete.\n");
    printf("Admin credentials: username=root  password=root\n");
    printf("Default student password: iiitb\n");
    printf("Default professor password: iiitbp\n");
    return 0;
}
