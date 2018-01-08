#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>

char file_type(mode_t st_mode)					//�ж��ļ����ͣ������ش�����ļ����͵���ĸ
{
	switch(st_mode & S_IFMT){
		case S_IFSOCK:return 's';
		case S_IFLNK:return 'l';
		case S_IFREG:return '-';
		case S_IFBLK:return 'b';
		case S_IFDIR:return 'd';
		case S_IFCHR:return 'c';
		case S_IFIFO:return 'P';
		default: return'?';
	}
}
int print_perm(mode_t st_mode)					//��ӡȨ����Ϣ
{
	putchar(st_mode & S_IRUSR ? 'r':'-');		//owner��Ȩ��
	putchar(st_mode & S_IWUSR ? 'w':'-');		//ownerдȨ��
	putchar(st_mode & S_IXUSR ? (st_mode & S_ISUID ? 's':'x') :(st_mode & S_ISUID ? 'S':'-'));//ownerִ��Ȩ��
	putchar(st_mode & S_IRGRP ? 'r':'-');		//group��Ȩ��
	putchar(st_mode & S_IWGRP ? 'w':'-');		//groupдȨ��
	putchar(st_mode & S_IXGRP ? (st_mode & S_ISGID ? 's':'x') :(st_mode & S_ISGID ? 'S':'-'));//groupִ��Ȩ��
	putchar(st_mode & S_IROTH ? 'r':'-');		//others��Ȩ��
	putchar(st_mode & S_IWOTH ? 'w':'-');		//othersдȨ��
	putchar(st_mode & S_IXOTH ? (st_mode & S_ISVTX ? 't':'x') :(st_mode & S_ISVTX ? 'T':'-'));//othersִ��Ȩ��
	return 0;
}
void printdir(char *dirpath,char *dirname)
{
	static char *months[] = {"Jan", "Feb", "Mar",
							"Apr", "May", "Jun",
							"Jul", "Aug", "Sep",
							"Oct", "Nov", "Dec"};
	DIR *p_dir;
	struct dirent *p_dirent;
	struct stat statbuf;
	struct passwd *p_passwd;
	struct group *p_group;
	struct tm *p_time;
	int pathlen = strlen(dirpath)+strlen(dirname)+2;
	char *path = (char*)malloc(pathlen);
	if(strlen(dirpath))
		sprintf(path,"%s/%s",dirpath,dirname);
	else
		sprintf(path,"%s",dirname);
	printf("%s:\n",path);						//��ʾ��ǰ������·����
	if(NULL == (p_dir = opendir(dirname))){		//���ļ�Ŀ¼
		perror("opendir");
		return;
	}
	if(-1 == chdir(dirname)){					//��dirname����Ϊ��ǰĿ¼
		perror("chdir");
		return;
	}
	int maxsize = 0;
	int total_512B_blocks = 0;
	while(NULL != (p_dirent = readdir(p_dir))){	//��ȡÿ��Ŀ¼��������
		if('.' == p_dirent->d_name[0])
			continue;							//���� "."��".."�������ļ�
		if(-1 == lstat(p_dirent->d_name,&statbuf)){	//��ȡ��ǰĿ¼�������
			perror(p_dirent->d_name);
			return;
		}
		if(statbuf.st_size > maxsize)
			maxsize = statbuf.st_size;
		total_512B_blocks += statbuf.st_blocks;
	}
	printf("total %d\n",total_512B_blocks / 2);	//��ʾ��ǰĿ¼�������ܿ���
	int maxsizelen = 1;
	while(maxsize /= 10)
		maxsizelen++;
	rewinddir(p_dir);							//Ŀ¼����λ����ʼ
	while(NULL != (p_dirent = readdir(p_dir))){	//��ȡÿ��Ŀ¼��������
		if('.' == p_dirent->d_name[0])
			continue;							//���� "."��".."�������ļ�
		if(-1 == lstat(p_dirent->d_name,&statbuf)){	//��ȡ��ǰĿ¼�������
			perror(p_dirent->d_name);
			return;
		}
		p_time = localtime(&statbuf.st_mtime);	//ת��ʱ���ʽ
		p_passwd = getpwuid(statbuf.st_uid);	//��ȡ�û���
		p_group = getgrgid(statbuf.st_gid);		//��ȡ�û�����
		putchar(file_type(statbuf.st_mode));	//����ļ�����
		print_perm(statbuf.st_mode);			//�������Ȩ����Ϣ
		printf(" %d %s %s %*d %s %2d %02d:%02d %s",
				(int)statbuf.st_nlink,			//������
				p_passwd->pw_name,				//ӵ�����û���
				p_group->gr_name,				//ӵ������
				maxsizelen,						//�����ļ���С��ʾ�ĳ���
				(int)statbuf.st_size,			//�ļ���С
				months[p_time->tm_mon],			//���һ���޸�ʱ��
				p_time->tm_mday,
				p_time->tm_hour,
				p_time->tm_min,
				p_dirent->d_name);				//�ļ���
		if('l' == file_type(statbuf.st_mode)){	//����Ƿ��������ļ�,���ȡ���ӵ����ݲ���ʾ
			static char buf[256] = "";
			int len = 0;
			if(-1 == (len = readlink(p_dirent->d_name,buf,256))){
				perror("readlink");
				return;
			}
			printf(" -> %.*s\n",len,buf);
		}
		else
			putchar('\n');
	}
	rewinddir(p_dir);							//Ŀ¼����λ����ʼ
	while(NULL != (p_dirent = readdir(p_dir))){	//����һ��Ŀ¼��
		if('.' == p_dirent->d_name[0])
			continue;							//���� ".\"��"..\"�������ļ�
		if(-1 == lstat(p_dirent->d_name,&statbuf)){
			perror(p_dirent->d_name);
			return;
		}
		if('d' == file_type(statbuf.st_mode)){	//�����Ŀ¼�ļ�
			putchar('\n');						//����
			printdir(path,p_dirent->d_name);	//�ݹ����printdir,��ӡ��Ŀ¼����Ϣ
		}
	}
	if(-1 == chdir("..")){						//�����ϼ�Ŀ¼
		perror("chdir (\"..\")");
		return;
	}
	closedir(p_dir);
	return;
}
int main(){
	printdir("",".");							//������ǰĿ¼
	return 0;
}
