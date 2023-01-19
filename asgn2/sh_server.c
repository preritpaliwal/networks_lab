#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define PORT 20000
#define MAX_LEN 1024
#define IO_BUFFER_LEN 5
#define MAX_USERNAME_LEN 25

int userExists(char *username){
    FILE *fp = fopen("user.txt","r");
    if(fp==NULL){
        perror("could not open file..!!\n");
        exit(EXIT_SUCCESS);
    }
    else{
        char buffer[MAX_LEN+1];
        while (fgets(buffer,MAX_LEN,fp)!=NULL)
        {
            // printf("%s vs %s",buffer,username);
            int match = 1;
            if(strlen(buffer)-1 == strlen(username)){
                for(int i = 0;i<strlen(username);i++){
                    // printf("%c vs %c\n",username[i],buffer[i]);
                    if(username[i]!=buffer[i]){
                        match = 0;
                        break;
                    }
                }
            }
            else{
                match = 0;
            }
            if(match==1){
                // printf("returning 1 from userexists\n");
                return 1;
            }
        }
        fclose(fp);
        // printf("returning 0 from userexists\n");
        return 0;
    }
}

int getCommandNo(char *buffer,char *arguments){
    int len = strlen(buffer);
    char *pwd = "pwd";
    char *dir = "dir";
    char *cd = "cd";
    int commandNo = 0;
    // printf("len of buffer: %d",len);
    if(len<3){
        return commandNo;
    }
    for(int i = 0;i<len;i++){
        if(buffer[0]==pwd[0] && buffer[1]==pwd[1] && buffer[2]==pwd[2]){
            commandNo = 1;
        }
        else if(buffer[0]==dir[0] && buffer[1]==dir[1] && buffer[2]==dir[2]){
            commandNo = 2;
            if(i<=3){
                continue;
            }
            else{
                arguments[i-4] = buffer[i];
                arguments[i-3] = '\0';
            }
        }
        else if(buffer[0]==cd[0] && buffer[1]==cd[1]){
            commandNo = 3;
            if(i<=2){
                continue;
            }
            else{
                arguments[i-3] = buffer[i];
                arguments[i-2] = '\0';
            }
        }
    }
    return commandNo;
}

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

int dir(char *buffer,char *arguments){
    struct dirent *de; 
    DIR *dr = opendir(arguments);
    if (dr == NULL){
        printf("Could not open current directory");
        return 1;
    }
    int i = 0;
    while ((de = readdir(dr)) != NULL){
        for(int j = 0;j<strlen(de->d_name);j++){
            buffer[i++] = de->d_name[j];
        }
        buffer[i++] = '\t';
    }
    buffer[i++] = '\0';
    closedir(dr);
    return 0;
}

int cd(char *buffer,char *arguments){
    // char argumentsForCd[MAX_LEN];
    // cleanBuffer(argumentsForCd,MAX_LEN);
    // if(arguments[0]!='/'){
    //     argumentsForCd[0] = '/';
    //     for(int i = 0;i<strlen(arguments)+1;i++){
    //         argumentsForCd[i+1] = arguments[i];
    //     }
    //     printf("%s\n%s",argumentsForCd,arguments);
    // }
    // printf("args: %s\n",arguments);
    if(chdir(arguments)!=0){
        perror("chdir() error!!");
        return 1;
    }
    else{
        // printf("directory changed!!\n");
        return pwd(buffer);
        // getCurrentWorkingDirectory(buffer);
    }
}

int pwd(char *buffer){
    if (getcwd(buffer, MAX_LEN) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    else
    {
        // printf("current working directory is: %s\n", buffer);
        return 0;
    }
}

void invalidCommand(char *buffer){
    for(int i = 0;i<4;i++){
        buffer[i] = '$';
    }
    buffer[4] = '\0';
}

void errorInCommand(char *buffer){
    for(int i = 0;i<4;i++){
        buffer[i] = '#';
    }
    buffer[4] = '\0';
}

int main(){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("failed in creating socket..!!");
        exit(EXIT_SUCCESS);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(sockfd,(const struct sockaddr *)&serv_addr,sizeof(serv_addr))){
        perror("failed to bind..!!");
        exit(EXIT_SUCCESS);
    }

    listen(sockfd, 5);

    while(1){
        struct sockaddr_in cli_addr;
        int clilen = sizeof(cli_addr);
        printf("waiting for connection\n");
        int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,&clilen);
        
        if(newsockfd < 0){
            perror("failed to accept..!!");
            exit(EXIT_SUCCESS);
        }

        if(fork()==0){
            printf("Connection made with a client..!!\n");
            close(sockfd);

            /*
            Todo:  
            send string LOGIN
            receive username
            open user.txt and search for the name and send FOUND or NOT-FOUND accordingly
            if:
            we send NOT-Found then end
            else:
            receive command from the client and send the result back to client
            if invalid command then send "$$$$"
            if valid command but error then send "####"
            
            */

            char *login = "LOGIN: ";
            // printf("sending login\n");
            sendMsg(newsockfd,login);
            // printf("login sent\n");
            char username[MAX_LEN];
            readMsg(newsockfd,username);

            if(userExists(username)){
                char *found = "FOUND";
                sendMsg(newsockfd,found);
                char buffer[MAX_LEN];
                int exits = 0;
                char *e = "exit";
                while (exits==0)
                {
                    readMsg(newsockfd,buffer);
                    if(strcmp(e,buffer)==0){
                        exits = 1;
                        break;
                    }
                    char arguments[MAX_LEN];
                    // printf("calling getcommand\n");
                    int commandNo = getCommandNo(buffer,arguments);
                    // printf("commandNo: %d",commandNo);
                    cleanBuffer(buffer,MAX_LEN);
                    int ret = 0;
                    switch (commandNo)
                    {
                    case 1:
                        ret = pwd(buffer);
                        break;
                    case 2:
                        ret = dir(buffer,arguments);
                        break;
                    case 3:
                        ret = cd(buffer,arguments);
                        break;
                    case 0:
                        invalidCommand(buffer);
                        break;
                    
                    default:
                        printf("Exception occured..!!\n");
                        break;
                    }
                    if(ret==1){
                        errorInCommand(buffer);
                    }
                    // printf("after switch buffer: %s\n",buffer);
                    sendMsg(newsockfd,buffer);
                }
                
            }
            else{
                char *notfound = "NOT-FOUND";
                sendMsg(newsockfd,notfound);
            }
            printf("closing connection with this client..!!\n");
            close(newsockfd);
            exit(EXIT_SUCCESS);
        }
    }
    close(sockfd);
    return 0;
}