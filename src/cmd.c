/*********************************************************
 * @file       cmd.c           
 * @author     Dwade
 * @version    V1.0
 * @date       2017-9-21
 * @brief      This file provides command line analysize
 *********************************************************/
#include "common.h"
#include <setjmp.h>
#include <signal.h>
#include <termios.h>

#include "gfish.h"
#include "wls.h"
#include "wchmod.h"
#include "wcd.h"
#include "history.h"

void do_line(char* cmd);

jmp_buf    jmp_env;  //跳转标记
char * prompt = "wade>>"; //提示符 
extern char** environ;

//void wgetchars(char* buf);


/*============去除前后空格============*/
void spaceToken(char** cmd)
{
	char** start = cmd;
	char* end  = *start + strlen(*cmd);
	while(*(*start) == ' ') (*start)++;  //start移动到第一非空格字符
	cmd = start;
	while(*end-- == ' ');
	*(end + 1) = '\0';     //添加分隔
}
/*-----------未解析到命令-------------*/
void otherCmd(void)
{
	longjmp(jmp_env, 10); //长跳转到main函数
}


/***********************************************
 * FunctionName:     pwd(void);           
 * Description:      执行pwd命令
 * Input:            NULL
 * Output:           NULL
 ************************************************/
void pwd(void)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	getcwd(buffer, 1024);
	write(STDOUT_FILENO, buffer, strlen(buffer));
	write(STDOUT_FILENO, "\n", 1);	
}


/***********************************************
 * FunctionName:     wenv(void);           
 * Description:      显示环境表 environ 内容
 * Input:            NULL
 * Output:           NULL
 ************************************************/
void wenv(void)
{
	char** en = environ;
	while(*en){
		write(STDOUT_FILENO, *en++, strlen(*en));
		write(STDOUT_FILENO, "\n", 1);
	}
}


/***********************************************
 * FunctionName:     wecho(void);           
 * Description:      显示指定环境表内容
 * Input:            char** av  参数列表
 * Output:           NULL
 ************************************************/
void wecho(char** av)
{
	char** argv = av;
	char** en = environ;
	// echo $HOME  / echo $PATH  
	if(!strncmp(*(argv+1), "$", 1)){   // echo $_  ..
		if(!strncmp((*(argv+1))+1, "$", 1)){  // echo $$ ...
			pid_t pid;
			pid = getpid();
			printf("pid=%d  ", pid);
			fflush(NULL);
			argv++;
		}
		else{
			argv++;
			while(*en){
				if(!strncmp(*en, *argv+1, strlen(*argv)-1)){
					write(STDOUT_FILENO, *en, strlen(*en)-1);
					write(STDOUT_FILENO, " ",  1);
					break;
				}
				*en++;
			}
		}
	}
	// echo abc ...
	argv++;
	while(*argv){
		write(STDOUT_FILENO, *argv++, strlen(*argv));
		write(STDOUT_FILENO, " ", 1);
	}
	write(STDOUT_FILENO, "\n", 1);
}

// export 
// exprot var=xxx
void wexport(int ar, char** av)
{
	if(ar == 1){                // exprot
		char** en = environ;
		char* ch = "declare -x ";
		while(*en){
			write(STDOUT_FILENO, ch, strlen(ch));
			write(STDOUT_FILENO, *en++, strlen(*en));
			write(STDOUT_FILENO, "\n", 1);
		}
	}
	else{                      // export var=xxx
		putenv(av[1]);
	}
}

/***********************************************
 * FunctionName:     wclear(void);           
 * Description:      执行外部程序清屏命令 clear
 * Input:            char** av 参数列表
 * Output:           NULL
 * Other:            自己实现的clear命令
 ************************************************/
void wclear(char** av)
{
	pid_t pid;
	pid = fork();
	if(pid < 0){
		fprintf(stderr, "fork error:%s\n", strerror(errno));
		return ;
	}
	else if(pid == 0){
		if(execvp("extcmd/wclear", av)){
		fprintf(stderr, "exec:%s\n", strerror(errno));
			otherCmd();
		}
	}
	else{
		wait(0);
	}
}


void wmkdir(int ar, char** av)
{
	if(ar < 2){
		fprintf(stderr,"mkdir: 缺少操作数\n");
		return ;
	}
	int i;
	for(i = 1; i < ar; i++){
		if(mkdir(av[i], 0775) < 0){
			fprintf(stderr, "mkdir error:%s\n", strerror(errno));
			return ;
		}
	}
}

void wtouch(int ar, char** av)
{
	if(ar < 2){
		fprintf(stderr," touch: 缺少操作数\n");
		return ;
	}
	int i;
	int fd;
	for(i = 1; i < ar; i++){
		if((fd = open(av[i], O_RDONLY|O_CREAT, 0664)) < 0){
			fprintf(stderr, "touch error: %s\n",strerror(errno));
			return ;
		}
		close(fd);
	}
}

