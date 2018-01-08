#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t* sem1,*sem2;								//�źŵ�����
pid_t pid1 = -1,pid2 = -1;						//���̾������

int main(){
	int segment_id;
	char* shard_memory;
	segment_id = shmget(IPC_PRIVATE,sizeof(int)+sizeof(sem_t)*2,0600);	//���빲���ڴ�
	shard_memory = (char*)shmat(segment_id,0,0);//ӳ�乲���ڴ�
	*((int*)shard_memory) = 0;					//��ʼ���������ֵ
	sem1 = shard_memory+sizeof(int);			//ӳ���źŵƵ��ڴ��ַ
	sem2 = sem1 + sizeof(sem_t);
	sem_init(sem1,1,1);							//�����źŵƲ�����ֵ��
	sem_init(sem2,1,0);
	pid1 = fork();								//�������̣���ͬ
	if(pid1 == 0 ){
		int* a = (int*)shard_memory;			//��aָ�򵽹�����������ͬ
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
	wait(&status1);								//�ȴ��������̽���
	wait(&status2);
	sem_destroy(sem1);							//ɾ���źŵƣ�
	sem_destroy(sem2);
	shmctl(segment_id,IPC_RMID,0);				//�ͷ�ɾ�������ڴ�
	return 0;
}
