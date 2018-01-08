#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int pipefd[2];
pid_t cpid1 = -1, cpid2 = -1; 

void handler_int(int signum) {					//SIGINT和SIGALRM信号处理程序
	if (SIGINT == signum || SIGALRM == signum) {
		kill(cpid1, SIGUSR1);
		kill(cpid2, SIGUSR2);
	}
}

void handler_usr1(int signum) {
	if (SIGUSR1 == signum) {
		close(pipefd[1]);						//关闭写管道
		printf("Child Process 1 is Killed by Parent!\n");
		exit(0);
	}
}

void handler_usr2(int signum) {	
	if (SIGUSR2 == signum) {
		close(pipefd[0]);						//关闭读管道
		printf("Child Process 2 is Killed by Parent!\n");
		exit(0);
	}
}

int main() {
	if (pipe(pipefd) == -1) {					//Create a pipe
		perror("pipe error!");
		exit(EXIT_FAILURE);
	}
	signal(SIGINT, handler_int); 
	signal(SIGALRM, handler_int);
	alarm(10);
	cpid1 = fork();								//创立子进程1
	if (cpid1 == 0) {/* Child1  */
		int x = 1;
		char buf[100];
		signal(SIGINT, SIG_IGN);				//忽略SIGINT信号
		signal(SIGUSR1, handler_usr1);			//响应SIGUSR1信号
		while (1) {
			sprintf(buf, "I send you %d times.", x);
			write(pipefd[1], buf, sizeof(buf));	//向管道中写数据
			x++;
			sleep(1);							//等待1s
		}
	}
	cpid2 = fork();								//创立子进程2
	if (cpid2 == 0) {/* Child2  */
		char buf[100];
		signal(SIGINT, SIG_IGN);				//忽略SIGINT信号
		signal(SIGUSR2, handler_usr2);			//响应SIGUSR2信号
		while (1) {
			read(pipefd[0], buf, sizeof(buf));	//从管道中读取数据
			printf("%s\n", buf);
		}
	}
	int status1, status2;
	wait(&status1);								//等待子进程结束
	wait(&status2);
	close(pipefd[1]);							//关闭写管道
	close(pipefd[0]);							//关闭读管道
	printf("Parent Process is Killed!\n");
	exit(0);
}
