#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
int pipefd[2];
pid_t cpid1 = -1, cpid2 = -1; 
void handler_int(int signum) {
	if (SIGINT == signum || SIGALRM == signum) {
		kill(cpid1, SIGUSR1);
		kill(cpid2, SIGUSR2);
	}
}
void handler_usr1(int signum) {	
	if (SIGUSR1 == signum) {
		close(pipefd[1]); /* Close unused write end */
		printf("Child Process 1 is Killed by Parent!\n");
		exit(0);
	}
}
void handler_usr2(int signum) {	
	if (SIGUSR2 == signum) {
		close(pipefd[0]);
		printf("Child Process 2 is Killed by Parent!\n");
		exit(0);
	}
}

int main() {
	if (pipe(pipefd) == -1) {
		//Create a pipe
		perror("pipe error!");
		exit(EXIT_FAILURE);
	}
	signal(SIGINT, handler_int); 
	signal(SIGALRM, handler_int);
	alarm(10);
	cpid1 = fork();
	if (cpid1 == 0) {
		/* Child1  */
        
		int x = 1;
		char buf[100];
		signal(SIGINT, SIG_IGN);
		signal(SIGUSR1, handler_usr1);
		while (1) {
			sprintf(buf, "I send you %d times.", x);
			write(pipefd[1], buf, sizeof(buf));
			x++;
			sleep(1);  
        	       
		}
	} 
	cpid2 = fork();
	if (cpid2 == 0) {
		/* Child2  */
        
		int x = 1;
		char buf[100];
		signal(SIGINT, SIG_IGN);
		signal(SIGUSR2, handler_usr2);
		while (1) {
			read(pipefd[0], buf, sizeof(buf));
			printf("%s", buf);
			printf("\n");       
		}
       
	}
	int status1, status2;
	wait(&status1);    
	wait(&status2); /* Wait for child */
	close(pipefd[1]); /* Reader will see EOF */
	close(pipefd[0]); /* Close unused read end */
	printf("Parent Process is Killed!\n");
	exit(0);
}
