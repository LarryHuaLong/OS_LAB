#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

//信号灯、线程句柄定义
sem_t* sem1,*sem2;
pid_t pid1 = -1,pid2 = -1;

int main() 	{
	int segment_id;
	char* shard_memory;
	segment_id = shmget(IPC_PRIVATE,sizeof(int)+sizeof(sem_t)*2,0600);
	shard_memory = (char*)shmat(segment_id,0,0);
	*((int*)shard_memory) = 0;
	sem1 = shard_memory+sizeof(int);
	sem2 = sem1 + sizeof(sem_t);
	sem_init(sem1,1,1);
	sem_init(sem2,1,0);//创建信号灯并赋初值；
	pid1 = fork();
	if(pid1 == 0 ){
		
		int* a = (int*)shard_memory;
		for (int i = 1;i<=100;i++){
			
			sem_wait(sem1);
			*a += i;
			sem_post(sem2);
		}
		exit(0);
	}
	pid2 = fork();
	if(pid2 == 0){
		
		int* a = (int*)shard_memory;
		for (int i = 1;i<=100;i++)  {
			
			sem_wait(sem2);
			printf("%d\n",*a);
			sem_post(sem1);
		}
		exit(0);
	}
	printf("parent");
	int status1, status2;
	wait(&status1);    
	wait(&status2); /* Wait for child */
	sem_destroy(sem1);//删除信号灯；
	sem_destroy(sem2);//删除信号灯；
	shmctl(segment_id,IPC_RMID,0);
	return 0;
}
