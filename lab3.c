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

#define BUFNUM 10
#define BUFSIZE 1020
typedef struct buffer{
	int size;
	char buf[BUFSIZE];
}BUFFER;

//信号灯、线程句柄定义
sem_t *full,*empty;
pid_t pid1 = -1,pid2 = -1;
key_t shm_key = (key_t)14477;
int main(int argc,char *const argv[]) 	{
	printf("this is parent\n");
	if(argc != 3){
		printf("argc != 3\n");
		exit(-1);
	}
	int segment_id;
	char* shard_memory;
	if(-1 == (segment_id = shmget(shm_key,sizeof(sem_t)*2 + sizeof(BUFFER)*BUFNUM,IPC_CREAT|0666))){
		printf("shmget error at lab3 : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (shard_memory = (char*)shmat(segment_id,0,0))){
		printf("\n shmat error in lab3 : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;
	empty =(sem_t*)(shard_memory + sizeof(sem_t));
	
	sem_init(full,1,0);//满缓存区数目
	sem_init(empty,1,BUFNUM);//空缓存区数目；
	
	pid1 = fork();
	if(pid1 == 0 ){
		//printf("in p1\n");
		char * argv1[] = {"./lab3_read.o",argv[1],NULL};
		if(-1 == execv("./lab3_read.o",argv1))
			printf("error on execv lab3_read.o :%s\n",strerror(errno));
		exit(-1);
	}
	pid2 = fork();
	if(pid2 == 0){
		//printf("in p2\n");
		char * argv2[] = {"./lab3_write.o",argv[2],NULL};
		if(-1 == execv("./lab3_write.o",argv2))
			printf("error on execv lab3_write.o :%s\n",strerror(errno));
		exit(-1);
	}
	int status1, status2;
	wait(&status1);    
	wait(&status2); /* Wait for child */
	sem_destroy(full);//删除信号灯；
	sem_destroy(empty);//删除信号灯；
	shmctl(segment_id,IPC_RMID,0);
	printf("file copy completed.\n");
	return 0;
}
