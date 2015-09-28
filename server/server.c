#include"main.c"

struct file_stat
{
    char file_name[31];
    char str[10];
    int nlink;
    char user[21];
    char group[21];
    int size;
    char time[41];
};

void display_dir(int aflag, int lflag,int conn_fd ,char *filename);
void display_file(int aflag, int lflag, char *path, char *name, struct file_stat *file);



void server_log(char *recv_buf)
{
    int fd = 0;
    char file_name[100];
    struct flock lock;
    time_t _time_;
    char system_time[51];

    //清零
    bzero(&lock, sizeof(lock));
    bzero(&_time_, sizeof(time));

    //服务器日志,默认目录以及文件名
    strcpy(file_name, "/home/linky/FTP/server");
    strcat(file_name,"/server_log.txt");
    //打开文件
    if((fd = open(file_name, O_RDWR|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR)) < 0)
    {
        my_error("open",__LINE__);
    }

    //对文件加写锁
    lock.l_len = 0;
    lock.l_whence = 0;
    lock.l_start = SEEK_SET;
    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLK, &lock);

    //获取当前系统时间
    time(&_time_);
    strcpy(system_time, ctime(&_time_));

    //将命令,以及时间写入文件中
    write(fd, recv_buf, strlen(recv_buf));
    write(fd, "\t", 1);
    write(fd, system_time, strlen(system_time));
    write(fd, "\n", 1);

    //释放锁
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    //关闭文件描述符
    close(fd);

    return;
}


void do_quit(int conn_fd, char *real_cmd)
{
    send(conn_fd,"goodbye~\n" , 9, 0);
    close(conn_fd);
    server_log(real_cmd);
    exit(0);
}


void do_cd(int conn_fd, char *real_cmd)
{
    char recv_buf[500];     //接收数据
    char chg_error[] = "切换目录出错";  //目录切换出错
    int cmd_len;                //命令长度
    char real_dir[40];          //当前目录
    char *cmd;                  //指向实际命令的参数

   //只有cd命令则改变目录到默认目录
    if(strlen(real_cmd) == strlen(CD))
    {
        if(chdir(SERV_DIR) == -1)
        {
            cmd_len = strlen(chg_error)+1;
            send(conn_fd, &cmd_len, sizeof(cmd_len), 0);
            send(conn_fd, chg_error, cmd_len, 0);
            //close(conn_fd);
            my_error("chdir", __LINE__);
        }

        else
        {
            strcpy(real_dir, SERV_DIR);
            cmd_len = strlen(real_dir)+1;
            send(conn_fd, &cmd_len, sizeof(cmd_len), 0);
            send(conn_fd, real_dir, cmd_len, 0);
        }
    }
    //否则切换到客户端命令的目录
    else
    {
        cmd = real_cmd + strlen(CD);
        if(*cmd == ' ')
            *cmd++;
        if(chdir(cmd) == -1)
        {
            cmd_len = strlen(chg_error)+1;
            send(conn_fd, &cmd_len, sizeof(cmd_len), 0);
            send(conn_fd, chg_error, cmd_len, 0);
            my_error("chdir", __LINE__);
        }
        else
        {
        strcpy(real_dir, cmd);
        cmd_len = strlen(real_dir)+1;
        send(conn_fd, &cmd_len, sizeof(cmd_len),0);
        send(conn_fd, real_dir, cmd_len, 0);
        }
    }

    bzero(recv_buf, sizeof(recv_buf));
    server_log(real_cmd);
    return;
}


void do_ls(int conn_fd, char *real_cmd)
{
 //支持-a, -l -la参数,其他参数一律视为ls处理
    int cmd_len;
    char argv_c[10][50];    //命令的具体内容,类似于main函数参数
    int count = 1;          //命令参数个数,类似于main函数argc
    int i = 0, j = 0;
    struct file_stat file;
    struct stat buf;
    char filename[100];
    //int ch;
    char *cmd;
    int aflag = 0;      //有-a参数
    int lflag = 0;      //有-l参数

    memset(filename, 0, 100);

    //解析所带参数
    cmd =real_cmd + strlen(LS) + 1;
    while(*cmd != '\0')
    {   //判断是否含有-a,-l.参数
        if(*cmd == 'a')
            aflag = 1;
        if(*cmd == 'l')
            lflag = 1;
        cmd++;
    }

    cmd = real_cmd;

    while(*cmd != '\0')
    {
        //将参数写入argv_c中
        while(*cmd != '\0' && *cmd != ' ')
        {
            argv_c[i][j] = *cmd;
            j++;
            cmd ++;
        }
        argv_c[i][j] = '\0';
        i++;
        cmd++;
        j = 0;
    }

    cmd = real_cmd;
    while(*cmd != '\0')
    {
        //参数个数
        if(*cmd == ' ')
            count++;
        cmd++;
    }

    //根据参数个数决定要查看的文件名
    if(count == 1)
    {
        strcpy(filename, ".");
    }
    else if(count == 2)
    {
        if(argv_c[1][0] == '-')
        {
            strcpy(filename, ".");
        }
        else
        {
            strcpy(filename, argv_c[1]);
        }
    }
    else if(count == 3)
    {
        if(argv_c[1][0] == '-')
        {
            strcpy(filename, argv_c[2]);
        }
        else
        {
            strcpy(filename, argv_c[1]);
        }
    }

    //获取文件属性
    if(lstat(filename, &buf) < 0)
    {
        my_error("stat", __LINE__);
    }

    //处理文件函数
        display_dir(aflag, lflag, conn_fd,filename);
        server_log(real_cmd);
        return;
}




