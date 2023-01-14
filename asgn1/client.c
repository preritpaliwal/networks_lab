#include <stdio.h>

#include <stdlib.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <string.h>

#include <arpa/inet.h>

#define BUFFER_SIZE 10
#define MAX_SIZE 100

int stripSpaces(char *a, int sizeA, char *b)
{
    int j = 0;
    for (int i = 0; i < sizeA; i++)
    {
        if (a[i] == ' ')
        {
            continue;
        }
        else
        {
            b[j++] = a[i];
        }
    }
    b[j] = '\0';
    return j;
}

int main()
{
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0)
    {
        perror("failed in creating a socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(20001);
    inet_aton("127.0.0.1", &serv_addr.sin_addr);

    if (connect(sockFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("failed to connect");
        exit(EXIT_SUCCESS);
    }

    int breakBit = -1;
    while (1)
    {
        int i = 2, j = 0;
        char *expression;
        printf("Enter any expression: ");

        expression = malloc(sizeof(char));
        while ((expression[j] = getchar()) != '\n')
        {
            expression = (char *)realloc(expression, sizeof(char) * i);
            i++;
            j++;
        }

        expression[j] = '\0';
        // printf("%s", expression);
        char exp[j];
        int newLen = stripSpaces(expression, j, exp);
        // printf("stripped exp: %s\n", exp);
        if (newLen == 2)
        {
            if (exp[0] == '-' && exp[1] == '1')
            {
                breakBit = 1;
            }
        }

        int lenSent = 0;
        char buffer[BUFFER_SIZE];
        while (newLen > 0)
        {
            int lenbuf = -1;
            if (newLen + 1 > BUFFER_SIZE)
            {
                for (int i = 0; i < BUFFER_SIZE - 1; i++)
                {
                    buffer[i] = exp[i + lenSent];
                }
                buffer[BUFFER_SIZE - 1] = '\0';
                lenbuf = BUFFER_SIZE - 1;
            }
            else
            {
                for (int i = 0; i < newLen; i++)
                {
                    buffer[i] = exp[i + lenSent];
                }
                buffer[newLen] = '\0';
                lenbuf = newLen;
            }
            int returnValOfSend = send(sockFD, buffer, lenbuf + 1, 0);
            printf(" Expression sent: %s\n", buffer);
            newLen -= lenbuf;
            lenSent += lenbuf;
        }

        buffer[0] = '\0';
        int returnValOfSend = send(sockFD, buffer, 1, 0);
        // printf("returnValOfSend: %d\n", returnValOfSend);

        if (breakBit == 1)
        {
            break;
        }

        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            buffer[i] = '\0';
        }
        int returnValOfRecv = recv(sockFD, buffer, 100, 0);
        // printf("returnValOfRecv: %d\n", returnValOfRecv);
        printf("Value of expression: %s = %s\n", expression, buffer);
    }
    printf("Byeee, I hope you were able to solve all the expressions!!\n");
}