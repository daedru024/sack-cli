#include "libcli.h"

//#define DEBUG

// --- Windows 相容性巨集與錯誤處理 ---
#ifdef _WIN32
    // 對應函式
    #define close closesocket
    // Windows socket 不支援 write，改用 send
    #define write(s, buf, len) send(s, (const char*)buf, (int)len, 0)
    // Windows 沒有 bzero，改用標準 memset
    #define bzero(b, len) memset(b, 0, len)
    
    // Shutdown 模式對應
    #ifndef SHUT_WR
        #define SHUT_WR SD_SEND
    #endif
    #ifndef SHUT_RD
        #define SHUT_RD SD_RECEIVE
    #endif
    #ifndef SHUT_RDWR
        #define SHUT_RDWR SD_BOTH
    #endif

    // 錯誤碼對應 (將 errno 對應到 WSAGetLastError)
    #define GET_ERROR WSAGetLastError()
    #define ERR_ECONNREFUSED WSAECONNREFUSED
    #define ERR_EBADF WSAENOTSOCK
    #define ERR_ENOTCONN WSAENOTCONN
    #define ERR_EINTR WSAEINTR
    #define ERR_ECONNRESET WSAECONNRESET
#else
    // Linux 保持原樣
    #define GET_ERROR errno
    #define ERR_ECONNREFUSED ECONNREFUSED
    #define ERR_EBADF EBADF
    #define ERR_ENOTCONN ENOTCONN
    #define ERR_EINTR EINTR
    #define ERR_ECONNRESET ECONNRESET
#endif

int Bid(int sockfd, int PlayerID, int amount, int rem_money) {
    char buf[MAXLINE];
    sprintf(buf, "17 %d %d %d", PlayerID, amount, rem_money);
    Write(sockfd, buf, strlen(buf));
    return 0;
}

int Close(int sockfd) {
    if(shutdown(sockfd, SHUT_WR) == -1) {
        if(errno == EBADF || errno == ENOTCONN) return 0;
        else err_sys("close error");
    }
    return 0;
}

int Conn(const char *servip) {
    // Windows 必須先初始化 Winsock
    #ifdef _WIN32
    static int wsa_initialized = 0;
    if (!wsa_initialized) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            err_sys("WSAStartup error");
            return -1;
        }
        wsa_initialized = 1;
    }
    #endif

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        err_sys("socket error");
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    
    if(inet_pton(AF_INET, servip, &servaddr.sin_addr) <= 0) {
        err_sys("inet_pton error");
        return -1;
    }
    
    if(connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
        if(errno == ECONNREFUSED) return -1;
        err_sys("connect error"); 
        return -1;
    }
    
    return sockfd;
}

int Join(int sockfd, int RoomID, const char* username, int PIN) {
    char buf[MAXLINE];
    sprintf(buf, "11 %d %s %05d", RoomID, username, PIN);
    Write(sockfd, buf, strlen(buf));
    return 0;
}

int Lock(int sockfd) {
    Write(sockfd, "3 ", strlen("3 "));
    return 0;
}

int PlayCard(int sockfd, int PlayerID, int Card, int MaskUc) {
    char buf[MAXLINE];
    sprintf(buf, "13 %d %d %d ", PlayerID, Card, MaskUc);
    Write(sockfd, buf, strlen(buf));
    return 0;
}

int Privt(int sockfd, int PIN) {
    char buf[MAXLINE];
    sprintf(buf, "5 %04d", PIN);
    Write(sockfd, buf, strlen(buf));
    return 0;
}

int Recv(int sockfd, char *recvline) {
    fd_set rfds;
    struct timeval tv;
    int sel;

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 150000;

    sel = select(sockfd + 1, &rfds, NULL, NULL, &tv);
    if (sel < 0) {
        if (errno == EINTR) return -1; 
        err_sys("Select");
        return -1;
    }
    else if (sel == 0) { // timeout
// #ifdef DEBUG
//         printf("Timeout\n");
// #endif
        return -2;
    }
    if (FD_ISSET(sockfd, &rfds)) {
        ssize_t n = recv(sockfd, recvline, MAXLINE - 1, 0);
        if (n < 0) {
            if(errno == ECONNRESET) return -4096;
            err_sys("Recv");
        }
#ifdef DEBUG
        if (n == 0) printf("Connection closed\n");
#endif
        recvline[n] = 0;
#ifdef DEBUG
        printf("Recv: %s\n", recvline);
#endif
        return (int)n;
    }

    return -1;
}

void Write(int sockfd, const void *vptr, size_t n) {
    size_t rem;
    ssize_t nw;
    const char *ptr = vptr;
    rem = n;
    while(rem>0) {
        if((nw = write(sockfd, ptr, rem)) <= 0) {
            if(nw<0 && errno == EINTR) nw = 0;
            else {
                err_sys("Write error");
                return;
            }
        }
        rem -= nw;
        ptr += nw;
    }
#ifdef DEBUG
    printf("Sent: %s\n", (char*)vptr);
#endif
    return;
}

void err_msg(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    return;
}

void err_quit(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

void err_sys(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap); 
    fprintf(stderr, ": %s\n", strerror(errno)); 
    va_end(ap);
    exit(1);
}