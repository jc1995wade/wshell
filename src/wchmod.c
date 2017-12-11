#include "common.h"

//用户读写执行
#define  U_ALL  S_IRWXU
#define  UR     S_IRUSR  
#define  UW     S_IWUSR
#define  UX     S_IXUSR

#define  GR     S_IRGRP
#define  GW     S_IWGRP
#define  GX     S_IXGRP

#define  OR     S_IROTH
#define  OW     S_IWOTH
#define  OX     S_IXOTH

#define  VTX    S_ISVTX  //保存正文(粘着位)
// con 0  wchmod a__ file | con=1 wchmod +/-_  file 
void switch_a(int argc, char* argv[]);


/*
 chmod 0xxx file
 chmod 644 file
 chmod 644 file1 file2 ...

 chmod u+s file
 chmod u+r file
 chmod u+w file
 chmod u+x file


 chmod g+s file
 chmod g+r file
 chmod g+w file
 chmod g+x file

 chmod o+s file
 ...

 chmod +s file
 chmod +r file
 chmod +w file
 chmod +x file
 
 */
/*==========改变所有用户权限===============*/
void chmod_a_rwxs(int ar, char* argv[] , int con)
{
	struct stat statbuf;
	int i;
	unsigned int BIT;
	for(i = 2; i < ar; i++ )
	{
	//	memset(statbuf, 0, strlen(statbuf));
		if(stat(argv[i], &statbuf) < 0){
			fprintf(stderr, "open error :%s\n", strerror(errno));
		}
		//修改权限  con 0 a+r  / 1 a+w  / 2 a+x
	    
		switch(con){
			case 0:
				BIT = (UR | GR | OR);  // a+r
				break;
			case 1:
				BIT = (UW | GW | OW);  // a+w
				break;
			case 2:
				BIT = (UX | GX | OX);  // a+x
				break;
			case 3:
				BIT = VTX;   // a+s
				break;
			case 4:
				BIT = ~(UR | GR | OR);  // a-r
				break;
			case 5:
				BIT = ~(UW | GW | OW);  // a-w
				break;
			case 6:
				BIT = ~(UX | GX | OW);  // a-x
				break;
			case 7:
				BIT = ~(VTX); // a-s
			default:
				printf("wchmod:无效模式\n");
				break;
		}
		if(con < 4)
		{
			if(chmod(argv[i], (statbuf.st_mode | BIT ))){
				fprintf(stderr, "open error :%s\n", strerror(errno));
			}
		}
		else{
			if(chmod(argv[i], (statbuf.st_mode & BIT ))){
				fprintf(stderr, "open error :%s\n", strerror(errno));
			}
		}
	}
} 





void wchmod(int argc, char* argv[])
{
	switch(argc){
		case 1:  // wchmod 
			fprintf(stderr, "wchmod:缺少需所要的操作数\n");
			break;
		case 2:  // wchmod 777  / wchmod a 
			fprintf(stderr, "wchmod:\"%s\" 后缺少操作数\n", argv[1]);
			break;
		default:
			if(argv[1][0] >= '0' && argv[1][0] <= '7')
			{
				// wchmod 0xxx file  
				// wchmod 777  file
			}
			else{
				switch(argv[1][0]){ // wchmod _xx file  
					case 'a':  // wchmod a_x file
						switch_a(argc,argv);
						break;
					case 'u':
						break;
					case 'g':
						break;
					case 'o':
						break;
					case '+':
						break;
					case '-':
						break;
					default:
						fprintf(stderr, "wchmod: 无效模式: \"%s\"\n", argv[1]);
						break;
				}
			}
		break;
	}
}


/*============wchmod a?? file ...  ===================*/
// con 0  wchmod a__ file | con=1 wchmod +/-_  file 
void switch_a(int argc, char* argv[])
{

	if(argv[1][1] == '+'){
	// a+_
		switch(argv[1][2]){
			case 'r':  // a+r
				chmod_a_rwxs(argc, argv, 0);
				break;
			case 'w':  // a+w
				chmod_a_rwxs(argc, argv, 1);
				break;
			case 'x':  // a+x
				chmod_a_rwxs(argc, argv, 2);
				break;
			case 's': // a+s  还不清楚
				chmod_a_rwxs(argc, argv, 3);
				break;
			default:
				fprintf(stderr, "wchmod: 无效模式: \"%s\"\n", argv[1]);	
				break;
		}
	}
	else if(argv[1][1] == '-'){
							// a-_
		switch(argv[1][2]){
			case 'r':  // a+r
				chmod_a_rwxs(argc, argv, 4);
				break;
			case 'w':  // a+w
				chmod_a_rwxs(argc, argv, 5);
				break;
			case 'x':  // a+x
				chmod_a_rwxs(argc, argv, 6);
				break;
			case 's': // a+s  还不清楚
				chmod_a_rwxs(argc, argv, 7);
				break;
			default:
				fprintf(stderr, "wchmod: 无效模式: \"%s\"\n", argv[1]);	
				break;
			
		}
	}
	else{
		fprintf(stderr, "wchmod: 无效模式: \"%s\"\n", argv[1]);	
		exit(0);
	}
	
}
