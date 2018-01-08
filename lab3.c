#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BUFNUM 10								//缓存区数量
#define BUFSIZE 1020							//缓存区大小

typedef struct buffer{							//缓存区结构
	int size;									//有效字节数
	char buf[BUFSIZE];							//数据缓存区
}BUFFER;

sem_t *full,*empty;
pid_t pid1 = -1,pid2 = -1;
key_t shm_key = (key_t)14477;					//申请共享内存用的键值

int main(int argc,char *const argv[]){
	if(argc != 3){
		printf("argc != 3\n");
		exit(-1);
	}
	int segment_id;
	char* shard_memory;
	if(-1 == (segment_id = shmget(shm_key, sizeof(sem_t)*2 + sizeof(BUFFER)*BUFNUM, IPC_CREAT|0666))){	//申请共享内存
		printf("shmget error at lab3 : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (shard_memory = (char*)shmat(segment_id,0,0))){	//映射共享内存
		printf("\n shmat error in lab3 : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;				//映射信号灯在共享内存中
	empty =(sem_t*)(shard_memory + sizeof(sem_t));
	sem_init(full,1,0);							//初始化信号灯满缓存区数目
	sem_init(empty,1,BUFNUM);					//空缓存区数目；
	pid1 = fork();
	if(pid1 == 0 ){
		char * argv1[] = {"./lab3_read.o",argv[1],NULL};	//读文件程序的参数
		if(-1 == execv("./lab3_read.o",argv1))	//启动读文件程序
			printf("error on execv lab3_read.o :%s\n",strerror(errno));
		exit(-1);
	}
	pid2 = fork();
	if(pid2 == 0){
		char * argv2[] = {"./lab3_write.o",argv[2],NULL};	//写文件程序的参数
		if(-1 == execv("./lab3_write.o",argv2))	//启动写文件程序
			printf("error on execv lab3_write.o :%s\n",strerror(errno));
		exit(-1);
	}
	int status1, status2;
	wait(&status1);								//等待两个子进程结束
	wait(&status2);
	sem_destroy(full);							//删除信号灯；
	sem_destroy(empty);
	shmctl(segment_id,IPC_RMID,0);				//释放共享内存
	printf("file copy completed.\n");
	return 0;
}
