#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

//信号灯、线程句柄定义
sem_t sem1,sem2;
pthread_t p1,p2;
int a = 0;
//线程执行函数定义：void *subp1();void *subp2();
void *subp1() {
	a = 0;
	for (int i = 1;i<=100;i++){
		sem_wait(&sem1);
		a += i;
		sem_post(&sem2);
	}	
	return;
}
void *subp2() {
	for (int i = 1;i<=100;i++)  {
		sem_wait(&sem2);
		printf("%d\n",a);
		sem_post(&sem1);
	}
	return;
}
int main() 	{
	sem_init(&sem1,0,1);
	sem_init(&sem2,0,0);//创建信号灯并赋初值；
	pthread_create(&p1,NULL,subp1,NULL);
	pthread_create(&p2,NULL,subp2,NULL);//创建两个线程subp1、subp2;
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);//等待两个线程运行结束；
	sem_destroy(&sem1);//删除信号灯；
	return 0;
}
