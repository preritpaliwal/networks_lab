#include <stdio.h>

#include <stdlib.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <unistd.h>

#include <string.h>

#define BUFFER_SIZE 50

int main(){
    int sockFD = socket(AF_INET,SOCK_STREAM,0);
    if(sockFD<0){
        perror("failed to create socket..!!");
        exit(EXIT_SUCCESS);
    }
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(20000);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockFD,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        perror("failed to bind socket to the given IP address and port");
        exit(EXIT_SUCCESS);
    }

    listen(sockFD,5);

    struct sockaddr_in cli_addr;
    int len_cli = sizeof(cli_addr);
    int newSockFD = accept(sockFD,(struct sockaddr *)&cli_addr, &len_cli);

    if(newSockFD < 0){
        perror("failed to accept the client request");
        exit(EXIT_SUCCESS);
    }

    while(1){
        char buffer[BUFFER_SIZE];
        int rec = recv(newSockFD,buffer,BUFFER_SIZE,0);
        printf("value of recv: %d\n",rec);
        if(rec==0){
            break;
        }
        printf("Client says: %s\n",buffer);
        char *c = "69";
        int sen = send(newSockFD,c,strlen(c)+1,0);
        printf("value of send: %d\n",sen);
    }
    return 0;
}