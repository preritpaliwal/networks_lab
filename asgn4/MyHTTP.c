#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>

#include <netdb.h>

#define BUFFER_SIZE 1024
#define MSG_SIZE 32
#define PORT_NO 9999

#define GET_METHOD 11
#define PUT_METHOD 12
#define UNKNOWN_METHOD 13

typedef struct Header
{

    char **field;
    char **value;
    int size;

} Header;

/*
GET <path> HTTP/1.1
Host: <hostname>
Connection: close
Accept: <accept>
...
*/
typedef struct Request
{
    int method;

    char *ip;   // ip address of the client
    char *path; // path of the file in the server

    char *HTTP_version;
    Header *headers;

    char *body;

} Request;

typedef struct Response
{

    char *HTTP_version;

    int status_code;
    char *status_message;

    Header *headers;
    char *body;

} Response;

void clear(char *buffer, int size)
{
    for (int i = 0; i < size; i++)
        buffer[i] = '\0';
}

void free_header(Header *h)
{

    for (int i = 0; i < h->size; i++)
    {
        free(h->field[i]);
        free(h->value[i]);
    }

    free(h->field);
    free(h->value);

    free(h);
}

void free_request(Request *req)
{

    free(req->ip);
    free(req->path);
    free(req->HTTP_version);

    free_header(req->headers);

    free(req->body);

    free(req);
}

void free_response(Response *rsp)
{

    free(rsp->HTTP_version);
    free(rsp->status_message);

    free_header(rsp->headers);

    free(rsp->body);

    free(rsp);
}

void print_header(Header *h)
{
    printf("\nPrinting Headers [size = %d]\n", h->size);

    for (int i = 0; i < h->size; i++)
        printf("%s: %s\n", h->field[i], h->value[i]);
}

void print_request(Request *req)
{
    printf("\n\nPrinting Request\n");

    printf("Method: %d\n", req->method);
    printf("IP: %s\n", req->ip);
    printf("Path: %s\n", req->path);
    printf("HTTP Version: %s\n", req->HTTP_version);

    print_header(req->headers);

    printf("\nBody: \n%s", req->body);
}

void print_response(Response *rsp)
{
    printf("\n\nPrinting Response\n");

    printf("HTTP Version: %s\n", rsp->HTTP_version);
    printf("Status Code: %d\n", rsp->status_code);
    printf("Status Message: %s\n", rsp->status_message);

    print_header(rsp->headers);

    printf("\nBody: \n%s", rsp->body);
}

char *get_status_message(int status_code)
{
    char * status_message;
    switch (status_code)
    {

    case 200:
        status_message = strdup("OK");
        break;

    case 400:
        status_message = strdup("Bad Request");
        break;

    case 403:
        status_message = strdup("Forbidden");
        break;

    case 404:
        status_message = strdup("Not Found");
        break;

    default:
        status_message = strdup("Unknown Issue");
    }

    return status_message;

}

// GET / HTTP/1.1
// Host: www.abc.com
Request *parse(char *rq)
{
    struct Request *req = malloc(sizeof(struct Request));
    memset(req, 0, sizeof(struct Request)); // initialize the request to 0

    // get the method , method_size = 3 for GET and PUT
    int method_size = strcspn(rq, " ");

    // Method Type
    if (!strncmp(rq, "GET", method_size))
        req->method = GET_METHOD;
    else if (!strncmp(rq, "PUT", method_size))
        req->method = PUT_METHOD;
    else
        req->method = UNKNOWN_METHOD;

    // Skip the space and method type
    rq = rq + method_size;
    while (rq[0] == ' ')
        rq++;

    // Get Path
    int path_size = strcspn(rq, " ");
    req->path = malloc(path_size + 1);
    strncpy(req->path, rq, path_size);
    req->path[path_size] = '\0';

    // skip the space and path
    rq = rq + path_size;
    while (rq[0] == ' ')
        rq++;

    // Get HTTP Version
    int HTTP_size = strcspn(rq, "\r");
    req->HTTP_version = malloc(HTTP_size + 1);
    strncpy(req->HTTP_version, rq, HTTP_size);
    req->HTTP_version[HTTP_size] = '\0';

    rq = rq + HTTP_size;

    // Move past all the whitespace and newlines
    while (rq[0] == ' ' || rq[0] == '\r' || rq[0] == '\n')
        rq++;

    // Get Headers
    req->headers = malloc(sizeof(struct Header));
    req->headers->size = 0;
    memset(req->headers, 0, sizeof(struct Header)); // initialize the header to 0

    int i = 5;

    while (rq != NULL && (rq[0] != '\r' && rq[1] != '\n'))
    {
        printf("Request = %d\n", (int)rq[0]);

        int field_size = strcspn(rq, ":");
        char *field = malloc(field_size + 1);
        strncpy(field, rq, field_size);
        field[field_size] = '\0';

        printf("Field = %s [%d]\n", field, field_size);

        // Move past the colon
        rq = rq + field_size + 1;
        while (rq[0] == ' ')
            rq++;

        int value_size = strcspn(rq, "\r");
        char *value = malloc(value_size + 1);
        strncpy(value, rq, value_size);
        value[value_size] = '\0';

        // Move past the newline and whitespace
        rq = rq + value_size;
        while (rq[0] == ' ')
            rq++;

        rq += 2;
        // printf("Remaining Request: \n%s\n", rq);
        // printf("==============================================================================================\n");

        // Allocate space for the name and value of new header
        req->headers->field = realloc(req->headers->field, (req->headers->size + 1) * sizeof(char *));
        req->headers->field[req->headers->size] = field;

        req->headers->value = realloc(req->headers->value, (req->headers->size + 1) * sizeof(char *));
        req->headers->value[req->headers->size] = value;

        req->headers->size++;
    }

    // The last while loop within the above while loop while also parse
    // the "\r\n" used to separate each header:value pair
    // and body has another "\r\n" preceeding it So, we can just start to parse the body now
    while (rq[0] == '\r' || rq[0] == '\n')
        rq++;

    int body_size = strlen(rq);

    req->body = malloc(body_size + 1);
    strncpy(req->body, rq, body_size);
    req->body[body_size] = '\0';

    return req;
}

