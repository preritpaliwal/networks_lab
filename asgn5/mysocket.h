#include <sys/socket.h>
#include <netinet/in.h>
#include "queue.h"

typedef struct MyFD{
    int sock_fd;
    int flag;
    Q receiveTable;
    Q sendTable;
};

struct MyFD *my_socket(int __domain,int __type,int __protocol);
int my_bind(struct MyFD *__fd,const struct sockaddr *__addr,socklen_t __len);
int my_listen(struct MyFD *__fd, int __n);
int my_accept(struct MyFD *__fd,struct sockaddr *__addr,socklen_t *__addr_len);
int my_connect(struct MyFD *__fd,struct sockaddr * __addr, socklen_t  __addr_len);
ssize_t my_send(struct MyFD *__fd,const void *__buf, size_t __n, int __flags);
ssize_t my_recv(struct MyFD *__fd, void *__buf, size_t __n, int __flags);
int my_close(struct MyFD *__fd);