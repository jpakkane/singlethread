#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>

static const int PORT = 4442;

void query(int s) {
    char q[10];
    strcpy(q, "hello\n");
    int l = strlen(q);
    write(s, q, l);
    q[0] = '\0';
    read(s, q, l);
    printf("%s\n", q);
}

int main(int argc, char **argv) {
    struct sockaddr_in addr;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htonl(PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
        perror("Fail.");
        return 1;
    }
    if(connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("Fail.");
        return 1;
    }
    query(s);
    return 0;
}
