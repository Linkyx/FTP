#include"my_error.h"

void do_cd(int conn_fd, char *recv_cmd);
void do_ls(int conn_fd, char *recv_cmd);
void do_get(int conn_fd, char *recv_cmd);
void do_quit(int conn_fd, char *recv_cmd);
void do_put(int conn_fd, char *recv_cmd);
int main(void)
{
    char recv_buf[BUF_SIZE];
    char send_buf[BUF_SIZE];
    pid_t pid;
 //   char *user_ip;
    int len;
    int optval;
    int sock_fd, conn_fd;
    int cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    //服务器默认地址
    chdir("/home/linky/FTP/server");

    signal(SIGCHLD,SIG_IGN);
    //创建套接字
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0)
    {
        printf("创建套接字出错\n");
        my_error("socket", __LINE__);
    }

    //若长时间某客户端没有反应,服务器自动关闭
    optval = 1;
    //设置该套接字可以重新绑定端口
    if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(int) )<0)
    {
        printf("重新绑定端口失败\n");
        my_error("setsockopt", __LINE__);
    }

    //初始化服务器地址
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //将套接字绑定端口
    if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) < 0)
    {
        printf("绑定套接字失败\n");
        my_error("bind", __LINE__);
    }

    //将套接字转化为监听套接字
    if(listen(sock_fd, LISTENQ) < 0)
    {
        printf("转化监听套接字失败\n");
        my_error("listen", __LINE__);
    }

    //将套接字设置为阻塞方式并接受连接
    cli_len = sizeof(struct sockaddr_in);

    while(1)
    {
        //循环接受每一个连接请求
        conn_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &cli_len);

        if(conn_fd < 0)
        {
            printf("连接失败\n");
            my_error("accept", __LINE__);
        }

        printf("accept client %s\n", inet_ntoa(cli_addr.sin_addr));
        //发送欢迎信息
        if(send(conn_fd, "welcome to my server\n", 21, 0) < 0)
        {
            my_error("send", __LINE__);
        }

        //创建子进程处理链接请求
        if((pid =fork()) == 0)
        {
           //printf("accept client %s\n", inet_ntoa(cli_addr.sin_addr));
            while(1)
            {
                recv(conn_fd, &len, sizeof(len), 0);
                recv(conn_fd, recv_buf, len, 0);
                if(strncmp(recv_buf, LS, strlen(LS)) == 0)
                {
                    do_ls(conn_fd, recv_buf);
                }
                else if(strncmp(recv_buf, CD, strlen(CD)) == 0)
                {
                    do_cd(conn_fd, recv_buf);
                    //printf("%s", recv_buf);
                }
                else if(strncmp(recv_buf, GET, strlen(GET)) == 0)
                {
                    do_get(conn_fd, recv_buf);
                }
                else if(strncmp(recv_buf, PUT, strlen(PUT)) == 0)
                {
                    do_put(conn_fd, recv_buf);
                }
                else if(strncmp(recv_buf, QUIT, strlen(QUIT)) == 0)
                {
                    do_quit(conn_fd, recv_buf);
                }
                bzero(&len, sizeof(len));
                memset(recv_buf, 0, BUF_SIZE);

            }

            close(sock_fd);
            close(conn_fd);
            exit(0);
        }
        else
        {
            close(conn_fd);
        }

    }


    return 0;

}
