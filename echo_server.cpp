#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    // 1. 创建 socket
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1)
    {
        perror("socket");
        return 1;
    }
    
    // 2. 填充地址结构体
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);
    

    // 3. 绑定 + 监听
    if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind");
        return 1;
    }
    
    if (listen(listenfd, 128) == -1)
    {
        perror("listen");
        return 1;
    }

    std::cout << "Server listening on port 8080...\n";

    // 4. 接受连接（accept）
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    int connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &len);
    if (connfd == -1)
    {
        perror("accept");
        return 1;
    }
    

    std::cout << "New connection accepted!\n";

    // 5. 循环 read → write（回显）
    char buf[1024];

    while (true)
    {
        ssize_t n = read(connfd, buf , sizeof(buf)); 

        if (n > 0)
        {
            write(connfd, buf , n);
        }
        else if (n == 0)
        {
            break;
        }
        else
        {
            perror("read");
            break;
        }
        
    }
    
    // 6. 关闭连接
    close(connfd);
    close(listenfd);
    return 0;
    
}