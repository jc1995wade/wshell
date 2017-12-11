#include "common.h"
#include "dirent.h"
#include "pwd.h"
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#define F_COUNTER  100
/*
 *  如何判断是文件还是目录
 */
//指针交换
#define SWAP(x,y)  {char* tmp; tmp = x; x = y; y = tmp;}
#define INT_SWAP(x,y)  {unsigned int tmp; tmp = x; x = y; y = tmp;}

int  fileMode(char* name);


/*==========ls file1 file2 ... ===============*/
#if 1
void ls_files(int ar, char* str[])
{
	struct stat sf;
	int i;
	int f_mode;
	for(i = 1; i < ar; i++)
	{
		f_mode = fileMode(str[i]);
		switch(f_mode){
			case -1:
				continue;
				break;
			case 0:
				write(STDOUT_FILENO, str[i], strlen(str[i]));
				write(STDOUT_FILENO, "  ", 2);
				break;
			case 1:
				printf("\e[32m%s  \e[0m", str[i]);
				break;
			default:
				break;
		}

	}
	printf("\n");
}
#endif


void file_type(mode_t m)
{
	char buffer[] = {'p', 'c', 'd', 'b', '-', 'l', 's'};
	write(1, &buffer[(m >> 12 & 0xF)/2], 1);
}

void file_perm(mode_t m)
{
	char buffer[] = {'r', 'w', 'x'};
	int i = (m >> 6) & 0x7;
	int index = 0;
	for(index = 0; index < 9; index++)
	{
		if(m >> (8 - index) & 0x1 == 0x1){
			write(1, &buffer[index % 3], 1);
		}else{
			write(1 ,"-", 1);
		}
	}
}

void file_link(nlink_t t)
{
	char s;
	s = '0' + t;
	write(1, " ", 1);
	write(1, &s, 1);
	write(1, " ", 1);
}

void file_id(uid_t u, gid_t g)
{
	struct passwd* pd;
	pd = getpwuid(u);

	printf("%s ", pd->pw_name);
	pd = getpwuid(g);
	printf("%s ", pd->pw_name);
}


void file_size(off_t z)
{
	printf(" %4lu  ", z);
}


void file_time(time_t* str)
{

	char arr[20] = {'\0'};
	char* p = ctime(str);
	strncpy(arr, ctime(str)+4, 12);
	printf("%s", arr);
}


// 输出单个文件的　ls -l　信息
void print_l(char* str)
{
		struct stat sf;
		stat(str, &sf);
		file_type(sf.st_mode);
		file_perm(sf.st_mode);
		file_link(sf.st_nlink);
		file_id(sf.st_uid, sf.st_gid);
		file_size(sf.st_size);
		file_time(&sf.st_ctime);
		if(fileMode(str) == 1)
			printf("\e[32m %s\e[0m", str);
		else
			printf(" %s", str);	
		printf("\n");
}

//ls -l file1 file2 ...
void ls_l_file(int ar, char* str[])
{
	
	int i;
	int fd;
	for(i = 2; i < ar; i++)
	{
		fd = open(str[i], O_RDONLY);
		if(-1 == fd){
			fprintf(stderr, "ls: 无法访问%s:%s\n", str[i], strerror(errno));
			continue;
		}
		close(fd);
	}
	for(i = 2; i < ar; i++)
	{
		
		fd = open(str[i], O_RDONLY);
		if(-1 == fd){
			continue;
		}
		close(fd);
		print_l(str[i]);
	}
}

// ls -i file1 file2
void ls_i_file(int ar, char* str[])
{
	struct stat sf;
	
	int i;
	for(i  = 2; i < ar; i++)
	{
		stat(str[i], &sf);
		printf("%lu %s  ", sf.st_ino, str[i]);
	}
	printf("\n");
}

#if 1

//ls 下排序
void sort_file(int len, char* str[], unsigned long *ino)
{
	int i, j;
	int tp;
	for(i = 1; i < len; i++){
		tp = i-1;
		for(j = i; j < len; j++){
			if(strcmp(str[j], str[tp]) < 0){ // tp > str[i]
				SWAP(str[tp],str[j]);
				INT_SWAP(ino[tp],ino[j]);
			}
		}
	}
}
// 按时间从新到旧（大到小） 冒泡排序..待改
void sort_time(int len, char* str[], unsigned long *tm)
{
	int i, j;
	int tp;
	for(i = 1; i < len; i++){
		tp = i-1;
		for(j = i; j < len; j++){
			if( tm[j] > tm[tp]){
				INT_SWAP(tm[j], tm[tp]);
				SWAP(str[j], str[tp]);
			}
		}
	}
}

