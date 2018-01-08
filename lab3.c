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

#define BUFNUM 10								//����������
#define BUFSIZE 1020							//��������С

typedef struct buffer{							//�������ṹ
	int size;									//��Ч�ֽ���
	char buf[BUFSIZE];							//���ݻ�����
}BUFFER;

sem_t *full,*empty;
pid_t pid1 = -1,pid2 = -1;
key_t shm_key = (key_t)14477;					//���빲���ڴ��õļ�ֵ

int main(int argc,char *const argv[]){
	if(argc != 3){
		printf("argc != 3\n");
		exit(-1);
	}
	int segment_id;
	char* shard_memory;
	if(-1 == (segment_id = shmget(shm_key, sizeof(sem_t)*2 + sizeof(BUFFER)*BUFNUM, IPC_CREAT|0666))){	//���빲���ڴ�
		printf("shmget error at lab3 : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (shard_memory = (char*)shmat(segment_id,0,0))){	//ӳ�乲���ڴ�
		printf("\n shmat error in lab3 : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;				//ӳ���źŵ��ڹ����ڴ���
	empty =(sem_t*)(shard_memory + sizeof(sem_t));
	sem_init(full,1,0);							//��ʼ���źŵ�����������Ŀ
	sem_init(empty,1,BUFNUM);					//�ջ�������Ŀ��
	pid1 = fork();
	if(pid1 == 0 ){
		char * argv1[] = {"./lab3_read.o",argv[1],NULL};	//���ļ�����Ĳ���
		if(-1 == execv("./lab3_read.o",argv1))	//�������ļ�����
			printf("error on execv lab3_read.o :%s\n",strerror(errno));
		exit(-1);
	}
	pid2 = fork();
	if(pid2 == 0){
		char * argv2[] = {"./lab3_write.o",argv[2],NULL};	//д�ļ�����Ĳ���
		if(-1 == execv("./lab3_write.o",argv2))	//����д�ļ�����
			printf("error on execv lab3_write.o :%s\n",strerror(errno));
		exit(-1);
	}
	int status1, status2;
	wait(&status1);								//�ȴ������ӽ��̽���
	wait(&status2);
	sem_destroy(full);							//ɾ���źŵƣ�
	sem_destroy(empty);
	shmctl(segment_id,IPC_RMID,0);				//�ͷŹ����ڴ�
	printf("file copy completed.\n");
	return 0;
}
