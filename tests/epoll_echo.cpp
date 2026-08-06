#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

int setNonBlocking(int fd) {
    int oldFlag = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oldFlag | O_NONBLOCK);
    return oldFlag;
}

int main() {
    // 创建 listenfd
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 端口复用
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 绑定 + 监听
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 128);
    
    // 设为非阻塞
    setNonBlocking(listenfd);
    
    // 创建 epoll 实例
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        return 1;
    }
    
    // 注册 listenfd 到 epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);
    
    std::cout << "Epoll server on 8080...\n";
    
    // 事件数组
    struct epoll_event events[1024];
    
    while (true) {
        // 等待事件，-1 :永久阻塞
        int nfds = epoll_wait(epollfd, events, 1024, -1);
        
        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;
            
            if (fd == listenfd) {
                // listenfd 可读 = 有新连接
                struct sockaddr_in cliaddr;
                socklen_t len = sizeof(cliaddr);
                int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
                
                if (connfd >= 0) {
                    std::cout << "New connection!\n";
                    setNonBlocking(connfd);

                    ev.events = EPOLLIN;
                    ev.data.fd = connfd;
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
                }
            } else {
                // 普通连接可读 = 有数据
                char buf[1024];
                ssize_t n = read(fd, buf, sizeof(buf));
                
                if (n > 0) {
                    write(fd, buf, n);
                } else if (n == 0) {
                    std::cout << "Client closed.\n";
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd); // epoll中删除该fd
                } else {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("read");
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                    }
                }
            }
        }
    }
    
    close(epollfd);
    close(listenfd);
    return 0;
}