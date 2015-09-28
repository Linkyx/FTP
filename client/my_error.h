//自定义错误处理函数

#include<string.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<sys/stat.h>
#include<limits.h>
#include<fcntl.h>
#include<dirent.h>

#define BUF_SIZE 1024
#define CLI_DIR "/home/linky/下载"

char LS[] = "ls";
char GET[] = "get";
char PUT[] = "put";
char QUIT[] = "quit";
char CD[] = "cd";
char HELP[] = "help";
void my_error(const char *string, int line)
{
    fprintf(stderr, "line:%d: ", line);
    perror(string);
    return;
}
