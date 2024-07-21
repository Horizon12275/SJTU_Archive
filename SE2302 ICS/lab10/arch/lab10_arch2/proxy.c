/*
 * proxy.c - ICS Web proxy
 * 赵楷越 522031910803
 */
#include "csapp.h"
#include <stdarg.h>
#include <sys/select.h>

/*
 * Function prototypes
 */

// 解析uri，提取主机名、路径名和端口号
int parse_uri(char *uri, char *target_addr, char *path, char *port);

// 日志函数
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, size_t size);

// 解析请求头，发送请求头，并返回请求体的长度
int header_writer(int fd, rio_t *rio, int *cnt);

// 处理连接的doit函数
void doit(int fd, struct sockaddr_in *sockaddr);

// RIO包装函数
ssize_t Rio_writen_w(int fd, void *usrbuf, size_t n);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n);

// 线程的入口函数，参数是一个指向 thread_arg 结构体的指针
void *thread(void *thread_args);

// 为了方便地传入线程的入口函数，创建用于存储连接的地址和套接字文件描述符的结构体
typedef struct
{
    struct sockaddr_in addr;
    int fd;
} thread_arg;

// 信号量，用于保护对服务器描述符的访问，这里需要3个信号量，一个用于保护服务器描述符的访问，一个用于保护打印日志的访问，一个用于保护字符串的访问
sem_t s_open, m_print, m_string;

/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;                // 是一个无符号整数类型，用于存储客户端地址的长度
    struct sockaddr_storage clientaddr; // 用于存储客户端套接字地址

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

    // To keep your proxy from crashing you can use the SIG IGN argument to the
    // signal function (CS:APP 8.5.3) to explicitly ignore these SIGPIPE signals
    Signal(SIGPIPE, SIG_IGN);

    // 初始化信号量，第一个参数是信号量的地址，第二个参数是pshared，第三个参数是信号量的初始值
    // pshared为0表示信号量是进程内共享的，否则是进程间共享的
    // 这里的信号量是用来保护对服务器描述符的访问的，初始化为1表示只有一个线程可以访问服务器描述符
    // 保证了对服务器描述符的访问是互斥的
    // 初始化为1表示在同一时间只有一个线程可以访问服务器描述符
    Sem_init(&s_open, 0, 1);
    Sem_init(&m_print, 0, 1);
    Sem_init(&m_string, 0, 1);

    clientlen = sizeof(clientaddr);    // 将clientaddr的大小赋值给clientlen，为了在调用Accept函数时传入clientaddr的大小
    listenfd = Open_listenfd(argv[1]); // 根据命令行参数创建监听描述符

    while (1)
    {
        // 接受一个新的连接，并将连接的文件描述符赋值给 connfd，
        // 将客户端的地址信息存储在 clientaddr 中，
        // 将地址结构的长度保存在 clientlen 中。
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        thread_arg *thread_args = Malloc(sizeof(thread_arg));
        // args的addr成员是连接的地址
        memcpy(&thread_args->addr, &clientaddr, sizeof(struct sockaddr_in));
        // args的fd成员是连接的文件描述符
        thread_args->fd = connfd;
        // 创建一个新的线程，标识符为tid，线程的入口函数是 thread，参数是 args。
        pthread_t tid;
        pthread_create(&tid, NULL, thread, thread_args);
    }

    exit(0);
}

// 线程的入口函数，参数是一个指向 thread_arg 结构体的指针
void *thread(void *thread_args)
{
    // To avoid a potentially fatal memory leak, your threads should run as detached, not joinable.
    pthread_detach(pthread_self()); // 将线程设置为分离状态
    int fd = ((thread_arg *)thread_args)->fd;
    struct sockaddr_in *addr = &((thread_arg *)thread_args)->addr;
    doit(fd, addr);
    free(thread_args);
    return NULL;
}