void display_dir(int aflag, int lflag, int conn_fd ,char *filename)
{
    struct file_stat file[100];
    DIR *dir;
    struct dirent *ptr;
    char name[100];
    int i = 0;
    int flag;
    int count=0;

    bzero(file, sizeof(file));

    //打开目录
    if((dir = opendir(filename)) == NULL)
    {
        my_error("opendir",__LINE__);
    }

    //依次读取目录下文件

    if(lflag == 1 && aflag == 0)
    {
        flag = 1;
        //有-l参数
        while((ptr = readdir(dir)) != NULL)
        {
            if(ptr->d_name[0] == '.')
                continue;
        strcpy(name, filename);
        strcat(name,"/");
        strcat(name, ptr->d_name);
        //获取该文件具体属性
        display_file(aflag, lflag, name, ptr->d_name, &file[i]);
        i++;
        count++;
        }
        send(conn_fd, &flag, sizeof(flag), 0);
    }

    if(aflag == 1 && lflag == 0)
    {
        flag = 2;
            //有-a参数
        while((ptr = readdir(dir)) != NULL)
        {
            strcpy(name, filename);
            strcat(name,"/");
            strcat(name, ptr->d_name);

            display_file(aflag, lflag, name, ptr->d_name, &file[i]);
            i++;
            count++;
        }

        send(conn_fd, &flag, sizeof(flag), 0);
    }
    if(aflag == 0 && lflag == 0)
    {
        flag = 0;
            //ls
        while((ptr = readdir(dir)) != NULL)
        {
            if(ptr->d_name[0] == '.')
                continue;
            strcpy(name, filename);
            strcat(name,"/");
            strcat(name, ptr->d_name);
            display_file(aflag, lflag, name, ptr->d_name, &file[i]);
            i++;
            count++;
        }
        send(conn_fd, &flag, sizeof(flag), 0);
    }
    if(aflag ==1 && lflag ==1)
    {
        flag = 3;
        //-al参数
        while((ptr = readdir(dir)) != NULL)
        {
            strcpy(name, filename);
            strcat(name,"/");
            strcat(name, ptr->d_name);
            display_file(aflag, lflag, name, ptr->d_name, &file[i]);
            i++;
            count++;
        }
        send(conn_fd, &flag, sizeof(flag), 0);
    }

    //发送文件个数
    send(conn_fd, &count, sizeof(count), 0);
    i = 0;
    //a发送ls内容
    while(i < 100)
    {
        send(conn_fd, &file[i], sizeof(file[i]), 0);
        i++;
    }
}


