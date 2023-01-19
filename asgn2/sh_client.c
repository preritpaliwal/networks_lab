#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 20000
#define MAX_LEN 1024
#define IO_BUFFER_LEN 5
#define MAX_USERNAME_LEN 25

void cleanBuffer(char *buffer,int len){
    for(int i = 0;i<len;i++){
        buffer[i] = '\0';
    }
}

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

void stripInStart(char *buffer,int len){
    int dif = 0;
    for(int i = 0;i<len;i++){
        if(buffer[i]==' '){
            dif++;
        }
        else{
            break;
        }
    }
    if(dif>0){
        for(int i = dif;i<len+1;i++){
            buffer[i-dif] = buffer[i];
        }
    }
}

int main(){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("failed to create socket..!!");
        exit(EXIT_SUCCESS);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_aton("127.0.0.1",&serv_addr.sin_addr);

    if(connect(sockfd,(const struct sockaddr *)&serv_addr,sizeof(serv_addr))){
        perror("failed to connet..!!");
        exit(EXIT_SUCCESS);
    }
    printf("connected\n");
    /*
    Todo:
    receive login string and display
    take input username (max len = 25)
    send username to server    
    if: 
    NOT-FOUND display error message = "Invalid username" and close connection
    else: 
    receive a command from the keyboard and send to server
    receive result from the server and display to user
    if commmand was invalid print "Invalid command"
    if error in running command print "Error in running command"
    prompt for next command
    if user enters exit client closes

    set of commands 
    pwd: getcwd
    dir: opendir + readdir
    cd: chdir
    */

    int exits = 0;
    while(exits==0){
        char buffer[MAX_LEN];
        // printf("waiting to receive\n");
        readMsg(sockfd,buffer);
        printf("[SERVER] %s",buffer);
        char username[MAX_LEN],pwd[MAX_LEN];
        cleanBuffer(pwd,MAX_LEN);
        scanf("%s",username);        
        sendMsg(sockfd,username);
        readMsg(sockfd,buffer);
        // printf("got this message: %s\n",buffer);
        char *found = "FOUND";
        if(strcmp(found,buffer)==0){
            printf("\t\t\t!!..Connected to server..!!\n\n\t\t\tYou can Execute commands now!\n\n");
            while (exits == 0)
            {
                printf("%s:~%s$ ",username,pwd);
                char *command=NULL;
                size_t len = 0;
                ssize_t lineSize = getline(&command, &len, stdin);
                // scanf("%s",command);
                // printf("got command: %s%ld\n",command,strlen(command));
                stripInStart(command,strlen(command));
                if(strlen(command)<2){
                    printf("\n");
                    continue;
                }
                char *e = "exit";
                
                for(int i = 0;i<strlen(command);i++){
                    if(command[i]=='\n'){
                        command[i] = '\0';
                    }
                }

                // printf("%ld\n",strlen(command));

                sendMsg(sockfd,command);
                if(strcmp(e,command)==0){
                    exits = 1;
                    break;
                }
                readMsg(sockfd,buffer);
                if(command[0]=='p' && command[1]=='w' && command[2]=='d' || command[0]=='c' && command[1]=='d' && buffer[0]!='#'){
                    for(int i = 0;i<strlen(buffer);i++){
                        pwd[i] = buffer[i];
                    }
                    pwd[strlen(buffer)] = '\0';
                }
                printf("%s\n",buffer);
            }
        }
        else{
            printf("Invalid Username\n");
            exits = 1;
        }
    }
    printf("Closing connection with server..!!\n");
    close(sockfd);
    return 0;
}