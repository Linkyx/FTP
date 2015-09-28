/*自定义错误处理函数*/
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<error.h>
#include<signal.h>
#include<sys/stat.h>
#include<dirent.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#include<limits.h>
#include<fcntl.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>


#define BUF_SIZE 1024
#define LISTENQ 10
#define SERV_DIR "/home/linky/FTP/server"

char LS[] = "ls";
char GET[] = "get";
char PUT[] = "put";
char QUIT[] = "quit";
char CD[] = "cd";

void my_error(const char *string, int line)
{
    fprintf(stderr, "line:%d: ", line);
    perror(string);
    return;
}
