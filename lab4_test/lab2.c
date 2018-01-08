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

#define BUFNUM 10
#define BUFSIZE 1020
typedef struct buffer{
	int size;
	char buf[BUFSIZE];
}BUFFER;

//信号灯、线程句柄定义
sem_t* full,*empty;
key_t shm_key = (key_t)14477;
int main(int argc,char *const argv[]) 	{
	printf("this is %s\n",argv[0]);
	int segment_id;
	char* shard_memory;
	if(-1 == (segment_id = shmget(shm_key,sizeof(sem_t)*2 + sizeof(BUFFER)*BUFNUM,IPC_CREAT|0666))){
		printf("shmget error at lab3_write : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (shard_memory = (char*)shmat(segment_id,0,0))){
		printf("\n shmat error in write : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;
	empty =(sem_t*)(shard_memory + sizeof(sem_t));
	BUFFER *bufs = (BUFFER*)(shard_memory + 2*sizeof(sem_t));
	
	int fd;
	if(-1 == (fd = open(argv[1],O_CREAT|O_RDWR,S_IRWXU|S_IRWXO|S_IRWXG))){
		printf("failed to open file:%s\n",strerror(errno));
		exit(-1);
	}
	printf("write file name %s\n",argv[1]);

	int buf_index = 0;
	BUFFER writebuf;
	sleep(1);
	while(1){
		buf_index = buf_index % 10;
		printf("getting\n");
		sem_wait(full);
		memcpy((void*)&writebuf,(void*)&bufs[buf_index],sizeof(BUFFER));
		sem_post(empty);
		int sizewrited = write(fd,writebuf.buf,writebuf.size);
		if(sizewrited<= 0)
			printf("write error %d:%s \n",errno,strerror(errno));
		printf("writed %d bytes from bufs[%d] \n",sizewrited,buf_index);
		if(writebuf.size < BUFSIZE)
			break;
		buf_index++;
	}
	printf("write completed.\n");
	if(-1 == close(fd))
		printf("writefile failed to close.\n");
	shmctl(segment_id,IPC_RMID,0);
	
	return 0;
}
