#include "csapp.h"

void unix_error(char *msg){
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

void posix_error(int code, char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(0);
}

void dns_error(char *msg) {
    fprintf(stderr, "%s: DNS error\n", msg);
    exit(0);
}

void app_error(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(0);
}

ssize_t rio_readn(int fd, void *usrbuf, size_t n){
    size_t nleft=n;
    ssize_t nread;
    char *bufp=usrbuf;

    while(nleft>0){
        if((nread=read(fd, bufp, nleft))<0){
            if(errno==EINTR){
                nread=0;
            }
            else return -1;
        }
        else if(nread==0) break;
        nleft-=nread;
        bufp+=nread;
    }
    return (n-nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n){
    size_t nleft=n;
    ssize_t nwritten;
    char *bufp=usrbuf;

    while(nleft>0){
        if((nwritten=write(fd, bufp, nleft))<=0){
            if(errno==EINTR){
                nwritten=0;
            }
            else return -1;
        }
        nleft-=nwritten;
        bufp+=nwritten;
    }
    return n;
}

void rio_readinitb(rio_t *rp, int fd){
    rp->rio_fd=fd;
    rp->rio_cnt=0;
    rp->rio_bufptr=rp->rio_buf;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n){
    int cnt;

    while(rp->rio_cnt <=0){
        rp->rio_cnt=read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt<0){
            if(errno!=EINTR) return -1;
        }
        else if(rp->rio_cnt ==0){
            return 0;
        }
        else{
            rp->rio_bufptr=rp->rio_buf;
        }
    }

    cnt=n;
    if(rp->rio_cnt<(int)n){
        cnt=rp->rio_cnt;
    }
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr+=cnt;
    rp->rio_cnt-=cnt;
    return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen){
    int n, rc;
    char c, *bufp=usrbuf;

    for(n=1; n<maxlen; n++){
        if((rc=rio_read(rp, &c, 1))==1){
            *bufp++ = c;
            if(c=='\n'){
                n++;
                break;
            }
        }
        else if(rc==0){
            if(n==1) return 0;
            else break;
        }
        else return -1;
    }
    *bufp=0;
    return (n-1);
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n){
    size_t nleft=n;
    ssize_t nread;
    char *bufp=usrbuf;

    while(nleft>0){
        if((nread=rio_read(rp, bufp, nleft))<0) return -1;
        else if(nread==0) break;
        nleft-=nread;
        bufp+=nread;
    }
    return (n-nleft);
}

void Rio_readinitb(rio_t *rp, int fd){
    rio_readinitb(rp, fd);
}

ssize_t Rio_readn(int fd, void *usrbuf, size_t n) {
    ssize_t nbytes;
    if ((nbytes = rio_readn(fd, usrbuf, n)) < 0)
        unix_error("Rio_readn error");
    return nbytes;
}

void Rio_writen(int fd, void *usrbuf, size_t n) {
    if (rio_writen(fd, usrbuf, n) != n)
        unix_error("Rio_writen error");
}

ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    ssize_t rc;
    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
        unix_error("Rio_readlineb error");
    return rc;
}

ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n) {
    ssize_t rc;
    if ((rc = rio_readnb(rp, usrbuf, n)) < 0)
        unix_error("Rio_readnb error");
    return rc;
}

int open_listenfd(char *port){
    struct addrinfo hints, *listp, *p;
    int listenfd, rc, optval=1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;

    if((rc=getaddrinfo(NULL, port, &hints, &listp))!=0){
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(rc));
        return -1;
    }

    for(p=listp; p; p=p->ai_next){
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))<0){
            continue;
        }
        setsockopt(listenfd ,SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
        if(bind(listenfd, p->ai_addr, p->ai_addrlen)==0) break;

        close(listenfd);
    }
    freeaddrinfo(listp);

    if(!p) return -1;
    if(listen(listenfd, LISTENQ)<0){
        close(listenfd);
        return -1;
    }
    return listenfd;
}

int Open_listenfd(char *port){
    int rc;
    if((rc=open_listenfd(port))<0){
        unix_error("Open_listenfd error");
    }
    return rc;
}

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int rc;
    if ((rc = accept(s, addr, addrlen)) < 0)
        unix_error("Accept error");
    return rc;
}

void Close(int fd) {
    int rc;
    if ((rc = close(fd)) < 0)
        unix_error("Close error");
}

void Getnameinfo(const struct sockaddr *sa, socklen_t salen,
                 char *host, size_t hostlen,
                 char *serv, size_t servlen, int flags) {
    int rc;

    if ((rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, flags)) != 0) {
        fprintf(stderr, "Getnameinfo error: %s\n", gai_strerror(rc));
        exit(0);
    }
}

/********************************
 * Unix I/O 래퍼 구현
 ********************************/
int Open(const char *pathname, int flags, mode_t mode) {
    int rc;
    if ((rc = open(pathname, flags, mode)) < 0)
        unix_error("Open error");
    return rc;
}

/********************************
 * 메모리 매핑(mmap) 래퍼 구현
 ********************************/
void *Mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void *ptr;
    if ((ptr = mmap(addr, length, prot, flags, fd, offset)) == MAP_FAILED)
        unix_error("mmap error");
    return ptr;
}

void Munmap(void *start, size_t length) {
    if (munmap(start, length) < 0)
        unix_error("munmap error");
}

/********************************
 * 프로세스 제어 래퍼 구현
 ********************************/
pid_t Fork(void) {
    pid_t pid;
    if ((pid = fork()) < 0)
        unix_error("Fork error");
    return pid;
}

void Execve(const char *filename, char *const argv[], char *const envp[]) {
    if (execve(filename, argv, envp) < 0)
        unix_error("Execve error");
}

pid_t Wait(int *status) {
    pid_t pid;
    if ((pid = wait(status)) < 0)
        unix_error("Wait error");
    return pid;
}

int Dup2(int fd1, int fd2) {
    int rc;
    if ((rc = dup2(fd1, fd2)) < 0)
        unix_error("Dup2 error");
    return rc;
}