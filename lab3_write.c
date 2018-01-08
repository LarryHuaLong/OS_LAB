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
	int segment_id;
	char* shard_memory;
	if(-1 == (segment_id = shmget(shm_key, sizeof(sem_t)*2 + sizeof(BUFFER)*BUFNUM, IPC_CREAT|0666))){	//���빲���ڴ�
		printf("shmget error at lab3_write : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (shard_memory = (char*)shmat(segment_id,0,0))){	//ӳ�乲���ڴ�
		printf("\n shmat error in write : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;				//ӳ���źŵ��ڹ����ڴ���
	empty =(sem_t*)(shard_memory + sizeof(sem_t));
	BUFFER *bufs = (BUFFER*)(shard_memory + 2*sizeof(sem_t));//ӳ�仺�����ڹ����ڴ���
	int fd;
	if(-1 == (fd = open(argv[1],O_CREAT|O_RDWR,S_IRWXU|S_IRWXO|S_IRWXG))){
		printf("failed to open file:%s\n",strerror(errno));
		exit(-1);
	}
	int buf_index = 0;
	BUFFER writebuf;
	while(1){
		buf_index = buf_index % BUFNUM;			//���������������ݻ���������ѭ��
		sem_wait(full);
		memcpy((void*)&writebuf,(void*)&bufs[buf_index],sizeof(BUFFER));	//�ӻ�����ȡ����
		sem_post(empty);
		int sizewrited = write(fd,writebuf.buf,writebuf.size);	//���ļ���д����
		if(sizewrited < 0)
			printf("write error %d:%s \n",errno,strerror(errno));
		if(writebuf.size < BUFSIZE)				//����������һ����������ѭ��
			break;
		buf_index++;
	}
	printf("write completed.\n");
	close(fd);
	shmctl(segment_id,IPC_RMID,0);
	return 0;
}
