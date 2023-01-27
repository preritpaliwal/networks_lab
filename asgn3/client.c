/*
Name: Prerit Paliwal
Roll Number: 20CS10046
Assignment 3
*/

#include <stdio.h>

#include <sys/socket.h>

#include <stdlib.h>

#include <netinet/in.h>

#include <string.h>
// for inet_aton function
#include <arpa/inet.h>

#include <unistd.h>

#define IO_BUFFER_LEN 50
#define MAX_LEN 1024

// function to clean the buffer
void cleanBuffer(char *buffer,int len){
    for(int i = 0;i<len;i++){
        buffer[i] = '\0';
    }
}

// function to send a given msg in chunks
int sendMsg(int fd,char *msg){
    int len = strlen(msg);
    char buffer[IO_BUFFER_LEN];
    cleanBuffer(buffer,IO_BUFFER_LEN);
    int lenSent = 0;
    // printf("len = %d",len);
    while (lenSent<len)
    {
        // printf("len sent: %d\n",lenSent);
        if(len+1<=IO_BUFFER_LEN){
            for(int i = 0;i<len+1;i++){
                buffer[i] = msg[i+lenSent];
            }
            // printf("about to send this:%s \n",buffer);
            send(fd,buffer,len+1,0);
            lenSent += len;
        }
        else{
            for(int i =0; i<IO_BUFFER_LEN-1;i++){
                buffer[i] = msg[i+lenSent];
            }
            buffer[IO_BUFFER_LEN-1] = '\0';
            // printf("about to send thissss:%s \n",buffer);
            send(fd,buffer,IO_BUFFER_LEN,0);
            lenSent += (IO_BUFFER_LEN-1);
        }
    }
    cleanBuffer(buffer,IO_BUFFER_LEN);
    buffer[0] = '\0';
    // printf("sending last bit\n");
    send(fd, buffer, 1, 0);
    return lenSent+1;
}

// function to read a msg in chunks
int readMsg(int fd, char *msg){
    cleanBuffer(msg,MAX_LEN);
    int read = 1;
    int lenRead = 0;
    while(read){
        char buffer[IO_BUFFER_LEN];
        cleanBuffer(buffer,IO_BUFFER_LEN);
        int rec = recv(fd, buffer, IO_BUFFER_LEN, 0);
        int i = 0;
        for(i = 0;i<IO_BUFFER_LEN;i++){
            if(i!=0){
                if(buffer[i]=='\0' && buffer[i-1]=='\0'){
                    read = 0;
                    break;
                }
            }
            msg[i+lenRead] = buffer[i];
        }
        lenRead += (i-1);
    }
    // printf("received msg: %s%ld\n",msg,strlen(msg));
    if(strlen(msg)==0){
        return readMsg(fd,msg);
    }
    return lenRead;
}

int main(int argc,char **argv){
    int PORT = -1;
    if(argc!=3){
        perror("wrong argument list");
        exit(EXIT_SUCCESS);
    }
    else{
        PORT = atoi(argv[2]);
    }
    int sockFD = socket(AF_INET,SOCK_STREAM,0);
    if(sockFD<0){
        perror("failed in creating a socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&serv_addr.sin_addr);
    serv_addr.sin_port = htons(PORT);


    if(connect(sockFD,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("failed to connect..!!");
        exit(EXIT_SUCCESS);
    }

    char buffer[MAX_LEN];
    for(int i = 0;i<MAX_LEN;i++){
        buffer[i] = '\0';
    }

    readMsg(sockFD,buffer);
    printf("The Time sent by server is: %s",buffer);

    // strcpy(buffer,"Message successfully received");
    // send(sockFD,buffer,strlen(buffer)+1,0);

    close(sockFD);
    return 0;
}