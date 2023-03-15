#include <sys/socket.h>
#include <netinet/in.h>
#include "queue.h"

struct MyFD{
    int sock_fd;
    int flag;
    Q receiveTable;
    Q sendTable;
};

int my_socket(int __domain,int __type,int __protocol);
int my_bind(MyFD __fd,const struct sockaddr *__addr,socklen_t __len);
int my_listen(MyFD __fd, int __n);
int my_accept(MyFD __fd,struct sockaddr *__addr,socklen_t *__addr_len);
int my_connect(MyFD __fd,struct sockaddr * __addr, socklen_t  __addr_len);
ssize_t my_send(MyFD __fd,const void *__buf, size_t __n, int __flags);
ssize_t my_recv(MyFD __fd, void *__buf, size_t __n, int __flags);
int my_close(MyFD __fd);