#if 0
void ccat(int ar, char** av)
{
	if(ar < 2){
		fprintf(stderr, "ccat error: %s\n", strerror(errno));
		return ;
	}
	int i; 
	int fd;
	char buffer[1024];
	for(i = 1; i < ar; i++)
	{
		if(!strcmp(av[i], "+")){
			if(++i >= ar){
				fprintf(stderr, "缺少操作符号\n");
				break;
			}
			int fd = open(av[i], O_RDONLY);
			if(fd < 0){
				fprintf(stderr, "open err: %s\n", strerror(errno));
				continue;
			}
			if(dup2(fd, STDOUT_FINENO) != 0){
				fprintf(stderr, "dup2 err",strerror(errno));
				return ;
			}
			close(fd);
		}
		else if(!strcmp(av[i], "-")){
			if(++i >= ar){
				fprintf(stderr, "缺少操作符号\n");
				break;
			}	
			int fd = open(av[++i], O_WRONLY|O_CREAT|_TRUNC, 0644);
			if(fd < 0){
				fprintf(stderr, "open err: %s\n", strerror(errno));
				continue;
			}
			if(dup2(fd, SOUTIN_FILENO) != 0){
				fprintf(stderr, "dup2 err: %s\n", strerror(errno));
				return ;
			}
			close(fd);
		}
	}
}

#endif

void wcat(int ar, char** av)
{
	if(ar < 2){
		fprintf(stderr,"wcat error: %s\n", strerror(errno));
		return ;
	}
	int i;
	int fd, fd_out, fd_in;
	char buffer[1024];
	int arc = 0;
	for(i = 1; i < ar; i++){
		if(!strcmp(av[i], "-") && ++i < ar){
			fd = open(av[i], O_WRONLY|O_CREAT|O_TRUNC, 0644);
			if(fd < 0){
				printf("open err\n");
				return ;
			}
			// 备份标准输出	
			fd_out = dup(STDOUT_FILENO);
			// 重定向标准出
			if(dup2(fd, STDOUT_FILENO) != STDOUT_FILENO){
				printf("dup2 err\n");
				break;
			}
			close(fd);
			arc = i;
			break;
		}
		else if(!strcmp(av[i], "+") && ++i < ar){
			fd = open(av[i], O_RDONLY);
			if(fd < 0){
				printf("open err\n");
				return ;
			}
			fd_in = dup(STDIN_FILENO);
			if(dup2(fd,	STDIN_FILENO) != STDIN_FILENO){
				printf("dup2 err\n");
				break;
			}
			close(fd);
		}
	}
#if 1
	for(i = 1; i < arc; i++){
		if((fd = open(av[i], O_RDONLY)) < 0){
			continue;
			fprintf(stderr, "open error: %s\n", strerror(errno));
		}
		else{
			while(read(fd, buffer, 1024) > 0);{
				write(STDOUT_FILENO, buffer, strlen(buffer));
				memset(buffer, 0, 1024);
			}
		}
	}
#endif
	// 恢复标准输出
	dup2(fd_out, STDOUT_FILENO);
	dup2(fd_in, STDIN_FILENO);
}


void whelp()
{
	int fd;
	if((fd = open("README.d", O_RDONLY)) < 0){
		fprintf(stderr,"can not find help file\n");
		return ;
	}
	char buffer[1024];
	while(read(fd, buffer, 1024) > 0){
		write(STDOUT_FILENO, buffer, strlen(buffer));
		memset(buffer, 0, 1024);
	}
}


/***********************************************
 * FunctionName:     exitCMD()       
 * Description:      执行外部程序命令
 * Input:            char** av 参数列表
 * Output:           NULL
 ************************************************/
