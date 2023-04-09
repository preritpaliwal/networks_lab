// STUDY ICMP TO WRITE THIS

// two threads - one for sending, one for receiving
// save time of sending and use that to calculate RTT after receiving
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

#define PACKET_SIZE 64
#define MAX_HOPS 30

void clear(char *buffer, int SIZE)
{
    for (int i = 0; i < SIZE; i++)
        buffer[i] = '\0';
}

void print(char* buffer, int SIZE)
{
    printf("BUFFER : ");
    for(int i=0;i<SIZE;i++)
        printf("|%d|",buffer[i]);
    printf("\n");
}

void print_hostent(struct hostent *hostent)
{
    printf("\n\nhostent -> h_name = %s\n", hostent->h_name);
    for (int i = 0; hostent->h_aliases[i] != NULL; i++)
        printf("hostent -> h_aliases[%d] = %s\n", i, hostent->h_aliases[i]);
    printf("hostent -> h_addrtype = %d\n", hostent->h_addrtype);
    printf("hostent -> h_length = %d\n", hostent->h_length);

    for (int i = 0; hostent->h_addr_list[i] != NULL; i++)
    {

        for (int j = 0; j < hostent->h_length; j++)
            printf("%d ", hostent->h_addr_list[i][j]);
        printf("hostent -> h_addr_list[%d] = %s.\n", i, inet_ntoa(*(struct in_addr *)(hostent->h_addr_list[i])));
        printf("hostent -> h_addr_list[%d] = %s.\n", i, inet_ntoa(*(struct in_addr *)*(hostent->h_addr_list)));
    }
}

int isValidIP(char ip[])
{
    int len = strlen(ip);
    int num = 0;
    for (int i = 0; i < len; i++)
    {
        if (ip[i] == '.')
        {
            if (num > 255)
            {
                return 0;
            }
            num = 0;
        }
        else
        {
            if (ip[i] < '0' || ip[i] > '9')
            {
                return 0;
            }
            num = num * 10 + ip[i] - '0';
        }
    }
    if (num > 255)
    {
        return 0;
    }
    return 1;
}

unsigned short czech_sum(unsigned short *arr, int len)
{
    unsigned short sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += arr[i];
    }
    // printf("Checksum : %d\n",~sum);
    return ~sum;
}



int main(int argc, char *argv[])
{

    // Validate argc
    if (argc != 4)
    {
        printf("Usage: %s <IP address/Domain Name Server> <Number of Probes to be sent per link> <Time difference between any two probes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check for sudo privilige (required for raw socket)
    if (getuid() != 0)
    {
        fprintf(stderr, "Please re-run %s with sudo!\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Validate argv (and convert IP from DNS if required)
    char IP[20];
    if (isValidIP(argv[1]))
        strcpy(IP, argv[1]);
    else
    {
        struct hostent *host = gethostbyname(argv[1]);
        if (host == NULL)
        {
            printf("unknown host %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        // print_hostent(host);
        strcpy(IP, inet_ntoa(*(struct in_addr *)(host->h_addr_list[0])));
    }
    printf("Host IP : %s\n", IP);
    // Integer value of IP (in_addr_t is unsigned int 32)
    in_addr_t IP_val = inet_addr(IP);
    // Number of probes per link
    int n = atoi(argv[2]);
    // Time difference between any two probes
    int T = atoi(argv[3]);

    // Create a raw socket to send ICMP packets
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        perror("Raw Socket Creation failed!");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in destination, intermediate;

    // Set timeout for recieve on socket
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("setsockopt Receive Timeout failed!");
        exit(EXIT_FAILURE);
    }

    char send_buffer[PACKET_SIZE], recv_buffer[PACKET_SIZE];
    clear(send_buffer, PACKET_SIZE);
    clear(recv_buffer, PACKET_SIZE);

    // Initialise ICMP struct for ICMP_ECHO packets
    struct icmphdr *ICMP = (struct icmphdr *)send_buffer;
    ICMP->type = ICMP_ECHO;
    ICMP->code = 0;
    ICMP->checksum = 0;
    ICMP->un.echo.id = 12;
    ICMP->un.echo.sequence = 0;
    ICMP->checksum = czech_sum((unsigned short *)send_buffer, PACKET_SIZE);

    destination.sin_family = AF_INET;
    destination.sin_addr.s_addr = IP_val;

    int ttl = 1;
    char route[MAX_HOPS][20];
    clear((char*)route, MAX_HOPS * 20);

    while (1)
    {
        printf("TTL : %d\n",ttl);
        if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &(ttl), sizeof(ttl)) < 0)
        {
            perror("setsockopt TTL failed!");
            exit(EXIT_FAILURE);
        }

        // printf("Send Buffer: ");
        // for (int i = 0; i < PACKET_SIZE; i++)
        //     printf("|%d|", send_buffer[i]);
        // printf("\n");

        // TODO- Get IP headers to see what socket is sending

        // Send ICMP_ECHO packet
        int nbytes = sendto(sockfd, send_buffer, PACKET_SIZE, 0, (struct sockaddr *)&destination, sizeof(destination));
        if (nbytes < 0)
        {
            perror("sendto failed!");
            exit(EXIT_FAILURE);
        }

        int intermediate_len = sizeof(intermediate);
        memset(&intermediate, 0, intermediate_len);
        clear(recv_buffer,PACKET_SIZE);

        nbytes = recvfrom(sockfd, recv_buffer, PACKET_SIZE, 0, (struct sockaddr *)&intermediate, &intermediate_len);
        if (nbytes < 0)
        {
            perror("recvfrom failed!");
            exit(EXIT_FAILURE);
        }

        struct iphdr* IP_packet = (struct iphdr *) recv_buffer;




        /*
        |IP header|ICMP header|ICMP data|
        */

        struct icmphdr *ICMP_hdr = (struct icmphdr *)(recv_buffer + (IP_packet->ihl * 2 * 2));
        if (ICMP_hdr->type == ICMP_ECHOREPLY)
        {
            printf("Last Node Reached! : %s\n", inet_ntoa(intermediate.sin_addr));
            strcpy(route[ttl-1],inet_ntoa(intermediate.sin_addr));
            break;
        }
        else if (ICMP_hdr->type == ICMP_TIME_EXCEEDED)
        {
            printf("No Time to Live : %s\n", inet_ntoa(intermediate.sin_addr));
            strcpy(route[ttl-1],inet_ntoa(intermediate.sin_addr));
        }
        else if (ICMP_hdr->type == ICMP_DEST_UNREACH)
        {
            printf("NO KNOW! : %s\n", inet_ntoa(intermediate.sin_addr));
        }
        else
        {
            printf("ICMP header type : %d\n",ICMP_hdr->type);
            printf("IP IHL : %d\n",IP_packet->ihl);
            break;
        }

        ttl++;
    }


    printf("Path followed by Traceroute: \n");
    for(int i=0;route[i][0] != '\0';i++){
        printf("%d) %s\n",i+1,route[i]);
    } 

    return 0;
}