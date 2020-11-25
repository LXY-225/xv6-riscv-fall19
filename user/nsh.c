#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXARGS 10
#define MAXLEN 100

int getcmd(char *buf, int nbuf);
void panic(char *s);
int fork1(void);
void setargs(char *s, char *argv[], int *argc);
void execPipe(char *argv[], int argc);
void runcmd(char *argv[], int argc);


//*****************START  from sh.c *******************
int
getcmd(char *buf, int nbuf)       // 从标准输入0获取至多nbuf个字符写到buf
{
    fprintf(2, "@ ");
    memset(buf, 0, nbuf);     
    gets(buf, nbuf);         
    if(buf[0] == 0) // EOF
        return -1;
    return 0;
}

char whitespace[] = " \t\r\n\v";

void
panic(char *s)                //  s : 标准错误输出
{
  fprintf(2, "%s\n", s);
  exit(-1);
}

int
fork1(void)                
{
    int pid;
    pid = fork();
    if(pid == -1)
        panic("fork");        // 添加错误输出
    return pid;
}

//*****************END  from sh.c ******************


// argv[i] 指向s中第i个字符串
// 修改s中每个字符串，以'\0'结尾
// 返回词数argc
void 
setargs(char *s, char *argv[], int *argc){     
    int i = 0;
    int j = 0;
    for(; s[j] != '\n' && s[j] != '\0'; j++){
        while(strchr(whitespace, s[j]))
            j++;
        argv[i++] = s+j;
        while(strchr(whitespace, s[j]) == 0)
            j++;
        s[j] = '\0';
    }
    argv[i] = 0;
    *argc = i;
}

void 
execPipe(char *argv[], int argc){
    int i;
    for(i = 0; i < argc; i++){
        if(!strcmp(argv[i], "|")){     // 找到第一个"|"
            argv[i] = 0;
            break;
        }
    }
    int fd[2];
    pipe(fd);
    if(fork() == 0){   // 子进程,执行"|"左边的命令,关闭标准输出
        close(1);
        dup(fd[1]);
        close(fd[1]);
        close(fd[0]);
        runcmd(argv, i);
    }else{             // 父进程,执行"|"右边的命令,关闭标准输入
        close(0);
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);
        runcmd(argv+i+1, argc-i-1);
    }
}

void 
runcmd(char *argv[], int argc){
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "|")){    // 存在"|"
            execPipe(argv, argc);
        }
    }
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "<")){    // 存在"<",执行输入重定向
            close(0);
            open(argv[i+1], O_RDONLY);
            argv[i] = 0;
        }
        if(!strcmp(argv[i], ">")){    // 存在">",执行输出重定向
            close(1);
            open(argv[i+1], O_CREATE|O_WRONLY);
            argv[i] = 0;
        }
    }
    exec(argv[0], argv);
}

int main(){
    static char buf[MAXLEN];
    int fd;                          
    
    while((fd = open("console", O_RDWR)) >= 0){       // 确保3个文件描述符均打开
        if(fd >= 3){
            close(fd);
            break;
        }
    }

    while(getcmd(buf, sizeof(buf)) >= 0){     
        if(fork1() == 0){        
            char *argv[MAXARGS];               // 定义指向命令中各单词的指针
            int argc = -1;     
        
            setargs(buf, argv, &argc);
            runcmd(argv, argc);
        }                              
        wait(0);                           
    }                         
    exit(0);
}