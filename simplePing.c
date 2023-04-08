#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <sys/time.h>

void clearBuffer(char *c,int len){
    for(int i = 0;i<len;i++){
        c[i] = '\0';
    }
}

unsigned short cksum(unsigned short *arr, int len){
    unsigned short sum = 0;
    for(int i = 0;i<len;i++){
        sum += arr[i];
    }
    return ~sum;
}

int main(int argc, char **argv){
    if(argc!=2){
        printf("wrong argument!");
        exit(EXIT_FAILURE);
    }
    char *host = argv[argc-1];
    printf("host = %s\n",host);

    char ip[20];
    if(host[0]<'0' || host[0]>'9'){
        struct hostent *h = gethostbyname(host);
        if(h==NULL){
            printf("get host by name failed!");
            exit(EXIT_FAILURE);
        }
        strcpy(ip,inet_ntoa( *(struct in_addr*) h->h_addr_list[0]));
    }
    else{
        strcpy(ip,host);
    }
    printf("IP = %s\n",ip);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sockfd<0){
        printf("socket failed\n");
        exit(EXIT_FAILURE);
    }


// SYNTAX TO USE SETSOCKOPT
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))){
        printf("setsockopt failed\n");
        exit(EXIT_FAILURE);
    }


    // SYNTAX TO USE GETSOCKOPT
    int ttl ;
    int len = 4;

    if(getsockopt(sockfd, IPPROTO_IP, IP_TTL,(void *) &ttl, &len)){
        printf("getsock failed\n");
        exit(EXIT_FAILURE);
    }
    printf("ttl = %d\nlen = %d\n",ttl,len);

    struct sockaddr_in SendAddr, RecvAddr;
    SendAddr.sin_family = AF_INET;
    SendAddr.sin_addr.s_addr = inet_addr(ip);

    while(1){
        struct icmphdr icmpPacket;
        icmpPacket.type = ICMP_ECHO;
        icmpPacket.code = 0;
        icmpPacket.checksum = 0;
        icmpPacket.un.echo.id = (12);
        icmpPacket.un.echo.sequence = 0;
        icmpPacket.checksum = cksum((unsigned short *)&icmpPacket,sizeof(struct icmphdr)/sizeof(unsigned short));
        char *tmp = (char *)&icmpPacket;
        for(int i = 0;i<sizeof(struct icmphdr);i++){
            printf("|%d|",tmp[i]);
        }
        printf("\n");
        if(sendto(sockfd,(void *)&icmpPacket,sizeof(struct icmphdr),0,(struct sockaddr *)&SendAddr,sizeof(SendAddr))<0){
            printf("sendto failed\n");
            exit(EXIT_FAILURE);
        }
        struct iphdr ipPacket;
        int addr_len = sizeof(RecvAddr);
        if(recvfrom(sockfd, (void *)&ipPacket, sizeof(struct iphdr),0,(struct sockaddr *)&RecvAddr,&addr_len)<0){
            printf("recvfrom failed\n");
            exit(EXIT_FAILURE);
        }
        struct icmphdr *icmpRecv = (struct icmphdr *)( (char *)&ipPacket + (ipPacket.ihl<<2));
        if(icmpRecv->type==ICMP_ECHOREPLY){
            printf("yayyy..got a reply!!\nRecvIP: %s",inet_ntoa(RecvAddr.sin_addr));
        }
        else if(icmpRecv->type==ICMP_TIME_EXCEEDED){
            printf("onnno..timeout!!\nRecvIP: %s",inet_ntoa(RecvAddr.sin_addr));
        }  
        else{
            printf("some random error!\nRecvIP: %s",inet_ntoa(RecvAddr.sin_addr));
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}