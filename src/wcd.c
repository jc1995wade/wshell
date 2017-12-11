#include "wcd.h"


void printDir(void);

void wcd(char *dir)
{
	static char buffer[1024] = "~\0";
	char tmp[1024];
	char *home;
	if(dir == NULL){  // cd
		//	
	}
	else if(!strcmp(dir, "~")){  // cd ~
		home = getenv("HOME");
		if(chdir(home) < 0){
			fprintf(stderr, "wcd:%s\n",strerror(errno));
			return ;
		}
		printDir();
	}
	else if(!strcmp(dir , "-")){   // cd -
		memset(tmp, 0, sizeof(tmp));
		getcwd(tmp, 1024);
		if(chdir(buffer) < 0){
			fprintf(stderr, "wcd:%s\n",strerror(errno));
			return ;
		}
		
		memset(buffer, 0, sizeof(buffer));
		strcpy(buffer, tmp);
	//	printDir();		

	}
	else{    // cd dir

		memset(buffer, 0, sizeof(buffer));
		getcwd(buffer, 1024);
		if(chdir(dir) < 0){
			fprintf(stderr, "wcd:%s\n",strerror(errno));
			return ;
		}
	//	printDir();
	}
}


#if 1
void printDir(void)
{
	char tmp[1024];
	memset(tmp, 0 , sizeof(tmp));
	getcwd(tmp, 1024);
	write(STDOUT_FILENO, tmp, strlen(tmp));
	write(STDOUT_FILENO, "\n", 1);

	
}

#endif
