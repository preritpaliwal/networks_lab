#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netinet/in.h>

#include "mysocket.h"

#define MY_PORT 20000

int main()
{

    MyFD *sockfd;
    struct sockaddr_in cli_addr, serv_addr;

    if ((sockfd = my_socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

    listen(sockfd, 5);


    while(1){

        MyFD* newsockfd;
        socklen_t clilen;
        // Change my_accept to be of MyFD* type.
        int temp = my_accept(newsockfd, (struct sockaddr *) &cli_addr, &clilen);

        if(temp < 0){
           perror("Accept error.");
           exit(0); 
        }

        my_close(newsockfd);
    }

    my_close(sockfd);

    return 0;
}