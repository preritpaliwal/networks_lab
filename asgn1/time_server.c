// imported to access time
#include <time.h> 
// for standard input output
#include <stdio.h>
// for exit function
#include <stdlib.h>
// for creating a socket
#include <sys/socket.h>
// has struct sockaddr_in 
#include  <netinet/in.h>
// for string operations
#include <string.h>
// for close function to close connection of socket
#include <unistd.h>



int main()
{
    // AF_INET is the family, SOCK_STREAM is for TCP/IP protocol and 0 in for user applications
    int sockFD = socket(AF_INET,SOCK_STREAM,0);
    if(sockFD<0){
        perror("failed in creating a socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    if(bind(sockFD,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("failed in binding the socket to specified IP Address and Port");
        exit(EXIT_SUCCESS);
    }

    listen(sockFD,5);
    char buffer[100];
    struct sockaddr_in cli_addr;
    int len_client = sizeof(cli_addr);
    int newsockFD = accept(sockFD, (struct sockaddr *)&cli_addr, &len_client);
    if(newsockFD<0){
        perror("failed in accepting connection..!!");
        exit(EXIT_SUCCESS);
    }
    time_t t;
    time(&t);
    char *c = ctime(&t);
    send(newsockFD, c,strlen(c)+1,0);
    recv(newsockFD,buffer,100,0);
    printf("Client says: %s\n",buffer);
    close(newsockFD);
    return 0;
}