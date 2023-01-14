#include <stdio.h>

#include <stdlib.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <unistd.h>

#include <string.h>

#define BUFFER_SIZE 10
#define MAX_SIZE 100

char *processBracketLessExpression(char *exp, double *val)
{
    char *newExp;
    double curAns = strtod(exp, &newExp);
    exp = newExp + 1;
    while (1)
    {
        if (*exp == '(')
        {
            break;
        }
        else if (*(exp - 1) == ')')
        {
            exp++;
            break;
        }
        if (*(exp-1) == '\0')
        {
            break;
        }
        double ret = 0;
        ret = strtod(exp, &newExp);
        // printf("curAns: %f,ret: %f, op: %c\n", curAns, ret, *(exp - 1));
        switch (*(exp - 1))
        {
        case '+':
            curAns += ret;
            break;
        case '-':
            curAns -= ret;
            break;
        case '*':
            curAns *= ret;
            break;
        case '/':
            curAns /= ret;
            break;
        default:
            break;
        }
        exp = newExp + 1;
    }
    *val = curAns;
    return exp;
}

double processExpression(char *exp, int expLen)
{
    double curAns = 0;
    int first = 1;
    char *newExp;
    // printf("explen = %d",expLen);
    // exp[expLen]='\0';
    while (*exp != '\0')
    {   
        // printf("exp: %s",exp);
        double val = 0;
        if (*exp == '(')
        {
            newExp = processBracketLessExpression(exp + 1, &val);
        }
        else
        {
            newExp = processBracketLessExpression(exp, &val);
        }
        if (first)
        {
            curAns = val;
        }
        else
        {
            // printf("operator: %c, val: %f", *(exp - 1), val);
            switch (*(exp - 1))
            {
            case '+':
                curAns += val;
                break;
            case '-':
                curAns -= val;
                break;
            case '*':
                curAns *= val;
                break;
            case '/':
                curAns /= val;
                break;
            default:
                break;
            }
        }
        if (first)
        {
            first = 0;
        }
        exp = newExp;
        printf("\ncurrent Ans: %f\n\n", curAns);
    }
    return curAns;
}

int main()
{
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0)
    {
        perror("failed to create socket..!!");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(20001);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("failed to bind socket to the given IP address and port");
        exit(EXIT_SUCCESS);
    }
    listen(sockFD, 5);
    while (1)
    {

        struct sockaddr_in cli_addr;
        int len_cli = sizeof(cli_addr);
        printf("waiting for connection\n");
        int newSockFD = accept(sockFD, (struct sockaddr *)&cli_addr, &len_cli);

        if (newSockFD < 0)
        {
            perror("failed to accept the client request");
            exit(EXIT_SUCCESS);
        }

        printf("connected\n");

        while (1)
        {
            char exp[MAX_SIZE];
            for(int i = 0;i<MAX_SIZE;i++){
                exp[i] = '\0';
            }
            int breakBit = -1;
            int expLen = 0;
            while (breakBit != 1)
            {
                char buffer[BUFFER_SIZE];
                for (int i = 0; i < BUFFER_SIZE; i++)
                {
                    buffer[i] = '\0';
                }
                int rec = recv(newSockFD, buffer, BUFFER_SIZE, 0);
                // printf("value recv: %s and return value of recv: %d\n", buffer, rec);
                int i = 0;
                for (i = 0; i < BUFFER_SIZE; i++)
                {
                    if (i != 0)
                    {
                        if (buffer[i] == '\0' && buffer[i - 1] == '\0')
                        {
                            breakBit = 1;
                            break;
                        }
                    }
                    exp[i + expLen] = buffer[i];
                }
                expLen += i - 1;
            }

            if (expLen == 2)
            {
                if (exp[0] == '-' && exp[1] == '1')
                {
                    break;
                }
            }
            if (exp[0] == '\0')
            {
                continue;
            }
            printf("calculating result for exp: %s and expLen: %d\n", exp, expLen);
            double getResult = processExpression(exp, expLen);
            char answer[MAX_SIZE];
            sprintf(answer, "%f", getResult);
            // printf("sending value: %s or %f\n", answer, getResult);
            int returnval = send(newSockFD, answer, strlen(answer) + 1, 0);
            printf("sent ans: %s \n", answer);
        }

        printf("closing connection\n");
        close(newSockFD);
    }
    return 0;
}