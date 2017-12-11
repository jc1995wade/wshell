#include "common.h"


jmp_buf env;

void takeSpace(char** cmd)
{
	char** start = cmd;
	char* end = *(start + strlen(*cmd));
	while(**start == ' ') (*start)++;
	cmd = start;
	while(*end == ' ') --end;
	*(end+1) = '\0';
}


void cmd_ls_exec()
{
	pid_t pid = fork();
	if(pid < 0){
	}
	else if(pid == 0){
		// start load new program's code
		if(execl("/bin/ls", "/bin/ls", (char*)0)){
			fprintf(stderr, "exec:%s\n", strerror(errno));
			longjmp(env, 1);
		}
	}
	else{
		wait(0);
	}
}

void do_line(char * cmd)
{

   // takeSpace(&cmd);
	if(!strcmp(cmd, "exit")){
		exit(0);
	}
	else if(!strcmp(cmd ,"ls")){
		cmd_ls_exec();
	}
}


int main(int argc, char* argv[])
{
	const char* prompt = ">>";
	
	char buffer[1024];
	int ret = setjmp(env);
	if(ret > 0){
		// invoked exception function
	}
	while(1){
		write(STDOUT_FILENO, prompt, strlen(prompt));
		memset(buffer, 0, sizeof(buffer));
		int l = read(STDOUT_FILENO, buffer, sizeof(buffer));
		buffer[l-1] = '\0';
		do_line(buffer);
	
	}
	return 0;
}
