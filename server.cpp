#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<string>
#include<signal.h>

static const int PORT = 4442;
#define MAX_EVENTS 100

int listen_sock;

static void handler(int) {
    close(listen_sock);
    exit(1);
}

void serve(int fd) {
    std::string buf;
    char inchar;
    buf.reserve(100);
    while(read(fd, &inchar, 1) >0) {
        buf.push_back(inchar);
        if(inchar == '\n')
            break;
    }
    write(fd, buf.c_str(), buf.length());
}

int mainloop() {
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_sock < 0) {
        perror("Create fail");
        exit(EXIT_FAILURE);
    }
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if(sigaction(SIGINT, &sa, nullptr) < 0) {
        perror("Signal fail");
        return 1;
    }
    if(bind(listen_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("Bind fail");
        exit(EXIT_FAILURE);
    }
    if(listen(listen_sock, 10) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    epollfd = epoll_create(1);
    if (epollfd == -1) {
        perror("epoll_create");
        exit (EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit (EXIT_FAILURE);
    }
    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_pwait");
            exit (EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                conn_sock = accept(listen_sock, nullptr, 0);
                if (conn_sock == -1) {
                    perror("accept");
                    exit (EXIT_FAILURE);
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit (EXIT_FAILURE);
                }
            } else {
                serve(events[n].data.fd);
            }
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    return mainloop();
}