char **tokenize(char *cmd)
{
    int index = 0;
    char temp[BUFFER_SIZE];

    char **cmdarr;
    cmdarr = (char **)malloc(sizeof(char *));
    cmdarr[index] = (char *)malloc(128 * sizeof(char));

    int cnt = 0;
    int space = 0;

    int i = 0;
    // remove whitespaces at start
    while (cmd[i] == ' ')
        i++;

    for (i; cmd[i] != '\0'; i++)
    {

        cnt = 0;
        if (space == 1 && cmd[i] == ' ')
            continue;
        else if (cmd[i] == ' ')
        {
            temp[cnt++] = cmd[i];
            space = 1;
            continue;
        }

        // index for populating the array
        while (!(cmd[i] == ' ' && cmd[i - 1] != '\\'))
        {
            if (cmd[i] == '\0')
                break;
            if (cmd[i] == '\\')
            {
                i++;
                // skipping the back slash
                temp[cnt++] = cmd[i++];
                continue;
            }
            temp[cnt++] = cmd[i++];
            // added random
        }

        temp[cnt++] = '\0';
        // printf("Temp is %s\n", temp);

        // copy temp into the cmdarr
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr

        cmdarr = (char **)realloc(cmdarr, (index + 1) * sizeof(char *));
        cmdarr[index] = (char *)malloc(BUFFER_SIZE * sizeof(char));

        if (cmd[i] == '\0')
            break;
    }

    cmdarr[index] = NULL;
    return cmdarr;
}

// returns 1 if the last_modified is before the if_modified_since
int is_modified(char *last_modified, char *modified_since)
{

    printf("LAST MODIFIED: %s\n", last_modified);

    char **date = tokenize_command(last_modified);
    char **check = tokenize_command(modified_since);

    for (int i = 0; date[i] != NULL; i++)
        printf("date[%d] = %s.\n", i, date[i]);

    int year = atoi(date[3]);
    int year_check = atoi(check[3]);

    if (year < year_check)
        return 1;
    if (year > year_check)
        return 0;

    char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    int month = -1, month_check = -1;

    for (int i = 0; i < 12; i++)
    {
        if (!strcmp(date[2], months[i]))
            month = i;
        if (!strcmp(check[2], months[i]))
            month_check = i;
    }
    if (month < month_check)
        return 1;
    if (month > month_check)
        return 0;

    int day = atoi(date[1]);
    int day_check = atoi(check[1]);

    if (day < day_check)
        return 1;
    if (day > day_check)
        return 0;

    return 0;
}

char *update_date(int changeday, struct tm local_t)
{
    // time_t t = time(NULL);
    // struct tm tm = *localtime(&t);
    local_t.tm_mday += changeday;
    mktime(&local_t);
    char buf[50];
    strcpy(buf, asctime(&local_t));

    buf[strlen(buf) - 1] = '\0';
    // printf("%s\n", buf);

    char **temp = tokenize(buf);

    // Now HTTP formatting
    char *final = (char *)malloc(100 * sizeof(char));
    strcpy(final, temp[0]);
    strcat(final, ", ");
    strcat(final, temp[2]);
    strcat(final, " ");
    strcat(final, temp[1]);
    strcat(final, " ");
    strcat(final, temp[4]);
    strcat(final, " ");
    strcat(final, temp[3]);
    strcat(final, " ");
    strcat(final, "IST");

    // printf("Final date : %s\n", final);
    return final;
}

