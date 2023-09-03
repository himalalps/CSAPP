#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
static const char *http_version = "HTTP/1.0";
static const char *connection = "Connection: close\r\n";
static const char *proxy_connection = "Proxy-Connection: close\r\n";

void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmst);
void send_requesthdrs(int connfd, char *method, char *path, char *hostPort,
                      rio_t rio);
void read_response(int connfd, int fd);

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    } else if (atoi(argv[1]) <= 1024 || atoi(argv[1]) >= 65536) {
        fprintf(stderr, "proxy: port number must be greater than 1024 and less "
                        "than 65536\n");
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr,
                        &clientlen); // line:netp:tiny:accept
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port,
                    MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);  // line:netp:tiny:doit
        Close(connfd); // line:netp:tiny:close
    }
    printf("%s", user_agent_hdr);
    return 0;
}

void doit(int fd) {
    int connfd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char path[MAXLINE], host[MAXLINE], port[MAXLINE], hostPort[MAXLINE];
    rio_t rio;
    char *token;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)) // line:netp:doit:readrequest
        return;
    printf("%s", buf);

    sscanf(buf, "%s http://%s %s", method, uri,
           version);                 // line:netp:doit:parserequest
    if (strcasecmp(method, "GET")) { // line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
    }
    strcpy(path, uri);
    token = strtok(uri, "/");
    if (token == NULL) {
        clienterror(fd, uri, "400", "Bad Request",
                    "Proxy does not understand this URI");
        return;
    }
    strcpy(host, token);
    strcpy(path, path + strlen(host));
    strcpy(hostPort, host);

    token = strtok(host, ":");
    token = strtok(NULL, ":");
    strcpy(port, token);

    connfd = Open_clientfd(host, port);

    send_requesthdrs(connfd, method, path, hostPort, rio);

    read_response(connfd, fd);

    Close(connfd);

    return;
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg) {
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Proxy Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor="
                 "ffffff"
                 ">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Proxy</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}

void send_requesthdrs(int connfd, char *method, char *path, char *hostPort,
                      rio_t rio) {
    char buf[MAXLINE];

    sprintf(buf, "%s %s %s\r\n", method, path, http_version);
    Rio_writen(connfd, buf, strlen(buf));
    sprintf(buf, "Host: %s\r\n", hostPort);
    Rio_writen(connfd, buf, strlen(buf));
    sprintf(buf, "%s", user_agent_hdr);
    Rio_writen(connfd, buf, strlen(buf));
    sprintf(buf, "%s", connection);
    Rio_writen(connfd, buf, strlen(buf));
    sprintf(buf, "%s", proxy_connection);
    Rio_writen(connfd, buf, strlen(buf));

    Rio_readlineb(&rio, buf, MAXLINE);
    printf("%s", buf);
    Rio_writen(connfd, buf, strlen(buf));
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(&rio, buf, MAXLINE);
        printf("%s", buf);
        Rio_writen(connfd, buf, strlen(buf));
    }
}

void read_response(int connfd, int fd) {
    char buf[MAXLINE];
    rio_t rio;
    size_t n;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE))) {
        printf("%s", buf);
        Rio_writen(fd, buf, n);
    }

    return;
}