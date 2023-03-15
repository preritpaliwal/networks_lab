#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "queue.h"

typedef struct MyFD{
    int sock_fd;
    int flag;
    Q recvTable;
    Q sendTable;
    pthread_t readThread;
    pthread_t writeThread;
    pthread_mutex_t recvTableLock;
    pthread_mutex_t sendTableLock;
}MyFD;

MyFD *my_socket(int __domain,int __type,int __protocol);
int my_bind(MyFD *__fd,const struct sockaddr *__addr,socklen_t __len);
int my_listen(MyFD *__fd, int __n);
int my_accept(MyFD *__fd,struct sockaddr *__addr,socklen_t *__addr_len);
int my_connect(MyFD *__fd,struct sockaddr * __addr, socklen_t  __addr_len);
ssize_t my_send(MyFD *__fd,const char *__buf, size_t __n, int __flags);
ssize_t my_recv(MyFD *__fd, char *__buf, size_t __n, int __flags);
int my_close(MyFD *__fd);