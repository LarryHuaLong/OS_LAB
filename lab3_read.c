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
	printf("this is in %s\n",argv[0]);
	int segment_id;
	char* shard_memory;
	if(-1 == (segment_id = shmget(shm_key,sizeof(sem_t)*2 + sizeof(BUFFER)*BUFNUM,IPC_CREAT|0666))){
		printf("shmget error at lab3_read : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (shard_memory = (char*)shmat(segment_id,0,0))){
		printf("\n shmat error while read : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;
	empty =(sem_t*)(shard_memory + sizeof(sem_t));
	BUFFER *bufs = (BUFFER*)(shard_memory + 2*sizeof(sem_t));
	
	printf("read file name %s\n",argv[1]);
	int fd;
	if(-1 == (fd = open(argv[1],O_RDONLY))){
		printf("failed to open file %d:%s\n",errno,strerror(errno));
		exit(-1);
	}
	int buf_index = 0;
	BUFFER readbuf;
	
	while(1){
		readbuf.size = read(fd,readbuf.buf,BUFSIZE);
		if(readbuf.size <= 0)
			break;
		buf_index = buf_index % BUFNUM;
		printf("posting\n");
		sem_wait(empty);
		printf("posting %d bytes to bufs[%d]\n",readbuf.size,buf_index);
		memcpy((void*)&bufs[buf_index],(void*)&readbuf,sizeof(BUFFER));
		sem_post(full);
		buf_index++;
	}
	printf("read completed.\n");
	close(fd);
	shmctl(segment_id,IPC_RMID,0);
	return 0;
}