// 从输入流中解析HTTP头部信息，并将其发送到指定的文件描述符。
int header_writer(int fd, rio_t *rp, int *cnt)
{
    char buf[MAXLINE];
    int n, len = 0, count = 0;

    if ((n = Rio_readlineb_w(rp, buf, MAXLINE)) <= 0)
        return -1;
    while (strcmp(buf, "\r\n"))
    {
        count += n;
        if (strcasestr(buf, "Content-Length: "))
        {
            sscanf(buf + strlen("Content-Length: "), "%d", &len);
        }
        if (buf[strlen(buf) - 1] != '\n')
            return -1;
        if (Rio_writen_w(fd, buf, strlen(buf)) <= 0)
            return -1;
        if ((n = Rio_readlineb_w(rp, buf, MAXLINE)) <= 0)
            return -1;
    }
    count += n;
    if (Rio_writen_w(fd, buf, strlen(buf)) <= 0)
        return -1;
    if (cnt != NULL)
    {
        *cnt += count;
    }
    return len;
}

void doit(int fd, struct sockaddr_in *sockaddr)
{
    int server_fd;
    int n, cnt, len;
    char buf[MAXLINE];
    char method[MAXLINE];
    char uri[MAXLINE];
    char version[MAXLINE];
    char hostname[MAXLINE];
    char pathname[MAXLINE];
    char port[MAXLINE];
    char request[MAXLINE];
    char body[102400];
    char logstring[MAXLINE];
    rio_t rio;

    rio_readinitb(&rio, fd);
    /* parse request header */
    if (Rio_readlineb_w(&rio, buf, MAXLINE) <= 0)
    {
        fprintf(stderr, "Bad request\n");
        close(fd);
        return;
    }

    if (sscanf(buf, "%s %s %s", method, uri, version) != 3)
    {
        fprintf(stderr, "sscanf error:[%s]\n", buf);
        close(fd);
        return;
    }

    if (parse_uri(uri, hostname, pathname, port) < 0)
    {
        fprintf(stderr, "Illegal URL\n");
        close(fd);
        return;
    }

    /* open server fd */
    P(&s_open);
    if ((server_fd = open_clientfd(hostname, port)) < 0)
    {
        V(&s_open);
        fprintf(stderr, "Fail to connect server\n");
        close(fd);
        return;
    }
    V(&s_open);
    if (pathname[0] == '\0')
    {
        pathname[0] = '/';
        pathname[1] = '\0';
    }
    P(&m_string);
    sprintf(request, "%s %s %s\r\n", method, pathname, version);
    V(&m_string);

    /* send request */
    if (Rio_writen_w(server_fd, request, strlen(request)) <= 0)
    {
        fprintf(stderr, "Fail to send request\n");
        close(fd);
        close(server_fd);
        return;
    }

    /* read and send header */
    len = header_writer(server_fd, &rio, NULL);
    if (len < 0)
    {
        fprintf(stderr, "parse error or fail to send hdr\n");
        close(fd);
        close(server_fd);
        return;
    }
    /* send body if receive a POST */
    if (strcasecmp(method, "POST") == 0)
    {
        while (len > 0)
        {
            n = Rio_readnb_w(&rio, body, len > 102400 ? 102400 : len);
            if (n <= 0)
            {
                fprintf(stderr, "read body error\n");
                close(fd);
                close(server_fd);
                return;
            }
            if (Rio_writen_w(server_fd, body, len > 102400 ? 102400 : len) <= 0)
            {
                fprintf(stderr, "Fail to send body\n");
                close(fd);
                close(server_fd);
                return;
            }
            len -= 102400;
        }
    }

    rio_readinitb(&rio, server_fd);

    /* get response and send to client */
    /* get header */
    cnt = 0;
    len = header_writer(fd, &rio, &cnt);
    if (len < 0)
    {
        fprintf(stderr, "parse error or fail to send hdr\n");
        close(fd);
        close(server_fd);
        return;
    }

    /* get body */
    cnt += len;
    while (len > 0)
    {
        n = Rio_readnb_w(&rio, body, 1);
        if (n <= 0)
        {
            fprintf(stderr, "read body error\n");
            close(fd);
            close(server_fd);
            return;
        }
        if (Rio_writen_w(fd, body, 1) <= 0)
        {
            fprintf(stderr, "Fail to send body");
            close(fd);
            close(server_fd);
            return;
        }
        len--;
    }

    close(fd);
    close(server_fd);

    // 当请求完成后，调用 format_log_entry 函数，将请求的信息写入到 logstring 中，记录日志
    if (cnt > 0)
    {
        format_log_entry(logstring, sockaddr, uri, cnt);
    }

    // 这里要保护对日志的访问，所以要加锁
    P(&m_print);
    printf("%s\n", logstring);
    fflush(stdout);
    V(&m_print);
}

