#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cassert>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<vector>

static const int PORT = 4442;

static std::mutex m;
static std::condition_variable cv;
static bool start = false;


void query() {
    struct sockaddr_in addr;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
        perror("Fail.");
        return;
    }
    if(connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("Fail.");
        return;
    }
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, []{return start;});
    }
    for(int i=0; i<10000; i++) {
        char q[10];
        strcpy(q, "hello\n");
        int l = strlen(q);
        write(s, q, l);
        q[0] = '\0';
        read(s, q, l);
        assert(strcmp(q, "hello\n") == 0);
    }
    close(s);
}

int main(int argc, char **argv) {
    const int threads = 10;
    std::vector<std::thread> qs;
    for(int i=0; i<threads; i++) {
        qs.emplace_back(query);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> l(m);
        start = true;
    }
    cv.notify_all();
    for(auto &i : qs) {
        printf("Join\n");
        i.join();
    }
    return 0;
}
