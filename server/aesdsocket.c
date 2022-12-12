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
          printf("Deleted successfully");
       else
          printf("Unable to delete the file");


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

    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    if (sigaction(SIGTERM, &new_action, NULL) != 0){
        printf("Error %d (%s) registering for SIGTERM", errno, strerror(errno));
    }
    if (sigaction(SIGINT, &new_action, NULL) != 0){
        printf("Error %d (%s) registering for SIGINT", errno, strerror(errno));
    }
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address_server.sin_family = AF_INET;
	address_server.sin_addr.s_addr = INADDR_ANY;
	address_server.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address_server,
			sizeof(address_server))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

    printf("start listening...\n");
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("\n\n****start accepting...\n");
        if ((new_socket
            = accept(server_fd, (struct sockaddr*)&address_client,
                    (socklen_t*)&addrlen))
            < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("accepting succeed.\n");

        /* get client ip */
        if (inet_ntop(AF_INET, &address_client.sin_addr, client_addr, sizeof(client_addr)) == NULL){
            printf("Invalid address/ Address not supported \n");
            return -1;
        }

        /* 1. get data from socket */
        syslog(LOG_ERR, "Accepted connection from %s\n", client_addr);
        printf("Accepted connection from %s\n", client_addr);
#define READ
#ifdef READ
        strlen_buff = 0;
        size_buff = 0;
        do{
            valread = read(new_socket, buffer + strlen_buff, 1024);
            strlen_buff = strlen(buffer);
            size_buff = sizeof(buffer);
            printf("server accepted: %s\nsize=[%d] len=[%d]\n\n", buffer, size_buff, strlen_buff);
        } while(!has_newline(buffer));
#else
        if( recv(new_socket, buffer , 1024 , 0) < 0)
        {
            printf("recv failed\n");
        }
#endif
        /* 2. write recevied data to recording file */
        file_fd = open(record_file_name, O_RDWR | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (file_fd < 0){
            printf("open file failed\n");
        }
        write(file_fd, buffer, strlen_buff);
        memset(buffer, 0, sizeof(buffer));
        close(file_fd);


        /* read whole file of recording and send back to client*/
        FILE *fd = fopen(record_file_name, "r");
        char ch;
        if (fd == NULL){
            printf("fopen failed\n");
        }
        printf("read recording file:\n");
        while((ch = fgetc(fd)) != EOF){
            printf("%c", ch);
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
        //printf("server sending hello\n");
    }
	//printf("Hello message sent\n");

	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}

