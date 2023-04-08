#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<netdb.h>
#include<string.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <sys/time.h>


int checksum(void *b, int len){
    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}


int main(){
    char *buf;
    size_t len = 0;
    getline(&buf, &len, stdin);
    buf[strlen(buf) - 1] = '\0'; // remove the newline character
    printf("Host name: %s\n", buf);
    struct hostent *host = gethostbyname(buf);
    if(host == NULL){
        printf("Error in gethostbyname");
        exit(1);
    }
    printf("IP address: %s\n", inet_ntoa(*((struct in_addr *)(host->h_addr_list[0]))));

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sockfd < 0){
        printf("Error in socket");
        exit(1);
    }
    char buff[1024];
    for(int i=0; ; i++){
        //We make the ping packet
        struct icmphdr *icmp = (struct icmphdr *) buff;
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.id = getpid();
        icmp->un.echo.sequence = i;
        char* data = buff + sizeof(struct icmphdr);
        //Get the time in microseconds
        struct timeval tv;
        gettimeofday(&tv, NULL);
        long int time = tv.tv_sec * 1000000 + tv.tv_usec;
        memcpy(data, &time, sizeof(time));
        int size = sizeof(struct icmphdr) + sizeof(time);
        icmp->checksum = checksum(icmp, size);

        //We send the packet
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)(host->h_addr_list[0]))));
        if(sendto(sockfd, buff, size, 0, (struct sockaddr *) &addr, sizeof(addr)) < 0){
            printf("Error in sendto");
            exit(1);
        }
        //We receive the packet
        struct sockaddr_in addr2;
        socklen_t len = sizeof(addr2);
        if(recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *) &addr2, &len) < 0){
            printf("Error in recvfrom");
            exit(1);
        }
        //We print the whole packet
        struct iphdr *ip = (struct iphdr *) buff;
        struct icmphdr *icmp2 = (struct icmphdr *) (buff + ip->ihl * 4);
        gettimeofday(&tv, NULL);
        long int time2 = tv.tv_sec * 1000000 + tv.tv_usec;
        time = *((long int *) (buff + ip->ihl * 4 + sizeof(struct icmphdr)));
        printf("RTT: %ld us\n", time2 - time);
        
        sleep(1);
    }



    return 0;
}