#include"main.c"

struct file_stat
{
    char file_name[31];//存储文件名
    char str[10];       //存储文件属性
    int nlink;          //文件连接数
    char user[21];      //文件所有者
    char group[21];     //所有者所在组
    int size;           //文件大小
    char time[41];      //文件创建时间

};


void do_help()
{
    printf("-----------------欢迎使用----------------\n");
    printf("<help>\t<cd>\t<ls>\t<get>\t<put>\t<quit>\n");
    printf("-------------------------------------------\n");
    return;
}


void do_cd(int conn_fd , char *cmd)
{
    char real_cmd[40];      //具体命令,代替cmd
    char recv_buf[BUF_SIZE];     //接收数据
    int cmd_len;        //命令长度

    bzero(recv_buf, BUF_SIZE);

    //接受当前目录并输出
    recv(conn_fd, &cmd_len, sizeof(cmd_len), 0);
    recv(conn_fd, recv_buf, cmd_len, 0);
    printf("当前目录:%s\n", recv_buf);

    bzero(recv_buf, BUF_SIZE);
}

void do_ls(int conn_fd, char *cmd)
{
    int flag;
    struct file_stat file[100];
    int i = 0, j = 0;
    char *real_cmd = cmd;
    struct stat *buf;
    int cmd_len;
    char recv_buf[BUF_SIZE];
    int size;
    int count;

    bzero(recv_buf, BUF_SIZE);
    memset(file, 0, sizeof(file));

    //接受ls的参数以及文件个数
    recv(conn_fd, &flag, sizeof(flag), 0);
    recv(conn_fd, &count, sizeof(count), 0);

    //接受文件属性内容
    while(i < 100)
    {
        recv(conn_fd, &file[i], sizeof(file[i]), 0);
        i++;
    }

    if(flag == 1)
    {//有 -l参数
        while(j < count)
        {
            printf("%s  %d  %s  %s  %d  %s  %s\n", file[j].str, file[j].nlink, file[j].user, file[j].group, file[j].size, file[j].time, file[j].file_name);
            j++;
        }
        printf("\n");
    }

    if(flag == 0)
    {   //ls
        while(j < count)
        {
            printf("%s   ", file[j].file_name);
            j++;
        }
        printf("\n");
    }

    if(flag == 2)
    {//-a
        while(j < count)
        {
            printf("%s   ", file[j].file_name);;
            j++;
        }
        printf("\n");
    }

    if(flag == 3)
    {//-la
        while(j < count)
        {
            printf("%s  %d  %s  %s  %d  %s  %s\n", file[j].str, file[j].nlink, file[j].user, file[j].group, file[j].size, file[j].time, file[j].file_name);
            j++;
        }
        printf("\n");
    }

    //清零
    bzero(&flag, sizeof(flag));
    bzero(file, sizeof(file));
    bzero(&count, sizeof(count));
}



void do_quit(int conn_fd, char *cmd)
{

    int cmd_len;
    char recv_buf[BUF_SIZE];
 //   int conn_fd;

    bzero(recv_buf, BUF_SIZE);

    //发送命令
    cmd_len = strlen(cmd)+1;
    send(conn_fd, &cmd_len, sizeof(cmd_len), 0);
    send(conn_fd, cmd, cmd_len, 0);

    //接受退出信息
//    recv(conn_fd, &cmd_len, sizeof(cmd_len), 0);
    recv(conn_fd, recv_buf, BUF_SIZE, 0);
    printf("%s\n", recv_buf);

    bzero(recv_buf, BUF_SIZE);
    exit(0);

}

void do_get(int conn_fd, char *cmd)
{
    struct stat file;
    char buf[BUF_SIZE];
    int flag = 0;
    char filename[100];
    int count;
    int ret;
    int fd;

    bzero(&file, sizeof(file));

    //接受文件是否存在的标志
    recv(conn_fd, &flag, sizeof(flag),0);

    //文件不存在
    if(flag == 0)
    {
        printf("文件不存在,请重新输入\n");
        return;
    }

    //文件存在
    else
    {
        printf("开始下载\n");
        //获取文件名
        recv(conn_fd, filename, sizeof(filename), 0);
        //打开文件
        if((fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) < 0)
        {
            my_error("open", __LINE__);
        }

        //接受读取次数
        recv(conn_fd, &count, sizeof(count), 0);
        //recv(conn_fd, &file, sizeof(file), 0);

        //接受文件内容并写入文件
        while(count--)
        {
            recv(conn_fd, &ret, sizeof(ret), 0);
            recv(conn_fd, &buf, ret, 0);
            write(fd, buf, ret);
            bzero(buf, BUF_SIZE);
        }
        printf("文件传输完成\n");
    }

    close(fd);
//    bzero(&flag, sizeof(flag));
    bzero(buf, BUF_SIZE);
    bzero(filename, sizeof(filename));
    return;

}

void do_put(int conn_fd, char *cmd)
{
    char *_cmd_;
    DIR *dir;
    struct dirent *ptr;
    char filename[100];
    int flag = 0;
    int fd;
    struct stat file;
    int count;
    int ret;
    char buf[BUF_SIZE];
    int len;

    bzero(filename, sizeof(filename));
    bzero(&file, sizeof(file));
    bzero(buf, BUF_SIZE);

    //获取文件名
    _cmd_ = cmd + strlen(PUT) + 1;
    strcpy(filename, _cmd_);
    //printf("%s\n", filename);

    //打开目录判断是否存在文件
    if((dir = opendir(".")) == NULL)
    {
        my_error("opendir",__LINE__);
    }
    while((ptr = readdir(dir)) != NULL)
    {
        if(strcmp(filename, ptr->d_name) == 0)
        {
            flag = 1;
        }
    }

    //告诉服务器文件是否存在
    if((len = send(conn_fd, &flag, sizeof(flag),0)) < 0)
    {
        my_error("send", __LINE__);
        //printf("%d\n", len);
    }
    printf("%d\n", len);

    //不存在
    if(flag == 0)
    {
        printf("文件不存在,请重新输入\n");
        return;
    }

    //文件存在
    else
    {
 //       printf("%s\n", filename);
        printf("开始上传\n");
        //发送文件名
        send(conn_fd, filename, sizeof(filename), 0);
        //打开文件
        fd = open(filename, O_WRONLY);

        //获取文件属性
        stat(filename, &file);

        //要发送的次数
        count = file.st_size / BUF_SIZE +1;
        send(conn_fd, &count, sizeof(count), 0);

        //读取文件内容并上传给服务器
        while(count--)
        {
            ret = read(fd, buf, BUF_SIZE);
            send(conn_fd, &ret, sizeof(ret), 0);
            send(conn_fd, &buf, ret, 0);
            bzero(buf, BUF_SIZE);
        }
        printf("文件上传完毕\n");
    }
        close(fd);
        bzero(buf, BUF_SIZE);
        bzero(&file, sizeof(file));
        bzero(filename, sizeof(filename));
     //   printf("文件上传完毕\n");
    return;
}

