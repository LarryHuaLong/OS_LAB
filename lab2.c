#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t sem1,sem2;								//信号灯声明
pthread_t p1,p2;								//线程句柄声明
int a = 0;										//公共变量

void *subp1() {									//线程1执行函数定义
	a = 0;
	for (int i = 1;i<=100;i++){
		sem_wait(&sem1);						//P(sem1)
		a += i;									//公共变量累加
		sem_post(&sem2);						//V(sem2)
	}	
	return;
}

void *subp2() {
	for (int i = 1;i<=100;i++)  {
		sem_wait(&sem2);						//P(sem2)
		printf("%d\n",a);						//打印公共变量的值
		sem_post(&sem1);						//V(sem1)
	}
	return;
}

int main() 	{
	sem_init(&sem1,0,1);						//创建信号灯并赋初值；
	sem_init(&sem2,0,0);
	pthread_create(&p1,NULL,subp1,NULL);		//创建两个线程subp1、subp2;
	pthread_create(&p2,NULL,subp2,NULL);
	pthread_join(p1,NULL);						//等待两个线程运行结束；
	pthread_join(p2,NULL);
	sem_destroy(&sem1);							//删除信号灯；
	return 0;
}
