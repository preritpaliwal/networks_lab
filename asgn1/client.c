#include <stdio.h>

#include <stdlib.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <string.h>

#include <arpa/inet.h>

#define BUFFER_SIZE 50

int main(){
    int sockFD = socket(AF_INET,SOCK_STREAM,0);
    if(sockFD<0){
        perror("failed in creating a socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(20000);
    inet_aton("127.0.0.1",&serv_addr.sin_addr);

    if(connect(sockFD,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        perror("failed to connect");
        exit(EXIT_SUCCESS);
    }

    while(1)
    {
        char expression[100];
        printf("\nenter an expression: ");
        scanf("%[^\n]s",expression);
        char newline;
        scanf("%c",&newline);
        int len = strlen(expression);
        printf("len = %d\nexprsssion: %s hehe%c",len,expression,newline);
        if(len==2){
            if(expression[0]=='-' && expression[1]=='1'){
                break;
            }
        }

        int bytesSent = 0;
        char buffer[BUFFER_SIZE];
        while(len>0){
            int lenbuf = -1;
            if(len+1>BUFFER_SIZE){
                for(int i = 0;i<BUFFER_SIZE-1;i++){
                    buffer[i] = expression[i+bytesSent];
                }
                buffer[49] = '\0';
                lenbuf = BUFFER_SIZE-1;
            }
            else{
                for(int i = 0;i<len;i++){
                    buffer[i] = expression[i+bytesSent];
                }
                buffer[len] = '\0';
                lenbuf = len;
            }
            int returnValOfSend = send(sockFD,buffer,lenbuf+1,0);
            printf("returnValOfSend: %d",returnValOfSend);
            len -= (lenbuf);
            bytesSent += lenbuf;
        }

        for(int i = 0;i<BUFFER_SIZE;i++){
            buffer[i] = '\0';
        }
        int returnValOfRecv = recv(sockFD,buffer,100,0);
        printf("returnValOfRecv: %d",returnValOfRecv);
        printf("Value of expression: %s = %s",expression,buffer);
    }
    printf("Byeee, I hope you were able to solve all the expressions!!");
}