// fetch header to find the parameters of the communication
char *fetch_header(int sockfd)
{
    
}

void send_rsp_status_line(int sockfd, int status_code){

    char* status_message = get_status_message(status_code);

    char* status_line = (char*)malloc(100*sizeof(char));

    // TODO : check if 2 "\r\n" are needed or 1
    sprintf(status_line, "HTTP/1.1 %d %s\r\n\r\n", status_code, status_message);

    send(sockfd,status_line, strlen(status_line)+1,0);
}

void send_rsp_header(int sockfd, Response* rsp)
{
    char response[BUFFER_SIZE];
    clear(response,BUFFER_SIZE);

    sprintf(response,"%s %d %s\r\n",rsp->HTTP_version,rsp->status_code,rsp->status_message);
    char* temp = response + strlen(response);

    for(int i=0;i<rsp->headers->size;i++){
        sprintf(temp,"%s: %s\r\n",rsp->headers->field[i],rsp->headers->value[i]);
        temp += strlen(temp);        
    }

    strcat(temp,"\r\n");

    // printf("Response String:\n%s",response);
    send(sockfd, response, strlen(response)+1,0);
}

/*
Headers to be sent: [CAse insenSITIVE] (we'll do lower to make it easier)
    - Expires, Cache-Control, Content-Language [added here]
    - Content-length, Content-type, Last-Modified [added when file is read]
*/
void set_header(int status_code, Response *rsp)
{
    rsp->HTTP_version = strdup("HTTP/1.1");
    rsp->status_code = status_code;
    rsp->status_message = get_status_message(status_code);

    Header *h = malloc(sizeof(Header));
    rsp->headers = h;

    h->field = (char **)malloc(3 * sizeof(char *));
    h->value = (char **)malloc(3 * sizeof(char *));
    h->size = 3;

    h->field[0] = strdup("expires");
    time_t t = time(NULL);
    struct tm local_t = *localtime(&t);
    h->value[0] = strdup(update_date(3, local_t));

    h->field[1] = strdup("cache-control");
    h->value[1] = strdup("no-store always");

    h->field[2] = strdup("content-language");
    h->value[2] = strdup("en-us");
}

// Get the value corresponding to a field from the request
// returns NULL if not found
char *get_field_value(Request *req, char *field)
{
    for (int i = 0; i < req->headers->size; i++)
    {
        char *f = req->headers->field[i];
        for (; *f; ++f)
            *f = tolower(*f);

        if (!strcmp(f, field))
            return req->headers->value[i];
    }
    return NULL;
}

// PUT method
void recv_file()
{
}

int main()
{
    FILE *log = fopen("access_log.txt", "a");

    int sockfd, newsockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(1);
    }

    struct sockaddr_in serv_addr, cli_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&cli_addr, 0, sizeof(cli_addr));

    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(PORT_NO);

    printf("Started Server at IP : %s, Port: %d", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Socket binding failed");
        exit(1);
    }

    // listen queue of 5
    listen(sockfd, 5);

    while (1)
    {

        int cli_len = sizzeof(cli_addr);

        printf("\nWaiting to establish a connection...\n");

        if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len)) < 0)
        {
            perror("Connection establishment failed");
            exit(1);
        }

        printf("Connection established with IP : %s, Port: %d", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        time_t t = time(NULL);
        struct tm local_t = *localtime(&t);

        int pid = fork();

        // Child process to make a concurrent connection to multiple clients
        if (pid == 0)
        {

            close(sockfd);

            char *buffer = fetch_header(newsockfd);
            Request *req = parse(buffer);

            if (req->method == UNKNOWN_METHOD)
            {
            }

            char *method = (req->method == GET_METHOD) ? "GET" : "PUT";

            fprintf(log, "%d-%d-%d : %d-%d-%d : %s : %s : %s : %s ",
                    local_t.tm_mday, local_t.tm_mon, local_t.tm_year,
                    local_t.tm_hour, local_t.tm_min, local_t.tm_sec,
                    inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port),
                    req->method, req->path);

            close(newsockfd);
            exit(0);
        }

        close(newsockfd);
    }

    fclose(log);
    close(sockfd);

    return 0;
}
