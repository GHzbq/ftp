// 1. 先包含标准库的文件
// 2. 再包含系统的文件
// 3. 再包含第三方库的文件
// 4. 再包含项目中的其他头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/wait.h>

#define IN 1
#define OUT 0

void ParseArg(char input[], char * output[]);
int Run(char *argv[]);

int main()
{
    char buf[1024] = {0};
    while(1)
    {
        // 1. 读取用户输入的指令
        printf("[zbq@localhost]$ ");
        fflush(stdout);
        scanf("%[^\n]%*c", buf); // %[] 表示读一个字符串 按[]里规定的内容读 ^\n 表示不是换行符的东西我都读
                                 // %*c 表示读一个字符并把它丢弃
        // 2. 解析用户输入的指令
        // 假设用户输入的是 ls -l .
        // 解析之后的结果期望是一个字符串数组，数组元素为：ls, -l, .
        char * argv[100] = {0};
        ParseArg(buf, argv);
        // 3. 创建子进程
        //    子进程进行程序替换
        //    父进程进行进程等待
        Run(argv);
        
    }

    return 0;
}

void ParseArg(char input[], char * output[])
{
    int argc = 0;
    int flag = OUT;
    int i = 0;
    for(i = 0; input[i] != '\0'; ++i)
    {
        if(!isspace(input[i]) && flag == OUT)
        {
            flag = IN;
            output[argc++] = &input[i];
        }
        else if(isspace(input[i]))
        {
            flag = OUT;
            input[i] = '\0';
        }
    }
    output[argc] = NULL;
}

int Run(char *argv[])
{
    pid_t pid = fork();
    if(pid == 0)
    {

        execvp(argv[0], argv);
        printf("command %s not found\n", argv[0]);
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    return 0;
}
