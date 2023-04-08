#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <sys/types.h>

int num_sizes = 10;
int size_f = 64;
int size_l = 4096;
double prev_slope = 0.0;

double bandwidth_all[100];
double latency_all[100];

unsigned short cksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }

    if (nleft == 1)
    {
      *(unsigned char *)(&answer) = *(unsigned char *)w;
      sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    
    return (answer);
}


// calculate latency and bandwidth by sending n packets each at T seconds interval for different packet sizes
void calculate_latency_bandwidth(int hop,char *inter_ip, int n, int T, int sock_fd, int sizes[])
{
    struct sockaddr_in inter_addr;
    double rtts[num_sizes];
    for(int i =0;i<num_sizes;i++)
    {
        rtts[i]=1000;
    }
    for (int k = 0; k < n; k++)
    {
        for (int j = 0; j < num_sizes; j++)
        {
            int packet_size = sizes[j];
            char send_buff[packet_size];
            memset(send_buff, 0, packet_size);
            struct icmp *icmp = (struct icmp *) send_buff;
            icmp->icmp_type = ICMP_ECHO;
            icmp->icmp_code = 0;
            icmp->icmp_cksum = 0;
            icmp->icmp_id = (getpid() & 0xFFFF);
            icmp->icmp_seq = 0;
            icmp->icmp_cksum = cksum((unsigned short *) send_buff, packet_size);
            struct sockaddr_in destination_addr;
            destination_addr.sin_family = AF_INET;
            destination_addr.sin_addr.s_addr = inet_addr(inter_ip);
            double rtt = 0;

            struct timeval start, end;
            
            gettimeofday(&start, NULL);
            if (sendto(sock_fd, send_buff, packet_size, 0, (struct sockaddr *) &destination_addr, sizeof(destination_addr)) < 0)
            {
                perror("sendto() error");
                exit(-1);
            }
            char recv_buff[packet_size];
            memset(recv_buff, 0, packet_size);
            memset(&inter_addr, 0, sizeof(inter_addr));
            int addr_len = sizeof(inter_addr);
            if (recvfrom(sock_fd, recv_buff, packet_size, 0, (struct sockaddr *) &inter_addr, &addr_len) < 0)
            {
                perror("recvfrom() error");
                exit(-1);
            }
            else
            {
                gettimeofday(&end, NULL);
            }

            rtts[j] = rtts[j]<((end.tv_usec - start.tv_usec) / 1000.0 + (end.tv_sec - start.tv_sec) * 1000.0)? rtts[j]:((end.tv_usec - start.tv_usec) / 1000.0 + (end.tv_sec - start.tv_sec) * 1000.0);
            usleep(T*1000);
        }
    }
    for(int i=0;i<num_sizes;i++)
    {
        printf("Min RTT for packet size %d bytes is %lfms\n", sizes[i], rtts[i]);
    }
    double slope=0.0;
    for(int i=0;i<num_sizes-1;i++)
    {
        slope += (rtts[i+1]-rtts[i])/(sizes[i+1]-sizes[i]);
    }
    slope = slope/(num_sizes-1);
    double link_bandwidth_inv = slope - prev_slope;
    prev_slope = link_bandwidth_inv;

    double bandwidth = (2000.0*8)/(link_bandwidth_inv*1024*1024);
    printf("Bandwidth is %f Mbps\n", bandwidth);
    bandwidth_all[hop] = bandwidth;

    double latency = 0.0;
    for(int i=0;i<num_sizes;i++)
    {
        latency += rtts[i] - 2*(sizes[i]*8000.0)/(bandwidth*1024*1024);
        for(int i=1;i<hop;i++)
        {
            latency -= 2*(sizes[i]*8000.0)/(bandwidth_all[i]*1024*1024) + latency_all[i];
        }
    }
    latency = latency/num_sizes;
    printf("Avg Latency is %f ms\n", latency);
    latency_all[hop] = latency;
}

