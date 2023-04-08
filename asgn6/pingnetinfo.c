// STUDY ICMP TO WRITE THIS

// two threads - one for sending, one for receiving 
// save time of sending and use that to calculate RTT after receiving


#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>


int isValidIP(char ip[]){
    int len = strlen(ip);
    int num = 0;
    for(int i = 0;i<len;i++){
        if(ip[i]=='.'){
            if(num>255){
                return 0;
            }
            num = 0;
        }
        else{
            if(ip[i]<'0' || ip[i] > '9'){
                return 0;
            }
            num = num*10 + ip[i]-'0';
        }
    }
    if(num>255){
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]){
    if(argc!=4){
        printf("Usage: %s <IP address/Domain Name Server> <Number of Probes to be sent per link> <Time difference between any two probes>\n",argv[0]);
        exit(EXIT_SUCCESS);
    }
    
    for(int i =1;i<argc;i++){
        if(isValidIP(argv[i])){
            // use IP as it is
        }
        else{
            // get IP from gethostbyname

            // struct hostent *hp = gethostbyname(argv[1]);
            // if (!hp)
            //     printf("unknown host %s\n", argv[1]);
            // else
            // {
            //     to.sin_family = hp->h_addrtype;
            //     memcpy(&(to->sin_addr.s_addr), hp->h_addr, hp->h_length);
            //     hostname = hp->h_name;
            //     printf("gethostbyname was successful\n");
            // }
        }
    }

}