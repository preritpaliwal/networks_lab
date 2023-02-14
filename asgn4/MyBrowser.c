#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#define BUFFER_SIZE 1024
#define MSG_SIZE 32

void print_protoent(struct protoent *protoent)
{
    printf("\n\nprotoent -> p_name = %s\n", protoent->p_name);
    printf("protoent -> p_proto = %d\n", protoent->p_proto);
    for (int i = 0; protoent->p_aliases[i] != NULL; i++)
        printf("protoent -> p_aliases[%d] = %s\n", i, protoent->p_aliases[i]);
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

void clear(char *buffer, int size)
{
    for (int i = 0; i < size; i++)
        buffer[i] = '\0';
}

// We assume url of the form http://<hostname><path>:<port>
int process_url(char *url, char *hostname, char *path, int *port)
{

    *port = 80;

    // Validate the URL
    if (strncmp(url, "http://", 7) != 0)
    {
        fprintf(stderr, "Invalid URL! URL must start with \"http://\"\n");
        return -1;
    }

    // Extract the hostname and path from the URL

    int i = 7;
    while (url[i] != '/' && url[i] != '\0')
    {
        hostname[i - 7] = url[i];
        i++;
    }
    hostname[i - 7] = '\0';

    int j = 0;
    while (url[i] != '\0' && url[i] != ':')
        path[j++] = url[i++];
    path[j] = '\0';

    if (url[i] == ':')
    {
        i++;
        *port = 0;
        while (url[i] != '\0')
        {
            *port = *port * 10 + (url[i] - '0');
            i++;
        }
    }

    return 0;
}

void process_header(char *header)
{
    char *line = strtok(header, "\n");
    while (line != NULL)
    {
        // printf("LINE => %s\n", line);

        line = strtok(NULL, "\n");
    }
}

int SetupRequest(char *hostname)
{
    int sockfd;
    // Prepare a HTTP socket
    struct protoent *protoent;
    protoent = getprotobyname("tcp");

    if (!protoent)
    {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }

    // print_protoent(protoent);

    // Create a socket file descriptor for the HTTP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Socket protocol can be manually specified using the protoent struct
    // or set as 0 in which case it will be automatically set to the default protocol for the specified socket type
    // int sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);

    if (sockfd < 0)
    {
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

    // Get the hostent struct (host entry) for the hostname
    struct hostent *hostent;
    hostent = gethostbyname(hostname);
    if (!hostent)
    {
        fprintf(stderr, "gethostbyname Failed for hostname : %s\n", hostname);
        exit(EXIT_FAILURE);
    }
    // print_hostent(hostent);

    char *addr = inet_ntoa(*(struct in_addr *)*(hostent->h_addr_list));
    // printf("\n\naddr : %s\n", addr);
    in_addr_t in_addr = inet_addr(addr);
    // printf("in_addr = %u\n", in_addr);

    // 255.255.255.255 is a special url that is not sent to the router
    if (in_addr == (in_addr_t)-1)
    {
        fprintf(stderr, "Internet Address Error:  inet_addr(\"%s\")\n", addr);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    int server_port = 80;

    serv_addr.sin_addr.s_addr = in_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void GET(char *url)
{

    int port_no;
    char hostname[2048];
    char path[2048];

    int ret = process_url(url, hostname, path, &port_no);

    if (ret < 0)
    {
        printf("Error in processing URL. Please Try Again.\n");
        return;
    }

    printf("\nhostname : %s, path : %s, port_no : %d\n\n", hostname, path, port_no);

    int sockfd = SetupRequest(hostname);

    // Send the HTTP request
    char *request_template = "GET %s HTTP/1.1\r\nHost: %s\r\nPort: %d\r\nConnection: close\r\n\r\n";
    // A lot of browsers (Internet Explorer, Safari, Opera) keep HTTP/HTTPS request length as 2kb or 4kb
    char request[4096];
    int req_len = snprintf(request, 4096, request_template, path, hostname, port_no);

    char buffer[BUFFER_SIZE];

    // Send the request

    int nbytes_total = 0, nbytes_last = 0;
    while (nbytes_total < req_len)
    {
        nbytes_last = write(sockfd, request + nbytes_total, req_len - nbytes_total);
        if (nbytes_last == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        nbytes_total += nbytes_last;
    }


    FILE *fp = fopen("response.html", "wb");

    // Receive the response
    while ((nbytes_total = read(sockfd, buffer, BUFFER_SIZE)) > 0)
    {
        // fprintf(stderr, "debug: after a read\n");
        // printf("%s", buffer);
        fwrite(buffer, 1, nbytes_total, fp);
        write(STDOUT_FILENO, buffer, nbytes_total);
    }

    if (nbytes_total == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // printf("\n\n HERE \n\n");

    printf("\n\nBye\n");

    fclose(fp);
    close(sockfd);
}

void PUT(char *url, char *file_path)
{
}

int main()
{

    while (1)
    {
        printf("MyBroswer> ");

        char request_type[128];
        scanf("%s", request_type);

        if (strcmp(request_type, "QUIT") == 0)
        {
            printf("Exiting...\n");
            break;
        }

        else if (strcmp(request_type, "GET") == 0)
        {
            // As Microsoft Edge and Internet Explored have a maximum URL length of 2048 characters,
            // We feel it is safe to assume that the URL length will not exceed 2048 characters for a simple browser like this
            char url[2048];
            scanf("%s", url); // We assume the URL will be entered without any spaces (spaces replaced by %20)

            printf("Attempting a GET request for URL : %s\n", url);

            GET(url);
        }

        else if (strcmp(request_type, "PUT") == 0)
        {
            char url[2048];
            scanf("%s", url); // We assume the URL will be entered without any spaces (spaces replaced by %20)

            char file_path[2048];
            // scanf("%s", file_path);
            fgets(file_path, 2048, stdin);

            printf("Attempting a POST request for URL : %s, file_path : %s\n", url, file_path);

            PUT(url, file_path);
        }

        else
        {
            printf("Invalid Request Type. Please Try Again.\n");
        }
    }

    return 0;
}