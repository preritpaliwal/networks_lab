#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>

#include "mysocket.h"

#define MY_PORT 20001   
#define BUFFER_SIZE 5000

void clear(char* buffer, int size)
{
    for(int i=0;i<size;i++)
        buffer[i] = '\0';
}

int main()
{
    MyFD *sockfd;
    struct sockaddr_in cli_addr, serv_addr;
    sockfd = my_socket(AF_INET, SOCK_MyTCP, 0);
    if (sockfd->sock_fd < 0)
    {
        perror("Unable to create a MyTCP socket.");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(MY_PORT);

    if ( (my_bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) )) < 0 ){
        perror("Unable to bind local address.");
        exit(0);
    }

    my_listen(sockfd, 5);


    // while(1){

        socklen_t clilen;


        MyFD* newsockfd = my_accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        // printf("SOCKFD : %d\n", sockfd->sock_fd);
        // printf("NEWSOCKFD: %d\n", newsockfd->sock_fd);
        // Change my_accept to be of MyFD* type.

        if(newsockfd->sock_fd < 0){
           perror("Accept error.");
           exit(0); 
        }

        char buffer[BUFFER_SIZE];
        clear(buffer, BUFFER_SIZE);

        strcpy(buffer,"This is a test string being sent from the server. MSG 1");
        my_send(newsockfd, buffer, strlen(buffer)+1, 0);
        sleep(2);
        
        strcpy(buffer,"This is a test string being sent from the server. MSG 2");
        my_send(newsockfd, buffer, strlen(buffer)+1, 0);
        sleep(2);
        
        strcpy(buffer,"This is a test string being sent from the server. MSG 3");
        my_send(newsockfd, buffer, strlen(buffer)+1, 0);
        sleep(2);
        
        strcpy(buffer,"This is a test string being sent from the server. MSG 4");
        my_send(newsockfd, buffer, strlen(buffer)+1, 0);
        sleep(2);
        
        strcpy(buffer,"This is a test string being sent from the server. MSG 5");
        my_send(newsockfd, buffer, strlen(buffer)+1, 0);
        sleep(2);
        

        my_recv(newsockfd, buffer, BUFFER_SIZE, 0);
        printf("Message from Client: %s\n", buffer);


        my_recv(newsockfd, buffer, BUFFER_SIZE, 0);
        printf("Message from Client: %s\n", buffer);

        my_recv(newsockfd, buffer, BUFFER_SIZE, 0);
        printf("Message from Client: %s\n", buffer);

        my_recv(newsockfd, buffer, BUFFER_SIZE, 0);
        printf("Message from Client: %s\n", buffer);

        my_recv(newsockfd, buffer, BUFFER_SIZE, 0);
        printf("Message from Client: %s\n", buffer);
        // sleep(2);

        my_close(newsockfd);
        
    // }

    my_close(sockfd);

    return 0;
}