void display_file(int aflag, int lflag,char *path, char *name, struct file_stat *file)
{
    struct passwd *psd;
    struct group *grp;
    struct stat buf;
    int i = 8;
    char filemode[10];

    if(lstat(path, &buf) < 0)
        my_error("stat", __LINE__);


    //判断文件属性
    if(S_ISLNK(buf.st_mode))
    {
        file->str[0] = 'l';
    }
    else if(S_ISREG(buf.st_mode))
    {
        file->str[0] = '-';
    }
    else if(S_ISDIR(buf.st_mode))
    {
        file->str[0] = 'd';
    }
    else if(S_ISCHR(buf.st_mode))
    {
        file->str[0] = 'c';
    }
    else if(S_ISBLK(buf.st_mode))
    {
        file->str[0] = 'b';
    }
    else if(S_ISFIFO(buf.st_mode))
    {
        file->str[0] = 'f';
    }
    else if(S_ISSOCK(buf.st_mode))
    {
        file->str[0] = 'a';
    }

    //获取文件所有者权限
    if(buf.st_mode & S_IRUSR)
    {
        file->str[1]= 'r';
    }
    else
    {
        file->str[1] = '-';
    }
    if(buf.st_mode & S_IWUSR)
    {
        file->str[2] = 'w';
    }
    else
    {

        file->str[2] = '-';
    }
    if(buf.st_mode & S_IXUSR)
    {
        file->str[3] = 'x';
    }
    else
    {
        file->str[3] = '-';
    }

    //同组用户其他人权限
    if(buf.st_mode & S_IRGRP)
    {
        file->str[4] = 'r';
    }
    else
    {
        file->str[4] = '-';
    }
    if(buf.st_mode & S_IWGRP)
    {
        file->str[5] = 'w';
    }
    else
    {
        file->str[5] = '-';
    }
    if(buf.st_mode & S_IXGRP)
    {
        file->str[6] = 'x';
    }
    else
    {
        file->str[6] = '-';
    }

    //其他人权限
    if(buf.st_mode & S_IROTH)
    {
        file->str[7] = 'r';
    }
    else
    {
        file->str[7] = '-';
    }
    if(buf.st_mode & S_IWOTH)
    {
        file->str[8] = 'w';
    }

    else
    {
        file->str[8] = '-';
    }
    if(buf.st_mode & S_IXOTH)
    {
        file->str[9] = 'x';
    }
    else
    {
        file->str[9] = '-';
    }

    //将文件属性写入自定义结构体中
        file->nlink = buf.st_nlink;
        strcpy(file->user,getpwuid(buf.st_uid)->pw_name);
        strcpy(file->group,getgrgid(buf.st_gid)->gr_name);
        file->size = buf.st_size;
        strcpy(file->time , ctime(&buf.st_mtime));
        file->time[strlen(file->time) - 1] = '\0';
        strcpy(file->file_name, name);
}



void do_get(int conn_fd, char *real_cmd)
{
    char *cmd;
    char buf[BUF_SIZE];
    struct stat file;
    char filename[100];
    DIR *dir;
    struct dirent *ptr;
    int flag = 0;
    int fd;
    struct flock lock;
    int ret;
    int count;

    bzero(&file, sizeof(file));
    bzero(filename, sizeof(filename));

    //解析文件名
    cmd =real_cmd + strlen(GET) + 1;
    strcpy(filename, cmd);

    //打开目录
    if((dir = opendir(".")) == NULL)
    {
        my_error("opendir", __LINE__);
    }

    //判断是否存在文件,并告知客户端
    while((ptr = readdir(dir)) != NULL)
    {
        if(strcmp(filename, ptr->d_name) == 0)
            flag = 1;
    }
    send(conn_fd, &flag, sizeof(flag), 0);

    //若文件存在
    if(flag == 1)
    {
        //向客户端发送文件名
        send(conn_fd, filename, sizeof(filename), 0);
        //以只读方式打开文件
        fd = open(filename, O_RDONLY);
        //对文件添加读锁
        lock.l_len = 0;
        lock.l_whence = 0;
        lock.l_type = F_RDLCK;
        lock.l_start = SEEK_SET;
        fcntl(fd, F_SETLK, lock);

        //获取文件属性
        stat(filename, &file);
        //发送文件
        count = file.st_size / BUF_SIZE + 1;
        send(conn_fd, &count, sizeof(count), 0);
        while(count--)
        {
            ret = read(fd, buf, BUF_SIZE);
            send(conn_fd, &ret, sizeof(ret), 0);
            send(conn_fd, &buf, ret, 0);
            bzero(buf, BUF_SIZE);
        }

        //解除文件锁
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, lock);
    }

        //清除缓冲区
        bzero(buf, BUF_SIZE);
        bzero(&file, sizeof(file));
        bzero(filename, sizeof(filename));
        server_log(real_cmd);
    close(fd);
    return;

}


void do_put(int conn_fd, char *real_cmd)
{
    struct flock lock;
    int flag = 0;
    int count;
    int ret;
    char buf[BUF_SIZE];
    char filename[100];
    int fd;

    //接受文件是否存在的标志
    recv(conn_fd, &flag, sizeof(flag), 0);

 //   printf("%d\n", flag);
    if(flag == 0)
    {
        return;
    }

    //文件存在
    else{

        chdir("/home/linky/FTP/server");
        recv(conn_fd, filename, sizeof(filename), 0);

        if((fd  = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) < 0)
        {
            my_error("open", __LINE__);
        }

        lock.l_len = 0;
        lock.l_whence = 0;
        lock.l_start = SEEK_SET;
        lock.l_type = F_WRLCK;
        fcntl(fd, F_SETLK, &lock);

        recv(conn_fd, &count, sizeof(count), 0);

        while(count--)
        {
            recv(conn_fd, &ret, sizeof(ret), 0);
            recv(conn_fd, &buf, ret, 0);
            write(fd, buf, ret);
            bzero(buf, BUF_SIZE);
        }
        lock.l_type = F_UNLCK;
        fcntl(fd, F_UNLCK, &lock);

    }
    close(fd);
    bzero(buf, BUF_SIZE);
    bzero(filename, sizeof(filename));
    return;
}
