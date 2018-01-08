#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t* sem1,*sem2;								//信号灯声明
pid_t pid1 = -1,pid2 = -1;						//进程句柄声明

int main(){
	int segment_id;
	char* shard_memory;
	segment_id = shmget(IPC_PRIVATE,sizeof(int)+sizeof(sem_t)*2,0600);	//申请共享内存
	shard_memory = (char*)shmat(segment_id,0,0);//映射共享内存
	*((int*)shard_memory) = 0;					//初始化共享变量值
	sem1 = shard_memory+sizeof(int);			//映射信号灯的内存地址
	sem2 = sem1 + sizeof(sem_t);
	sem_init(sem1,1,1);							//创建信号灯并赋初值；
	sem_init(sem2,1,0);
	pid1 = fork();								//创建进程，下同
	if(pid1 == 0 ){
		int* a = (int*)shard_memory;			//将a指向到公共变量，下同
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
		for (int i = 1;i<=100;i++){
			sem_wait(sem2);
			printf("%d\n",*a);
			sem_post(sem1);
		}
		exit(0);
	}
	printf("parent");
	int status1, status2;
	wait(&status1);								//等待两个进程结束
	wait(&status2);
	sem_destroy(sem1);							//删除信号灯；
	sem_destroy(sem2);
	shmctl(segment_id,IPC_RMID,0);				//释放删除共享内存
	return 0;
}
