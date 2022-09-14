#include <stdio.h>
#include <stdlib.h>
#include "syslog.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>




int main(int argc, char * argv[])
{
    int fd;
    ssize_t rv;
    char * filename;
    char * content;
    /* check parameter number */
    if (argc < 3)
    {
        /* error output */
        syslog(LOG_ERR, "number of parameters should be greater than 2\n");
        exit(1);
    }
    /* get files name and content */
    filename = argv[1];
    content = argv[2];
    printf("filename:%s content:%s", filename, content);
    openlog(NULL, 0, LOG_USER);
    /* open or create a file */
    fd = open(filename, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0)
    {
        syslog(LOG_ERR, "Failed to open file\n");
        exit(1);
    }

    rv = write(fd, content, strlen(content));
    if (rv < 0)
    {
        syslog(LOG_ERR, "Failed to write content to file:%s\n", filename);
        exit(1);
    }

    /* write the content to the file */
    exit(0);
}
