#include"my_error.h"


void print_prompt()
{
    printf("ftp>");
}

void do_cd(int conn_fd, char *cmd);
void do_ls(int conn_fd, char *cmd);
void do_get(int conn_fd, char *cmd);
void do_put(int conn_fd, char *cmd);
void do_quit(int conn_fd, char *cmd);
void do_help();

int main(int argc, char**agrv)
{
    char cmd[21];
    int conn_fd;
    int serv_port;
    struct sockaddr_in serv_addr;
    char recv_buf[BUF_SIZE];
    int len;
    char send_buf[BUF_SIZE];
    char ip[21];
    char dir[100];


    //初始化服务器端地址
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);
    printf("请输入ip地址\n");

    fgets(ip, sizeof(ip), stdin);

    if(inet_aton(ip, &serv_addr.sin_addr) == 0)
    {

        printf("ip地址转换出错\n");
        my_error("inet_aton", __LINE__);
        //printf("ip地址转换出错\n");
    }

    //创建套接字
    if((conn_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("创建套接字失败\n");
        my_error("socket", __LINE__);
    }
    //连接服务器
    if(connect(conn_fd, (struct sockaddr*)& serv_addr, sizeof(struct sockaddr_in)) < 0)
    {
        printf("连接服务器失败\n");
        my_error("connect",__LINE__);
    }

    //接受服务器欢迎信息
    else
    {
        if(recv(conn_fd, recv_buf, BUF_SIZE, 0) < 0)
    {
        my_error("recv", __LINE__);
    }
    printf("%s\n", recv_buf);
    }

    //默认目录
    strcpy(dir, "/home/linky/FTP/client/");
    strcat(dir, ip);
    //dir[strlen(dir)] = '\0';
    mkdir(dir, 0777);
    chdir(dir);

    do_help();
    //输入并执行相应命令对应的函数
    while(1)
    {
//        do_help();
        print_prompt();
        bzero(cmd, sizeof(cmd));
        gets(cmd);

        //向服务器端发送命令
        len = strlen(cmd) + 1;
        send(conn_fd, &len, sizeof(len), 0);
        send(conn_fd, cmd, sizeof(cmd), 0);


        if(strncmp(cmd, CD, strlen(CD)) == 0)
        {
            do_cd(conn_fd ,cmd);
        }
        else if(strncmp(cmd, LS, strlen(LS)) == 0)
        {
            do_ls(conn_fd, cmd);
        }
        else if(strncmp(cmd, GET, strlen(GET)) == 0)
        {
            do_get(conn_fd, cmd);
        }
        else if(strncmp(cmd, PUT, strlen(PUT)) == 0)
        {
            do_put(conn_fd, cmd);
        }
        else if(strncmp(cmd, QUIT, strlen(QUIT)) == 0)
        {
            do_quit(conn_fd, cmd);
        }
        else if(strncmp(cmd, HELP, strlen(HELP)) == 0)
        {
            do_help();
        }
        else
        {
            printf("输入命令有误, 请重新输入\n");
        }
    }
    return 0;
}
