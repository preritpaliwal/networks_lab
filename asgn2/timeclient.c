/*
Name: Prerit Paliwal
Roll Number: 20CS10046
Assignment 2
Question 1
*/


#include <stdio.h>
#include <sys/socket.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLEN 1024
#define PORT 20000

int main()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("could not make socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_aton("127.0.0.1", &serv_addr.sin_addr);

    struct pollfd fds[1];
    int nfds = 1;

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    int timeout = (3 * 1000);
    int done = 0;

    for (int i = 0; i < 5; i++)
    {

        char *msg = "Hi, please send me your date and time\n";
        // printf("sending msg\n");
        sendto(sockfd, (const char *)msg, strlen(msg), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
        // printf("msg sent\n");

        // printf("Waiting on poll()...\n");
        int rc = poll(fds, nfds, timeout);
        // printf("rc = %d", rc);

        if (rc < 0)
        {
            perror("  poll() failed");
            break;
        }
        else if (rc == 0)
        {
            // printf("  poll() timed out.  \n");
            continue;
        }
        else if (fds[0].revents == POLLIN)
        {
            char buffer[MAXLEN];
            for (int i = 0; i < MAXLEN; i++)
            {
                buffer[i] = '\0';
            }
            // printf("waiting to receive ack from server..!!\n");
            socklen_t len = sizeof(serv_addr);
            int n = recvfrom(sockfd, (char *)buffer, MAXLEN, 0, (struct sockaddr *)&serv_addr, &len);
            printf("server says: %s", buffer);
            done = 1;
            break;
        }
    }

    if(!done){
        printf("Timeout exceeded..!!\n");
    }
    close(sockfd);
    return 0;
}