#endif

//判断是否为目录
int  fileMode(char* name)
{
	struct stat sf;
	int result;

	if(-1 == stat(name,&sf))        // 无效文件      -1
		result = -1;
	else if(S_ISREG(sf.st_mode)){   // 普通文件       0
		result = 0;
	}
	else if(S_ISDIR(sf.st_mode)){   // 目录文件       1
		result = 1;
	} 
	else if(S_ISCHR(sf.st_mode)){   // 字符特殊文件   2
		result = 2;
	}
	else if(S_ISBLK(sf.st_mode)){   // 块特殊文件     3
		result = 3;
	}
	else if(S_ISFIFO(sf.st_mode)){  // 管道或FIFO     4
		result = 4;
	}
	else if(S_ISLNK(sf.st_mode)){   // 符号链接       5
		result = 5;
	}
	else if(S_ISSOCK(sf.st_mode)){  // 套接字         6 
		result = 6;
	}
	return result;
}


// ls  / ls -a  /ls -s / ls -l
void ls_a_or_s(int con) 
{
	DIR	 *dp;
	struct dirent *dirp;
	struct stat sf;
	char *strp[F_COUNTER];
	unsigned int blocks[F_COUNTER];
	unsigned long time[F_COUNTER];
	unsigned long ino[F_COUNTER];
	mode_t mode[F_COUNTER];
	int max = 0;
	int i;

	if((dp = opendir((char*)"./")) == NULL){
		fprintf(stderr, "ls:%s\n", strerror(errno));
	}
		
	while((dirp = readdir(dp)) != NULL)
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")){
			continue;
		}

		stat(dirp->d_name, &sf);
		blocks[max] = sf.st_blocks/2;
		time[max] = sf.st_ctime;
		ino[max] = sf.st_ino;
		strp[max++] =  dirp->d_name;
	}
    /* 排序后输出　*/
	
	if(4 == con){   // ls -t
		//按时间排序
		sort_time(max, strp, time);
		for(i = 0; i < max; i++){
			if(fileMode(strp[i]) == 1)
				printf("\e[32m%s  \e[0m",strp[i]);
				//printf("dir:%s  ", strp[i]);
			else
				printf("%s  ", strp[i]);
		}
		printf("\n");
	}
	else{
	// 按名字排序
	sort_file(max, strp, ino);
	if(0 == con){   // ls 
		for(i = 0; i < max; i++){
			if(fileMode(strp[i]) == 1)
				printf("\e[32m%s  \e[0m", strp[i]);
			else
				printf("%s  ", strp[i]);
		}
		printf("\n");
	}
	else if(1 == con)  //ls -a
	{
		printf("\e[32m.  ..  \e[0m");
		for(i = 0; i < max; i++){
			if(fileMode(strp[i]) == 1)
				printf("\e[32m%s  \e[0m", strp[i]);
			else
				printf("%s  ",strp[i]);
		}	
		printf("\n");
	}
	else if(2 == con){  //ls -s
		for(i = 0; i < max; i++){
			if(fileMode(strp[i]) == 1)	
				printf("%u \e[32m%s  \e[0m", blocks[i], strp[i]);
			else
				printf("%u %s   ", blocks[i], strp[i]);
		}
		printf("\n");
	}
	else if(3 == con){ // ls -l
		for(i = 0; i < max; i++){
			print_l(strp[i]);
		}
	}
	else if(5 == con){  // ls -i
		for(i = 0; i < max; i++){
			if(fileMode(strp[i]) == 1)
				printf("%lu \e[32m%s  \e[0m",ino[i], strp[i]);
			else
				printf("%lu %s  ",ino[i], strp[i]);
		}
		printf("\n");
	}
	}
	closedir(dp);
}

//ls -s file1 ...
void ls_s_file(int ar, char* str[])
{	
	int i;
	int fd;
	struct stat sf;
	for(i = 2; i < ar; i++)
	{
		fd = open(str[i], O_RDONLY);
		if(-1 == fd){
			fprintf(stderr, "ls: 无法访问%s:%s\n", str[i], strerror(errno));
			continue;
		}
		close(fd);
	}
	for(i = 2; i < ar; i++)
	{
		
		fd = open(str[i], O_RDONLY);
		if(-1 == fd){
			continue;
		}
		
		stat(str[i], &sf);
		printf("%lu %s   ", sf.st_blocks/2, str[i]);
		close(fd);
	}
	printf("\n");
	
}