void exitCMD(char** av)
{
	setpgid(getpid(), getpid());  // 设置父进程组
	pid_t pid;
	pid = fork();
	if(pid < 0){
		fprintf(stderr, "fork error:%s\n", strerror(errno));
		return ;	
	}
	else if(pid == 0){
		setpgid(getpid(), getpgid(getppid()));// 子进程和父进程设为同一组
		signal(SIGINT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		if(execvp(av[0], av)){
			fprintf(stderr, "exec:%s\n", strerror(errno));
		}
		exit(0);
	}
	else{
		setpgid(pid, getpgrp());   //
		wait(0);
	}
}


/*==============处理特殊字符==============*/
/*===  ; > < ' "  \  |   ====*/
// ; 
void do_line_colon(char* cmds)
{
	int ls = 0;
	char* lines[1024] = { NULL };
	char* splitptr = ";";
	lines[ls] = strtok(cmds, splitptr);
	if(lines[ls++] == NULL){
		fprintf(stderr,"无效命令\n");
		return ;
	}
	do{
		lines[ls] = strtok(NULL, splitptr);
	}while(lines[ls++]);
	ls--;
	int i;
	for(i = 0; i < ls; i++){
		do_line(lines[i]);
	}

}


                 

/*=============解析命令===============*/
void do_line(char* cmd)
{
#if 1
	char tmp[1024] = {'\0'};
	spaceToken(&cmd);  //去前后空格 传cmd首地址
	strcpy(tmp, cmd);
	char* av[1024] = {NULL};
	char* splitptr = " ";
	int ar = 0;
	av[ar] = strtok(cmd, splitptr);
	if(av[ar++] == NULL){
		fprintf(stderr, "无效命令\n");
		return;
	}
	do{
		av[ar] = strtok(NULL, splitptr);
	}while(av[ar++]);
	ar--;
#endif

	if(!strcmp(av[0], "exit")){  //退出命令
		exit(0);
	}
	else if(!strcmp(av[0], "pwd")){   // pwd
		pwd();
	}
	else if(!strcmp(av[0], "wcd")){   // wcd
		wcd(av[1]);
	}
	else if(!strcmp(av[0], "wls")){    // ls ...
		wls(ar, av);
	}
	else if(!strcmp(av[0], "clear")){  // clear
		wclear(av);
	}
	else if(!strcmp(av[0], "wenv")){  // env
		wenv();
	}
	else if(!strcmp(av[0], "echo")){  // echo  ...
		wecho(av);
	}
	else if(!strcmp(av[0], "export")){  // export ...
		wexport(ar, av);
	}
	else if(!strcmp(av[0], "wmkdir")){  // mkdir ...
		wmkdir(ar, av);
	}
	else if(!strcmp(av[0], "wtouch")){  // touch ...
		wtouch(ar, av);
	}
	else if(!strcmp(av[0], "wcat")){   // cat ...
		wcat(ar, av);
	}
	else if(!strcmp(av[0], "wchmod")){   // wchmod ...
		wchmod(ar, av);
	}
	else if(!strcmp(av[0], "help")){  // help ...
		whelp();
	}
	else if(!strcmp(av[0], "history")){ // history num
		mDisplayHistory(ar, av);	
	}
	else if(!strcmp(av[0], "gfish")){
		if(av[1] == NULL){
			fprintf(stderr, "缺少所需文件\n");
			otherCmd();
		}
		int len = strlen(av[1]);
		if(len < 4){
			fprintf(stderr,"请输入 \".def\" 文件!\n");
			otherCmd();
		}
		else if(!strncmp( av[1]+len-4, ".def", 4)){   // gfish file.def
			gfish(av[1]);
		}
		//otherCmd();
	}
	else{        // 待加其他命令
		exitCMD(av);
		otherCmd();
	}
	// 保存有效命令
	mSaveHistory(tmp);
//	printf("#%s\n", tmp);
}

// 命令提示符
void printCWD(void)
{
	char buf[1024];
	char* home;
	char* wd;
	memset(buf, 0, sizeof(buf));
	getcwd(buf, 1024);
	home = getenv("HOME");
	wd = buf + strlen(home);
	if(!strncmp(home, buf, strlen(home))){
		write(STDOUT_FILENO, "~", 1);
		write(STDOUT_FILENO, wd, strlen(wd));
		write(STDOUT_FILENO, ">> ", 3);
	}
	else{
		write(STDOUT_FILENO, buf, sizeof(buf));
		write(STDOUT_FILENO, ">> ", 3);
	}
}
#if 0 
//获取一个字符不回显
int wgetch()
{
	struct termios tm_old;
	struct termios tm;
	int ch;
	tcgetattr(0, &tm_old);
//	cfmakeraw(&tm);
	tm = tm_old;
	tm.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &tm);
	ch = getchar();
	tcsetattr(0, 0, &tm_old);

	return ch;
}


void wgetchars(char* buf)
{
	char* str;
	str = buf;
	while((*str = atoa(wgetch())) != '\n')
	{
		if(*str == '0')
		{
			exit(0);
		}
		
		write(STDOUT_FILENO, str, 1);
		if(atoi(str) == 65)
		{
			write(STDOUT_FILENO, "KEY_UP", 6);
		}
		else
		{
			write(STDOUT_FILENO, str, 1);

			str++;
			break;
		}
	}
	write(STDOUT_FILENO,"\n", 1);
			write(STDOUT_FILENO, "KEY_UP", 6);
}


#endif

int main(int argc, char* argv[])
{
//	signal(SIGINT, SIG_IGN);      // 父进程不处理信号
//	signal(SIGTSTP, SIG_IGN);
	initHistoryList(&Hist);

	char buffer[1024];
	//设置跳转点
	int len = setjmp(jmp_env);
	if(len < 0){
		fprintf(stderr, "flags error: %s\n", strerror(errno));
		return -1;
	}else if(0 == len){
		//
	}else{
	//	printf("setjmp:%d\n", len);
	}
	char* st;
	while(1){
		printCWD();
		memset(buffer, '\0', sizeof(buffer));
		read(STDIN_FILENO, buffer, sizeof(buffer)); // 获取字符
		//wgetchars(buffer);
		int l = strlen(buffer); 
		if(l <= 1){
			otherCmd();
		}
		buffer[l-1] = '\0'; //去回车

		do_line_colon(buffer);  // 执行　ls / cd / pwd ...

	}
	return 0;
}
