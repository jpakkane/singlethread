#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<string>

static const int PORT = 4442;
#define MAX_EVENTS 100

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
    int listen_sock, conn_sock, nfds, epollfd;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htonl(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(bind(listen_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("Fail\n");
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