int main()
{
    printf("Enter the website address: ");
    char website[100];
    scanf("%s", website);

    int n;
    printf("Enter the number of times a probe will be sent per link: ");
    scanf("%d", &n);

    int T;
    printf("Enter the time interval between probes in milliseconds: ");
    scanf("%d", &T);

    char ip[100];

    if (website[0] >= 'a' && website[0] <= 'z' || website[0] >= 'A' && website[0] <= 'Z')
    {
        struct hostent *host = gethostbyname(website);
        if (host == NULL)
        {
            printf("Invalid website address\n");
            return 0;
        }
        strcpy(ip, inet_ntoa (*(struct in_addr*)host->h_addr_list[0]));
    }
    else
    {
        strcpy(ip, website);
    }

    printf("IP address: %s\n", ip);

    // dont know what is it used for..!
    int sizes[num_sizes];
    int curr=0;
    for(int i=size_f;curr<num_sizes;i+=(size_l-size_f)/(num_sizes-1))
    {
        sizes[curr]=i;
        curr++;
    }

    int sock_fd;
    struct sockaddr_in website_addr, inter_addr;

    if((sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        perror("socket() error");
        exit(-1);
    }

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("Error");
    }

    int packet_size = 64;
    char send_buff[packet_size];
    memset(send_buff, 0, packet_size);
    struct icmp *icmp = (struct icmp *) send_buff;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_id = (12);
    icmp->icmp_seq = 0;
    icmp->icmp_cksum = cksum((unsigned short *) send_buff, packet_size);
    printf("checksum = %d",icmp->icmp_cksum);
    website_addr.sin_family = AF_INET;
    website_addr.sin_addr.s_addr = inet_addr(ip);

    char path[100][100];
    double avg_latency[100];

    int ttl = 1;
    prev_slope = 0.0;
    int i;
    while(1)
    {
        setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

        for(int i = 0;i<packet_size;i++){
            printf("|%d|",send_buff[i]);
        }
        printf("\n");
        if (sendto(sock_fd, send_buff, packet_size, 0, (struct sockaddr *) &website_addr, sizeof(website_addr)) < 0)
        {
            perror("sendto() error");
            exit(-1);
        }
        else
        {
            // printf("Packet %d sent.\n", ttl);
        }

        //receive the ICMP packet
        char recv_buff[packet_size];
        memset(recv_buff, 0, packet_size);
        memset(&inter_addr, 0, sizeof(inter_addr));
        int addr_len = sizeof(inter_addr);
        if (recvfrom(sock_fd, recv_buff, packet_size, 0, (struct sockaddr *) &inter_addr, &addr_len) < 0)
        {
            perror("recvfrom() error");
            exit(-1);
        }
        else
        {
            // printf("Reply received.\n");
        }
        
        struct ip *ip_hdr = (struct ip *) recv_buff;
        struct icmp *icmp_hdr = (struct icmp *) (recv_buff + (ip_hdr->ip_hl << 2));

        if (icmp_hdr->icmp_type == ICMP_ECHOREPLY)
        {
            // printf("Destination reached.\n");
            strcpy(path[ttl - 1], ip);

            printf("Hop %d: %s\n", ttl, ip);

            calculate_latency_bandwidth(ttl,ip, n, T, sock_fd,sizes);

            break;
        }
        else if (icmp_hdr->icmp_type == ICMP_TIME_EXCEEDED)
        {
            // printf("Time exceeded.\n");
            char *inter_ip = inet_ntoa(inter_addr.sin_addr);
            strcpy(path[ttl - 1], inter_ip);

            printf("Hop %d: %s\n", ttl, inter_ip);

            calculate_latency_bandwidth(ttl,inter_ip, n, T, sock_fd,sizes);

        }
        else if (icmp_hdr->icmp_type == ICMP_DEST_UNREACH)
        {
            printf("Destination unreachable.\n");
            return 0;
        }
        else
        {
            printf("Other error.\n");
            return 0;
        }
        ttl++;
    }

    printf("The path to the website is:\n");
    for (i = 0; i < ttl ; i++)
    {
        // printf("%s %lfms \n", path[i], avg_latency[i]);
        printf("%s\n", path[i]);
    }

    close(sock_fd);
    return 0;
}