void ls_t_file(int ar, char* str[])
{
	int fd;
	struct stat sf;
	int i;
	int max = 0;

	char* strp[F_COUNTER];
	unsigned long time[F_COUNTER];
	

	for(i = 2; i < ar; i++){
		fd = open(str[i], O_RDONLY);
		if(-1 == fd){
			fprintf(stderr, "ls: 无法访问%s:%s\n", str[i], strerror(errno));
			continue;
		}
		close(fd);
	}
	for(i = 2; i < ar; i++)
	{
		fd = open(str[i], O_RDONLY);
		if(-1 == fd){
			continue;
		}
		stat(str[i], &sf);
		strp[max] =  str[i];
		time[max++] = sf.st_ctime;
	}
		
	/*------按时间排序--------*/
	sort_time(max, strp, time);
	
	for(i = 0; i < max; i++)
	{
		if(fileMode(strp[i]) == 1){
			printf("\e[32m%s  \e[0m", strp[i]);
		}
		else
			printf("%s  ", strp[i]);
	}
	printf("\n");
	
}

/*
 * 1 显示当前目录文件 
 * 2 从当前文件找目录
 * 3 如果找到目录进入目录
 * 4 递归
 */

void ls_R(char* str)
{
	char buffer[1024];
	DIR* dp;
	struct dirent *dirp;
	// 显示当前目录文件
	write(STDOUT_FILENO, str, strlen(str));
	write(STDOUT_FILENO, ":\n", 2);
	
	if((dp = opendir(str)) == NULL){
		fprintf(stderr, "ls:%s\n", strerror(errno));
		return ;
	}
	while((dirp = readdir(dp)) != NULL){
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")){
			continue;
		}
//		memset(buffer, 0, sizeof(buffer));
//		strcpy(buffer, str);
//		strcpy(buffer, "/");
//		strcpy(buffer, dirp->d_name);
//		if(fileMode(buffer) == 1){
//			printf("\e[32m%s  \e[0m", dirp->d_name);
//			fflush(NULL);
//		}
//		else{
			write(STDOUT_FILENO, dirp->d_name, strlen(dirp->d_name));
			write(STDOUT_FILENO, "  ", 2);
//		}
	}
	write(STDOUT_FILENO, "\n\n", 2);
	closedir(dp);
#if 1
	dp = NULL;
	dirp = NULL;
	// 从当前文件寻找目录
	if((dp = opendir(str)) == NULL){
		fprintf(stderr, "ls:%s\n", strerror(errno));
		return ;
	}
	while((dirp = readdir(dp)) != NULL){
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")){
			continue;
		}
	//　判断是否为目录
	
			memset(buffer, 0, sizeof(buffer));
			strcat(buffer, str);
			strcat(buffer,"/");
			strcat(buffer,dirp->d_name);
	//	printf("%s=%d\n", dirp->d_name, fileMode(dirp->d_name));
		
		if(fileMode(buffer) == 1){
		//	printf("path=%s\n",buffer);
			ls_R(buffer);
		}
	}
	closedir(dp);
#endif
}


int wls(int argc, char* argv[])
{
	switch(argc){
/*============$ ls  =====================*/
		case 1:
			ls_a_or_s(0); // $ ls
			break;
/*============$ ls -?  ==================*/
		case 2:         
			if(argv[1][0] == '-'){
				switch(argv[1][1]){
					case 'l':
						ls_a_or_s(3); // ls -l
						break;
					case 'a':
						ls_a_or_s(1); // ls -a
						break;
					case 'i':
						ls_a_or_s(5); // ls -i
						break;
					case 's':
						ls_a_or_s(2); // ls -s
						break;
					case 't':
						ls_a_or_s(4); // ls -t
						break;
					case 'R':
						ls_R(".");
						//...
						break;
					default:
						break;
				}
			}
			else{      // ls file file2 ...
				ls_files(argc, argv);	
			}
			break;

/*===========$ ls -? ...   ===================*/
		default:
			if(argv[1][0] == '-'){
				switch(argv[1][1]){
					case 'l':        
						ls_l_file(argc, argv); // ls -l file1 ...
						break;
					case 'i':
						ls_i_file(argc, argv); // ls -i file1 ... 
						break;
					case 's':
						ls_s_file(argc, argv); // ls -s file1 ...
						break;
					case 't':
						ls_t_file(argc, argv); // ls -t file1 ...
						break;
					default:
						break;
				}
			}
			else{
				ls_files(argc, argv);  // ls file1 file2 ...
			}
			break;
	}
	
	return 0;
}
