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

#define BUFNUM 10								//缓存区数量
#define BUFSIZE 1020							//缓存区大小

typedef struct buffer{							//缓存区结构
	int size;									//有效字节数
	char buf[BUFSIZE];							//数据缓存区
}BUFFER;

sem_t* full,*empty;
key_t shm_key = (key_t)14477;					//申请共享内存用的键值

int main(int argc,char *const argv[]){
	printf("this is in %s\n",argv[0]);
	int segment_id;
	char* shard_memory;
	if(-1 == (segment_id = shmget(shm_key, sizeof(sem_t)*2 + sizeof(BUFFER)*BUFNUM, IPC_CREAT|0666))){	//申请共享内存
		printf("shmget error at lab3_read : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (shard_memory = (char*)shmat(segment_id,0,0))){	//映射共享内存
		printf("\n shmat error while read : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;				//映射信号灯在共享内存中
	empty =(sem_t*)(shard_memory + sizeof(sem_t));
	BUFFER *bufs = (BUFFER*)(shard_memory + 2*sizeof(sem_t));//映射缓存区在共享内存中
	int fd;
	if(-1 == (fd = open(argv[1],O_RDONLY))){
		printf("failed to open file %d:%s\n",errno,strerror(errno));
		exit(-1);
	}
	int buf_index = 0;
	BUFFER readbuf;
	while(1){
		int sizewrited = readbuf.size = read(fd,readbuf.buf,BUFSIZE);	//从文件中读数据
		if(sizewrited<= 0)
			printf("read error %d:%s \n",errno,strerror(errno));
		if(readbuf.size <= 0)					//如果读到最后一块数据跳出循环
			break;
		buf_index = buf_index % BUFNUM;			//缓存区索引，根据缓存区数量循环
		sem_wait(empty);
		memcpy((void*)&bufs[buf_index],(void*)&readbuf,sizeof(BUFFER));	//向缓存区存数据
		sem_post(full);
		buf_index++;
	}
	printf("read completed.\n");
	close(fd);
	shmctl(segment_id,IPC_RMID,0);
	return 0;
}
