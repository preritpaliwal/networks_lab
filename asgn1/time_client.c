#include <stdio.h>

#include <sys/socket.h>

#include <stdlib.h>

#include <netinet/in.h>

#include <string.h>
// for inet_aton function
#include <arpa/inet.h>

#include <unistd.h>

int main(){
    int sockFD = socket(AF_INET,SOCK_STREAM,0);
    if(sockFD<0){
        perror("failed in creating a socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&serv_addr.sin_addr);
    serv_addr.sin_port = htons(20000);


    if(connect(sockFD,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("failed to connect..!!");
        exit(EXIT_SUCCESS);
    }

    char buffer[100];
    for(int i = 0;i<100;i++){
        buffer[i] = '\0';
    }

    recv(sockFD,buffer,100,0);
    printf("The Time sent by server is: %s",buffer);

    strcpy(buffer,"Message successfully received");
    send(sockFD,buffer,strlen(buffer)+1,0);

    close(sockFD);
    return 0;
}