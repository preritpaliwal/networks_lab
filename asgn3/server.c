/*
Name: Prerit Paliwal
Roll Number: 20CS10046
Assignment 3
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <time.h>

#define MAX_LEN 1024
#define IO_BUFFER_LEN 50

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

int main(int argc, char** argv){
    int PORT = -1;
    if(argc!=2){
        perror("wrong argument list");
        exit(EXIT_SUCCESS);
    }
    else{
        PORT = atoi(argv[1]);
    }
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("failed in creating socket..!!");
        exit(EXIT_SUCCESS);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    // printf("port= %d",PORT);

    if(bind(sockfd,(const struct sockaddr *)&serv_addr,sizeof(serv_addr))){
        perror("failed to bind..!!");
        exit(EXIT_SUCCESS);
    }
    srand(time(0));
    char requestTime[] = "Send Time";
    char requestLoad[] = "Send Load";
    listen(sockfd, 5);
    while (1)
    {   
        struct sockaddr_in cli_addr;
        int len_cli = sizeof(cli_addr);
        printf("waiting for clients to connect\n");
        int newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&len_cli);
        if(newsockfd<0){
            perror("failed to accept the client request");
            exit(EXIT_SUCCESS);
        }
        printf("connected\n");

        char buffer[MAX_LEN];
        readMsg(newsockfd,buffer);
        int q1 = strcmp(buffer,requestLoad);
        int q2 = strcmp(buffer, requestTime);
        // printf("bool1 = %d,bool2 = %d\n",q1,q2);
        if(q1==0){
            int randomNumber = 1 + rand()%100;
            cleanBuffer(buffer,MAX_LEN);
            sprintf(buffer,"%d",randomNumber);
            printf("sending load: %d\n",randomNumber);
        }
        else if (q2==0)
        {
            time_t t;
            time(&t);
            char *c = ctime(&t);
            printf("sending time: %s\n",c);
            strcpy(buffer,c);
        }
        else{
            cleanBuffer(buffer,MAX_LEN);
            strcpy(buffer,"Sorry! Cannot provide this service");
        }
        // printf("sending #%s# to client\n",buffer);
        sendMsg(newsockfd,buffer);
        close(newsockfd);

    }
    
    return 0;
}