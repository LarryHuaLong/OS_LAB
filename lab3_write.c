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
	int segment_id;
	char* shard_memory;
	if(-1 == (segment_id = shmget(shm_key, sizeof(sem_t)*2 + sizeof(BUFFER)*BUFNUM, IPC_CREAT|0666))){	//申请共享内存
		printf("shmget error at lab3_write : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (shard_memory = (char*)shmat(segment_id,0,0))){	//映射共享内存
		printf("\n shmat error in write : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;				//映射信号灯在共享内存中
	empty =(sem_t*)(shard_memory + sizeof(sem_t));
	BUFFER *bufs = (BUFFER*)(shard_memory + 2*sizeof(sem_t));//映射缓存区在共享内存中
	int fd;
	if(-1 == (fd = open(argv[1],O_CREAT|O_RDWR,S_IRWXU|S_IRWXO|S_IRWXG))){
		printf("failed to open file:%s\n",strerror(errno));
		exit(-1);
	}
	int buf_index = 0;
	BUFFER writebuf;
	while(1){
		buf_index = buf_index % BUFNUM;			//缓存区索引，根据缓存区数量循环
		sem_wait(full);
		memcpy((void*)&writebuf,(void*)&bufs[buf_index],sizeof(BUFFER));	//从缓存区取数据
		sem_post(empty);
		int sizewrited = write(fd,writebuf.buf,writebuf.size);	//向文件中写数据
		if(sizewrited < 0)
			printf("write error %d:%s \n",errno,strerror(errno));
		if(writebuf.size < BUFSIZE)				//如果读到最后一块数据跳出循环
			break;
		buf_index++;
	}
	printf("write completed.\n");
	close(fd);
	shmctl(segment_id,IPC_RMID,0);
	return 0;
}
