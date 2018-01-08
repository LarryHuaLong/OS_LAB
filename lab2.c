#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t sem1,sem2;								//�źŵ�����
pthread_t p1,p2;								//�߳̾������
int a = 0;										//��������

void *subp1() {									//�߳�1ִ�к�������
	a = 0;
	for (int i = 1;i<=100;i++){
		sem_wait(&sem1);						//P(sem1)
		a += i;									//���������ۼ�
		sem_post(&sem2);						//V(sem2)
	}	
	return;
}

void *subp2() {
	for (int i = 1;i<=100;i++)  {
		sem_wait(&sem2);						//P(sem2)
		printf("%d\n",a);						//��ӡ����������ֵ
		sem_post(&sem1);						//V(sem1)
	}
	return;
}

int main() 	{
	sem_init(&sem1,0,1);						//�����źŵƲ�����ֵ��
	sem_init(&sem2,0,0);
	pthread_create(&p1,NULL,subp1,NULL);		//���������߳�subp1��subp2;
	pthread_create(&p2,NULL,subp2,NULL);
	pthread_join(p1,NULL);						//�ȴ������߳����н�����
	pthread_join(p2,NULL);
	sem_destroy(&sem1);							//ɾ���źŵƣ�
	return 0;
}
