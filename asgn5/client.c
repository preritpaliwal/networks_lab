#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "mysocket.h"

#define MY_PORT 20000
#define BUFFER_SIZE 5000

void clear(char* buffer, int size){
    for(int i=0;i<size;i++)
        buffer[i] = '\0';
}

int main()
{

    MyFD *sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0)
    {
        perror("Unable to create a MyTCP socket.");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(MY_PORT);

    if ((my_connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    char buffer[BUFFER_SIZE];
    clear(buffer, BUFFER_SIZE);

    my_recv(sockfd, buffer, BUFFER_SIZE, 0);
    printf("Message from Server: %s\n", buffer);

    strcpy(buffer,"This is a test string being sent from the client.");
    my_send(sockfd, buffer, strlen(buffer)+1, 0);

    // sleep(2);

    my_close(sockfd);
    return 0;

}