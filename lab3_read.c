#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
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

sem_t* full,*empty;
key_t shm_key = (key_t)14477;					//���빲���ڴ��õļ�ֵ

int main(int argc,char *const argv[]){
	printf("this is in %s\n",argv[0]);
	int segment_id;
	char* shard_memory;
	if(-1 == (segment_id = shmget(shm_key, sizeof(sem_t)*2 + sizeof(BUFFER)*BUFNUM, IPC_CREAT|0666))){	//���빲���ڴ�
		printf("shmget error at lab3_read : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (shard_memory = (char*)shmat(segment_id,0,0))){	//ӳ�乲���ڴ�
		printf("\n shmat error while read : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;				//ӳ���źŵ��ڹ����ڴ���
	empty =(sem_t*)(shard_memory + sizeof(sem_t));
	BUFFER *bufs = (BUFFER*)(shard_memory + 2*sizeof(sem_t));//ӳ�仺�����ڹ����ڴ���
	int fd;
	if(-1 == (fd = open(argv[1],O_RDONLY))){
		printf("failed to open file %d:%s\n",errno,strerror(errno));
		exit(-1);
	}
	int buf_index = 0;
	BUFFER readbuf;
	while(1){
		int sizewrited = readbuf.size = read(fd,readbuf.buf,BUFSIZE);	//���ļ��ж�����
		if(sizewrited<= 0)
			printf("read error %d:%s \n",errno,strerror(errno));
		if(readbuf.size <= 0)					//����������һ����������ѭ��
			break;
		buf_index = buf_index % BUFNUM;			//���������������ݻ���������ѭ��
		sem_wait(empty);
		memcpy((void*)&bufs[buf_index],(void*)&readbuf,sizeof(BUFFER));	//�򻺴���������
		sem_post(full);
		buf_index++;
	}
	printf("read completed.\n");
	close(fd);
	shmctl(segment_id,IPC_RMID,0);
	return 0;
}