/*
 * The Rio readn, Rio readlineb, and Rio writen error checking wrappers in csapp.c are not
 * appropriate for a realistic proxy because they terminate the process when they encounter an
 * error. Instead, you should write new wrappers called Rio readn w, Rio readlineb w, and Rio
 * writen w that simply return after printing a warning message when I/O fails. When either
 * of the read wrappers detects an error, it should return 0, as though it encountered EOF on the socket.
 */

// 读取n个字节到userbuf中，fd为文件描述符，userbuf为缓冲区，n为要读取的字节数
// 它的作用是将用户缓冲区 userbuf 中的 n 个字节写入到文件描述符 fd 指向的文件或套接字中
ssize_t Rio_writen_w(int fd, void *userbuf, size_t n)
{
    ssize_t rc;

    if ((rc = rio_writen(fd, userbuf, n)) != n)
    {
        fprintf(stderr, "%s: %s\n", "Rio writen error", strerror(errno));
        return 0;
    }

    return rc;
}

ssize_t Rio_readnb_w(rio_t *fp, void *userbuf, size_t n)
{
    ssize_t rc;

    if ((rc = rio_readnb(fp, userbuf, n)) != n)
    {
        fprintf(stderr, "%s: %s\n", "Rio readnb error", strerror(errno));
        return 0;
    }

    return rc;
}

ssize_t Rio_readlineb_w(rio_t *fp, void *userbuf, size_t maxlen)
{
    ssize_t rc;

    if ((rc = rio_readlineb(fp, userbuf, maxlen)) < 0)
    {
        fprintf(stderr, "%s: %s\n", "Rio readlineb error", strerror(errno));
        return 0;
    }

    return rc;
}

ssize_t Open_clientfd_w(char *hostname, char *port)
{
    ssize_t rc;

    if ((rc = open_clientfd(hostname, port)) < 0)
    {
        fprintf(stderr, "%s: %s\n", "Open_clientfd error", strerror(errno));
        return 0;
    }
    return rc;
}

/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, char *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0)
    {
        hostname[0] = '\0';
        return -1;
    }

    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    if (hostend == NULL)
        return -1;
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* Extract the port number */
    if (*hostend == ':')
    {
        char *p = hostend + 1;
        while (isdigit(*p))
            *port++ = *p++;
        *port = '\0';
    }
    else
    {
        strcpy(port, "80");
    }

    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL)
    {
        pathname[0] = '\0';
    }
    else
    {
        strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), the number of bytes
 * from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
                      char *uri, size_t size)
{
    time_t now;
    char time_str[MAXLINE];
    char host[INET_ADDRSTRLEN];

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    if (inet_ntop(AF_INET, &sockaddr->sin_addr, host, sizeof(host)) == NULL)
        unix_error("Convert sockaddr_in to string representation failed\n");

    /* Return the formatted log entry string */
    P(&m_string);
    sprintf(logstring, "%s: %s %s %zu", time_str, host, uri, size);
    V(&m_string);
}