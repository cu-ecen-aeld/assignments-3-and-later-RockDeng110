// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>



#define PORT 9000
int server_fd;
const char* record_file_name = "/var/tmp/aesdsocketdata";

static void skeleton_daemon();

int has_newline(char* str){

    if (strstr(str, "\n") != NULL)
        return 1;
    return 0;
}

static void signal_handler(int signal){
    if (signal == SIGTERM || signal == SIGINT){
        //close socket
        close(server_fd);
        //delete recording file
       if (remove(record_file_name) == 0)
          syslog(LOG_INFO, "Deleted successfully");
       else
          syslog(LOG_WARNING, "Unable to delete the file");


        syslog(LOG_ERR, "Caught signal, exiting");
    }
}

int main(int argc, char const* argv[])
{
	int new_socket, valread;
	struct sockaddr_in address_server;
	struct sockaddr_in address_client;
	int opt = 1;
	int addrlen = sizeof(address_client);
	char buffer[100000] = { 0 };
    char buffer_sending[100000] = { 0 };
    int buffer_count = 0;
	char buffer_addr[10] = { 0 };
	char* hello = "Hi from server";
    char client_addr[INET_ADDRSTRLEN];
    int file_fd;
    struct sigaction new_action;
    int newline_flag = 0;
    int strlen_buff = 0;
    int size_buff = 0;
    int daemon_flag = 0;

    printf("argc=%d\n", argc);
    if (argc >= 2){
        printf("argv[0]=%s, artv[1]=%s\n", argv[0], argv[1]);
        if (strcmp("-d", argv[1]) == 0){
            daemon_flag = 1;
            printf("Daemon mode started\n");
            skeleton_daemon();
        }
    }


    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    if (sigaction(SIGTERM, &new_action, NULL) != 0){
        syslog(LOG_ERR, "Error %d (%s) registering for SIGTERM", errno, strerror(errno));
    }
    if (sigaction(SIGINT, &new_action, NULL) != 0){
        syslog(LOG_ERR, "Error %d (%s) registering for SIGINT", errno, strerror(errno));
    }
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        syslog(LOG_ERR, "socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
        syslog(LOG_ERR, "setsockopt failed");
		exit(EXIT_FAILURE);
	}
	address_server.sin_family = AF_INET;
	address_server.sin_addr.s_addr = INADDR_ANY;
	address_server.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address_server,
			sizeof(address_server))
		< 0) {
        syslog(LOG_ERR, "bind failed");
		exit(EXIT_FAILURE);
	}

    syslog(LOG_INFO, "start listening...\n");
    if (listen(server_fd, 3) < 0) {
        syslog(LOG_ERR, "listen failed.\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        syslog(LOG_INFO, "\n\n****start accepting...\n");
        if ((new_socket
            = accept(server_fd, (struct sockaddr*)&address_client,
                    (socklen_t*)&addrlen))
            < 0) {
            syslog(LOG_ERR, "accept failed.\n");
            exit(EXIT_FAILURE);
        }
        syslog(LOG_INFO, "accepting succeed.\n");

        /* get client ip */
        if (inet_ntop(AF_INET, &address_client.sin_addr, client_addr, sizeof(client_addr)) == NULL){
            syslog(LOG_ERR, "Invalid address/ Address not supported \n");
            return -1;
        }

        /* 1. get data from socket */
        syslog(LOG_INFO, "Accepted connection from %s\n", client_addr);
        syslog(LOG_INFO, "Accepted connection from %s\n", client_addr);
#define READ
#ifdef READ
        strlen_buff = 0;
        size_buff = 0;
        do{
            valread = read(new_socket, buffer + strlen_buff, 1024);
            strlen_buff = strlen(buffer);
            size_buff = sizeof(buffer);
            syslog(LOG_INFO, "server accepted: %s\nsize=[%d] len=[%d]\n\n", buffer, size_buff, strlen_buff);
        } while(!has_newline(buffer));
#else
        if( recv(new_socket, buffer , 1024 , 0) < 0)
        {
            syslog(LOG_ERR, "recv failed\n");
        }
#endif
        /* 2. write recevied data to recording file */
        file_fd = open(record_file_name, O_RDWR | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (file_fd < 0){
            syslog(LOG_ERR, "open file failed\n");
        }
        write(file_fd, buffer, strlen_buff);
        memset(buffer, 0, sizeof(buffer));
        close(file_fd);


        /* read whole file of recording and send back to client*/
        FILE *fd = fopen(record_file_name, "r");
        char ch;
        if (fd == NULL){
            syslog(LOG_ERR, "fopen failed\n");
        }
        syslog(LOG_INFO, "read recording file:\n");
        while((ch = fgetc(fd)) != EOF){
            //syslog(LOG_INFO, "%c", ch);
            buffer_sending[buffer_count] = (char)ch;
            buffer_count++;
        }
        fclose(fd);
        send(new_socket, buffer_sending, strlen(buffer_sending), 0);
        memset(buffer_sending, 0, sizeof(buffer_sending));
        buffer_count = 0;

        if (newline_flag)
        {
            close(file_fd);
            /*close connection*/
            syslog(LOG_ERR, "Closed connection from %s\n", client_addr);
            newline_flag = 0;
        }

        //send(new_socket, hello, strlen(hello), 0);
        //syslog("server sending hello\n");
    }
	//syslog("Hello message sent\n");

	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
    if (daemon_flag){
        syslog (LOG_NOTICE, "First daemon terminated.");
        closelog();
    }

	return 0;
}

static void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

     /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    /*TODO: Implement a working signal handler */
    //signal(SIGCHLD, SIG_IGN);
    //signal(SIGHUP, SIG_IGN);

    ///* Fork off for the second time*/
    //pid = fork();

    ///* An error occurred */
    //if (pid < 0)
    //    exit(EXIT_FAILURE);

    ///* Success: Let the parent terminate */
    //if (pid > 0)
    //    exit(EXIT_SUCCESS);

    /* Set new file permissions */
    //umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("socket daemon", LOG_PID, LOG_DAEMON);
}

