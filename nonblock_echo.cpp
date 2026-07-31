#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int setNonBlocking(int fd)
{
    int oldFlag = fcntl(fd,F_GETFL); //拿到原来的标志位
    int newFlag = oldFlag | O_NONBLOCK; //加上非阻塞标志
    fcntl(fd,F_SETFL,newFlag);
    return oldFlag; //旧值后面可能有用
}

int main(int argc, char* argv[]) {
    //1. 创建 socket
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1)
    {
        perror("socket");
        return 1;
    }
    
    //2. 端口复用 填充地址结构体
    int opt = 1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);
    

    //3. 绑定 + 监听
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

    //4. 把 listenfd 设为非阻塞
    setNonBlocking(listenfd);
    std::cout << "Server listening on port 8080...\n";

    int connfd = -1;
    while(true)
    {
        //5. 尝试 accept（非阻塞）
        if (connfd == -1)
        {
            struct sockaddr_in cliaddr;
            socklen_t len = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &len);
            
            if (connfd >= 0)
            {
                std::cout<<"New connection!\n";
                setNonBlocking(connfd);
            }
            else
            {
                //accept返回-1，有两种可能：
                //1. errno == EAGAIN → 真的没连接，正常
                //2. 其他errno→出错
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    perror("accept");
                    break;
                }
            }
        }
        
        //6. 尝试 read（非阻塞）
        if (connfd >= 0)
        {
            char buf[1024];
            ssize_t n = read(connfd, buf , sizeof(buf)); 
            if (n > 0)
            {
                write(connfd, buf, n);
            }
            else if (n == 0)
            {
                std::cout << "Client closed.\n";
                close(connfd);
                connfd = -1; //重置，准备接受下一个连接
            }
            else
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    perror("read");
                    close(connfd);
                    connfd = -1;
                }
                //如果是EAGAIN:没数据，继续轮询
            }
        }

        //7. 轮询间隔
        usleep(1000); //睡 1ms，让出 CPU
    }
    
    close(listenfd);
    return 0;
    
}