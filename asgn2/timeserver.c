#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#define MAXLEN 1024
#define PORT 20000

int main(){
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        perror("failed to create socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv_addr, cli_addr;
    memset(&serv_addr,0,sizeof(serv_addr));
    memset(&cli_addr,0,sizeof(cli_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(sockfd,(const struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        perror("failed to bind the sokcet..!!");
        exit(EXIT_SUCCESS);
    }

    printf("listening from clients..!\n");
    
    while (1)
    {
        char buffer[MAXLEN];
        for(int i = 0;i<MAXLEN;i++){
            buffer[i] = '\0';
        }
        socklen_t len = sizeof(cli_addr);
        printf("waiting to receive..!!\n");
        int n = recvfrom(sockfd,(char *)buffer,MAXLEN,0,(struct sockaddr *)&cli_addr,&len);
        printf("client said: %s",buffer);
        
        char *msg = "server has received your message..!!\n";
        // send(sockfd,msg,strlen(msg)+1,0);
        sendto(sockfd, (const char *)msg, strlen(msg), 0, (const struct sockaddr *)&cli_addr, sizeof(cli_addr));
        printf("msg sent from server..!!\n");
        
    }
    
    close(sockfd);

    return 0;
}