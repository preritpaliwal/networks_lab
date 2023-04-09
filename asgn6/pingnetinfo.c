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
#define TIMEOUT 2
#define NEXT_LINK_PACKETS 5
#define NEXT_LINK_TIMEOUT 0.001

typedef struct
{
    char ip[20];
    int count;
} ip_map;

int ttl = 1;
char route[MAX_HOPS][20];
double latency[MAX_HOPS];
double bandwidth[MAX_HOPS];


void clear(char *buffer, int SIZE)
{
    for (int i = 0; i < SIZE; i++)
        buffer[i] = '\0';
}

void print_buffer(char *buffer, int SIZE)
{
    printf("BUFFER : ");
    for (int i = 0; i < SIZE; i++)
        printf("|%d|", buffer[i]);
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

/*
     --------------------------------
    |IP header|ICMP header|ICMP data|
    --------------------------------
*/
void print_ICMP_packet(char *buffer, int SIZE, int sent)
{
    struct iphdr *IP = (struct iphdr *)buffer;
    struct icmphdr *ICMP = (struct icmphdr *)(buffer + (IP->ihl << 2));

    if (sent)
        printf("\n==========IP HEADER[sent]===============\n");
    else
        printf("\n==========IP HEADER[received]===============\n");

    char protocol[10];
    if (IP->protocol == IPPROTO_ICMP)
        strcpy(protocol, "ICMP");
    else if (IP->protocol == IPPROTO_TCP)
        strcpy(protocol, "TCP");
    else if (IP->protocol == IPPROTO_UDP)
        strcpy(protocol, "UDP");
    else
        strcpy(protocol, "unknown");

    printf("Version: %d, IHL: %d, Type of Service: %d, Total Length: %d\n", IP->version, IP->ihl, IP->tos, IP->tot_len);
    printf("Identification: %d,Flags: %d, Fragment Offset: %d\n", IP->id, (IP->frag_off & 0xe000), (IP->frag_off & 0x1fff));
    printf("Time to Live: %d, Protocol: %s(%d), Header Checksum: %d\n", IP->ttl, protocol, IP->protocol, IP->check);
    struct in_addr s, d;
    s.s_addr = IP->saddr;
    d.s_addr = IP->daddr;
    printf("Source Address: %s\n", inet_ntoa(s));
    printf("Destination Address: %s\n", inet_ntoa(d));

    if (sent)
        printf("==========ICMP HEADER[sent]==========\n");
    else
        printf("==========ICMP HEADER[received]==========\n");

    if (ICMP->type == ICMP_ECHOREPLY)
    {
        printf("Type: Echo Reply(%d), Code: %d, Checksum: %d\n", ICMP->type, ICMP->code, ICMP->checksum);
        printf("Identifier: %d, Sequence No: %d\n", ICMP->un.echo.id, ICMP->un.echo.sequence);
    }
    else if (ICMP->type == ICMP_TIME_EXCEEDED)
    {
        printf("Type: Time Exceeded(%d), Code: %d, Checksum: %d\n", ICMP->type, ICMP->code, ICMP->checksum);
        // printf("Unused: [echo]: %d, [frag]: %d\n",ICMP->un.echo.id,ICMP->un.echo.sequence);
    }
    printf("\n\n");
}

void getLatencyBand(int sock_fd, int hop,int N,int T){
    struct sockaddr_in inter_addr;
    double rtts[N];
    for(int i =0;i<N;i++)
    {
        rtts[i]=1000;
    }
    int prevData = 1;
    int incData = 1000;
    for (int k = 0; k < N; k++)
    {
        // for (int j = 0; j < num_sizes; j++)
        // {
        int packet_size = 0;
        if(k==0){
             packet_size = sizeof(struct icmphdr);
        }
        else{
            packet_size = sizeof(struct icmphdr) + prevData;
            prevData+=incData;
        }
        char send_buffer[packet_size];
        memset(send_buffer, 0, packet_size);
        // Initialise ICMP struct for ICMP_ECHO packets
        struct icmphdr *ICMP = (struct icmphdr *)send_buffer;
        ICMP->type = ICMP_ECHO;
        ICMP->code = 0;
        ICMP->checksum = 0;
        ICMP->un.echo.id = k+100;
        ICMP->un.echo.sequence = 0;
        send_buffer[packet_size-2] = k;

        ICMP->checksum = czech_sum((unsigned short *)send_buffer, packet_size/sizeof(unsigned short));
        struct sockaddr_in destination;
        destination.sin_family = AF_INET;
        destination.sin_addr.s_addr = inet_addr(route[hop-1]);


        struct timeval start, end;

        gettimeofday(&start, NULL);
        if (sendto(sock_fd, send_buffer, packet_size, 0, (struct sockaddr *) &destination, sizeof(destination)) < 0)
        {
            perror("sendto() error");
            exit(-1);
        }
        char recv_buff[packet_size+sizeof(struct iphdr)];
        memset(recv_buff, 0, packet_size);
        memset(&inter_addr, 0, sizeof(inter_addr));
        int addr_len = sizeof(inter_addr);
        if (recvfrom(sock_fd, recv_buff, packet_size, 0, (struct sockaddr *) &inter_addr, &addr_len) < 0)
        {
            perror("recvfrom() error");
            // printf("could not receive the packet!!\n");
            // continue;
            exit(-1);
        }
        else
        {
            gettimeofday(&end, NULL);
        }

        // check if it is the same packet or not?
        // print(send_buffer,packet_size);
        // print(recv_buff,packet_size);

        // if(send_buffer[packet_size-2]==recv_buff[packet_size-2])

        double rtt = ((end.tv_sec - start.tv_sec) * 1000.0 + ((end.tv_usec - start.tv_usec) / 1000.0));
        rtts[k] = rtt;
        usleep(T*1000);
        // }
        printf("rtts[%d] = %lf ms\n",k,rtt);
    }
    double bndw = 0;
    for(int i = 0;i<N-1;i++){
        bndw += (incData*1.0)/(rtts[i+1]-rtts[i]);
    }
    bndw/=(N-1);
    if(hop==1){
        latency[hop-1] = rtts[0];
    }
    else{
        latency[hop-1] = rtts[0] - latency[hop-2];
    }
    bandwidth[hop-1] = bndw*1000;
    printf("Latency of the link : %lf ms\n",latency[hop-1]);
    printf("Bandwidth of the link : %lf bps\n",bandwidth[hop-1]);


    return;
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
    tv.tv_sec = TIMEOUT;
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

    clear((char*)route, MAX_HOPS * 20);
    clear((char*)latency,MAX_HOPS);
    clear((char*)bandwidth,MAX_HOPS);

    int loop=1;

    while (loop)
    {
        printf("TTL : %d\n", ttl);
        if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &(ttl), sizeof(ttl)) < 0)
        {
            perror("setsockopt TTL failed!");
            exit(EXIT_FAILURE);
        }

        // TODO- change id and sequnce values
        ip_map mp[NEXT_LINK_PACKETS];
        memset(&mp,0,sizeof(mp));
        for(int j=0;j<NEXT_LINK_PACKETS;j++) mp[j].count=0;
        int idx = 0;

        for (int i = 0; i < NEXT_LINK_PACKETS; i++)
        {

            // Send ICMP_ECHO packet
            // print_ICMP_packet(send_buffer, PACKET_SIZE, 1);
            int nbytes = sendto(sockfd, send_buffer, PACKET_SIZE, 0, (struct sockaddr *)&destination, sizeof(destination));
            if (nbytes < 0)
            {
                perror("sendto failed!");
                exit(EXIT_FAILURE);
            }

            int intermediate_len = sizeof(intermediate);
            memset(&intermediate, 0, intermediate_len);
            clear(recv_buffer, PACKET_SIZE);

            nbytes = recvfrom(sockfd, recv_buffer, PACKET_SIZE, 0, (struct sockaddr *)&intermediate, &intermediate_len);
            if (nbytes < 0)
            {
                perror("recvfrom failed!");
                exit(EXIT_FAILURE);
            }
            // print_ICMP_packet(recv_buffer, PACKET_SIZE, 0);
            struct iphdr *IP_packet = (struct iphdr *)recv_buffer;
            struct icmphdr *ICMP_hdr = (struct icmphdr *)(recv_buffer + (IP_packet->ihl * 2 * 2));
            if (ICMP_hdr->type == ICMP_ECHOREPLY)
            {
                printf("Last Node Reached! : %s\n", inet_ntoa(intermediate.sin_addr));
                strcpy(route[ttl - 1], inet_ntoa(intermediate.sin_addr));
                getLatencyBand(sockfd,ttl,n,T);
                
                loop = 0;
                break;
            }
            else if (ICMP_hdr->type == ICMP_TIME_EXCEEDED)
            {
                // printf("No Time to Live : %s\n", inet_ntoa(intermediate.sin_addr));
                int j=0;
                for(j=0;j<idx;j++){
                    if(!strcmp(mp[j].ip,inet_ntoa(intermediate.sin_addr))){
                        mp[j].count++;
                        break;
                    }
                }
                if(j == idx){
                    strcpy(mp[j].ip,inet_ntoa(intermediate.sin_addr));
                    mp[j].count = 1;
                    idx++;
                }
            }
            else if (ICMP_hdr->type == ICMP_DEST_UNREACH)
            {
                printf("NO KNOW! : %s\n", inet_ntoa(intermediate.sin_addr));
            }
            else
            {
                printf("ICMP header type : %d\n", ICMP_hdr->type);
                printf("IP IHL : %d\n", IP_packet->ihl);
                break;
            }

            sleep(NEXT_LINK_TIMEOUT); 
        }
        
        if(!loop) continue;

        int max_selected_link_idx = -1, max_count = -1;
        for(int i=0;i<idx;i++){
            if(mp[i].count > max_count){
                max_count = mp[i].count;
                max_selected_link_idx = i;
            }
        }

        printf("IP: %s is the most frequently selected link with count = %d.\n",mp[max_selected_link_idx].ip,max_count);
        strcpy(route[ttl-1],mp[max_selected_link_idx].ip);
        getLatencyBand(sockfd,ttl,n,T);

        ttl++;
    }

    printf("Path followed by Traceroute: \n");
    for (int i = 0; route[i][0] != '\0'; i++)
    {
        printf("%d) %s\n", i + 1, route[i]);
    }

    close(sockfd);
    return 0;
}