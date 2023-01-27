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
#include <poll.h>
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

int main(int argc, char ** argv){
    if(argc!=4){
        perror("wrong argument list");
        exit(EXIT_SUCCESS);
    }
    int CLI_PORT = atoi(argv[1]);
    int SERV_PORT1 = atoi(argv[2]);
    int SERV_PORT2 = atoi(argv[3]);

    // printf("port= %d",CLI_PORT);
    // printf("port= %d",SERV_PORT1);
    // printf("port= %d\n",SERV_PORT2);

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("failed in creating socket..!!");
        exit(EXIT_SUCCESS);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(CLI_PORT);

    if(bind(sockfd,(const struct sockaddr *)&serv_addr,sizeof(serv_addr))){
        perror("failed to bind..!!");
        exit(EXIT_SUCCESS);
    }

    int serv1sockFD = socket(AF_INET,SOCK_STREAM,0);
    if(serv1sockFD<0){
        perror("failed in creating a socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv1_addr;
    serv1_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&serv1_addr.sin_addr);
    serv1_addr.sin_port = htons(SERV_PORT1);

    int serv2sockFD = socket(AF_INET,SOCK_STREAM,0);
    if(serv2sockFD<0){
        perror("failed in creating a socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv2_addr;
    serv2_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&serv2_addr.sin_addr);
    serv2_addr.sin_port = htons(SERV_PORT2);

    struct pollfd fds[1];
    int nfds = 1;

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    int timeout = (5 * 1000);

    listen(sockfd, 5);
    int load1,load2 = 0;


    char buffer[MAX_LEN];
    cleanBuffer(buffer,MAX_LEN);
    strcpy(buffer,"Send Load");
    if(connect(serv1sockFD,(const struct sockaddr *)&serv1_addr, sizeof(serv1_addr)) < 0){
        perror("failed to connect..1!!");
        exit(EXIT_SUCCESS);
    }
    sendMsg(serv1sockFD, buffer);
    readMsg(serv1sockFD, buffer);
    load1 = atoi(buffer);
    close(serv1sockFD);

    cleanBuffer(buffer,MAX_LEN);
    strcpy(buffer,"Send Load");
    if(connect(serv2sockFD,(const struct sockaddr *)&serv2_addr, sizeof(serv2_addr)) < 0){
        perror("failed to connect..!!");
        exit(EXIT_SUCCESS);
    }
    sendMsg(serv2sockFD, buffer);
    readMsg(serv2sockFD, buffer);
    load2 = atoi(buffer);
    close(serv2sockFD);



    // updateLoad(serv1sockFD,(struct sockaddr *)&serv1_addr,serv2sockFD,(struct sockaddr *)&serv2_addr,&load1,&load2);
    int LastUpdate = time(0);
    while (1)
    {   
        printf("load of server 1 = %d,load of server 2 = %d\n",load1,load2);
        int rc = poll(fds, nfds, timeout);
        // printf("rc = %d", rc);
        if (rc < 0)
        {
            perror("  poll() failed");
            break;
        }
        else if (rc == 0)
        {
            // printf("  poll() timed out.  \n");
            char buffer[MAX_LEN];
            cleanBuffer(buffer,MAX_LEN);
            strcpy(buffer,"Send Load");
            serv1sockFD = socket(AF_INET,SOCK_STREAM,0);
            if(connect(serv1sockFD,(const struct sockaddr *)&serv1_addr, sizeof(serv1_addr)) < 0){
                perror("failed to connect..2!!");
                exit(EXIT_SUCCESS);
            }
            sendMsg(serv1sockFD, buffer);
            readMsg(serv1sockFD, buffer);
            load1 = atoi(buffer);
            close(serv1sockFD);

            cleanBuffer(buffer,MAX_LEN);
            strcpy(buffer,"Send Load");
            serv2sockFD = socket(AF_INET,SOCK_STREAM,0);
            if(connect(serv2sockFD,(const struct sockaddr *)&serv2_addr, sizeof(serv2_addr)) < 0){
                perror("failed to connect..!!");
                exit(EXIT_SUCCESS);
            }
            sendMsg(serv2sockFD, buffer);
            readMsg(serv2sockFD, buffer);
            load2 = atoi(buffer);
            close(serv2sockFD);

            // updateLoad(serv1sockFD,(struct sockaddr *)&serv1_addr,serv2sockFD,(struct sockaddr *)&serv2_addr,&load1,&load2);
            LastUpdate = time(0);
            timeout = 5*1000;
        }
        else if (fds[0].revents == POLLIN){
            struct sockaddr_in cli_addr;
            int clilen = sizeof(cli_addr);
            printf("waiting for connection\n");
            int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,&clilen);
            
            if(newsockfd < 0){
                perror("failed to accept..!!");
                exit(EXIT_SUCCESS);
            }
            
            if(fork()==0)
            {
                printf("Connection made with a client..!!\n");
                close(sockfd);
                char buffer[MAX_LEN];
                if(load1<load2){
                    printf("Sending Client Request to IP: 127.0.0.1 PORT: %d\n",SERV_PORT1);
                    cleanBuffer(buffer,MAX_LEN);
                    strcpy(buffer,"Send Time");
                    serv1sockFD = socket(AF_INET,SOCK_STREAM,0);
                    if(connect(serv1sockFD,(struct sockaddr *)&serv1_addr, sizeof(serv1_addr)) < 0){
                        perror("failed to connect..!!");
                        exit(EXIT_SUCCESS);
                    }
                    sendMsg(serv1sockFD, buffer);
                    readMsg(serv1sockFD, buffer);
                }
                else{
                    printf("Sending Client Request to IP: 127.0.0.1 PORT: %d\n",SERV_PORT2);
                    cleanBuffer(buffer,MAX_LEN);
                    strcpy(buffer,"Send Time");
                    serv2sockFD = socket(AF_INET,SOCK_STREAM,0);
                    if(connect(serv2sockFD,(struct sockaddr *)&serv2_addr, sizeof(serv2_addr)) < 0){
                        perror("failed to connect..!!");
                        exit(EXIT_SUCCESS);
                    }
                    sendMsg(serv2sockFD, buffer);
                    readMsg(serv2sockFD, buffer);
                }
                sendMsg(newsockfd,buffer);
                printf("closing connection with this client..!!\n");
                close(newsockfd);
                exit(EXIT_SUCCESS);
            }

            timeout -= (time(0)-LastUpdate)*1000;
            if(timeout<0){
                char buffer[MAX_LEN];
                cleanBuffer(buffer,MAX_LEN);
                strcpy(buffer,"Send Load");
                serv1sockFD = socket(AF_INET,SOCK_STREAM,0);
                if(connect(serv1sockFD,(const struct sockaddr *)&serv1_addr, sizeof(serv1_addr)) < 0){
                    perror("failed to connect..3!!");
                    exit(EXIT_SUCCESS);
                }
                sendMsg(serv1sockFD, buffer);
                readMsg(serv1sockFD, buffer);
                load1 = atoi(buffer);
                close(serv1sockFD);

                cleanBuffer(buffer,MAX_LEN);
                strcpy(buffer,"Send Load");
                serv2sockFD = socket(AF_INET,SOCK_STREAM,0);
                if(connect(serv2sockFD,(const struct sockaddr *)&serv2_addr, sizeof(serv2_addr)) < 0){
                    perror("failed to connect..!!");
                    exit(EXIT_SUCCESS);
                }
                sendMsg(serv2sockFD, buffer);
                readMsg(serv2sockFD, buffer);
                load2 = atoi(buffer);
                close(serv2sockFD);
                // updateLoad(serv1sockFD,(struct sockaddr *)&serv1_addr,serv2sockFD,(struct sockaddr *)&serv2_addr,&load1,&load2);
                LastUpdate = time(0);
                timeout = 5*1000;
            }
        }
    }
    
    return 0;
}